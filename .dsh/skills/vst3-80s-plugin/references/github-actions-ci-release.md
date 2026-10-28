# Cross-Platform GitHub Actions CI & Release Workflow (Windows, Linux, macOS)

This reference provides the complete GitHub Actions pipeline (`.github/workflows/build.yml`), CMake `FetchContent` fallback, and `.gitignore` configuration to build, test, package, and publish **Windows x64**, **Linux x64**, and **macOS Universal (`arm64` + `x86_64`)** VST3 + Standalone releases.

---

## 1. How Local Offline (`dsh`) and GitHub Actions Coexist

1. **In `.gitignore`**:
   Always exclude local build/dependency directories so the multi-hundred-megabyte local JUCE checkout and Linux sysroot are never committed to Git:
   ```gitignore
   build/
   build-*/
   dist/
   external/
   deps/
   stage/
   stage-*/
   *.zip
   *.sha256
   ```

2. **In `CMakeLists.txt` (Hybrid Local / CI JUCE Resolution)**:
   Use `add_subdirectory(external/JUCE)` when present locally in DeepSeek Harness, and automatically fall back to `FetchContent` (JUCE `8.0.8` shallow clone) on GitHub Actions runners:
   ```cmake
   option(MIDSIDE670_BUILD_TESTS "Build the offline DSP test executable" ON)
   option(MIDSIDE670_BUILD_SCREENSHOT "Build the headless editor renderer" ON)
   option(MIDSIDE670_BUILD_HOST_TEST "Build the VST3 host smoke test" ON)

   if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
       add_subdirectory(external/JUCE)
   else()
       include(FetchContent)
       FetchContent_Declare(
           JUCE
           GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
           GIT_TAG 8.0.8
           GIT_SHALLOW TRUE)
       FetchContent_MakeAvailable(JUCE)
   endif()
   ```
   > **Important JUCE 8.0.8 `CMakeLists.txt` Note**: Do **not** set `JUCE_DISPLAY_SPLASH_SCREEN=0` in `target_compile_definitions`—JUCE 8.0.8 removed the splash screen and errors out if `JUCE_DISPLAY_SPLASH_SCREEN` is defined.

---

## 2. Platform-Specific CI Nuances

- **Windows (`runs-on: windows-2022`)**:
  - Pin `runs-on: windows-2022` instead of `windows-latest`. `windows-latest` maps to the newer runner image without the Visual Studio 17 2022 toolchain expected by JUCE 8.0.8.
  - Pass `-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded` so the built `.vst3` and Standalone `.exe` link the MSVC runtime statically and run on end-user systems without extra redistributable installers.
- **Linux (`runs-on: ubuntu-latest`)**:
  - Install X11, ALSA, FreeType, and Fontconfig dev headers via `apt-get`, build with `-G Ninja`, run both `MidSide670Tests` and `MidSide670HostSmokeTest`, and run `strip --strip-unneeded` on the `.so` inside the `.vst3` bundle and the Standalone binary before zipping.
- **macOS Universal (`runs-on: macos-14`)**:
  - Pass `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` and `-DCMAKE_OSX_DEPLOYMENT_TARGET="11.0"` with `-G Ninja` so a single Universal Binary bundle supports both Apple Silicon and Intel Macs.
  - Run ad-hoc code signing (`codesign --force --deep --sign -`) on both `MidSide670.vst3` and `MidSide670.app` before running `MidSide670HostSmokeTest` and packaging (required on Apple Silicon so `arm64` bundles execute without signing errors).
  - Package macOS bundles using `ditto -c -k --sequesterRsrc` so bundle metadata and structure are preserved.
- **Publishing GitHub Releases (`publish-release` job)**:
  - Runs automatically after all three platform jobs succeed **when pushing a version tag (`v*`, e.g., `v1.0.0`)** OR **when manually triggered via `workflow_dispatch` with a non-empty `release_tag` input**.
  - Top-level workflow permissions remain `contents: read`; only the `publish-release` job elevates to `contents: write`.

---

## 3. Complete `.github/workflows/build.yml` Template

Replace `MidSide670` / `MIDSIDE670` only if your CMake target name differs:

```yaml
name: Build Plugins

on:
  workflow_dispatch:
    inputs:
      release_tag:
        description: "Optional release tag (e.g. v1.0.0). Leave empty to only upload Actions artifacts."
        required: false
        type: string
  push:
    branches: [ main ]
    tags: [ "v*" ]
    paths:
      - "Source/**"
      - "Resources/**"
      - "Tests/**"
      - "Tools/**"
      - "CMakeLists.txt"
      - ".github/workflows/build.yml"
  pull_request:
    branches: [ main ]

permissions:
  contents: read

jobs:
  windows-x64:
    name: Windows x64
    # windows-latest maps to the VS 2026 image without the VS 2022 generator.
    # Pin windows-2022 for JUCE 8.0.8 compatibility.
    runs-on: windows-2022
    timeout-minutes: 40

    steps:
      - name: Check out source
        uses: actions/checkout@v5

      - name: Configure
        shell: pwsh
        run: |
          cmake -S . -B build-windows -A x64 `
            -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded `
            -DMIDSIDE670_BUILD_SCREENSHOT=OFF `
            -DMIDSIDE670_BUILD_HOST_TEST=ON
          if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

      - name: Build
        shell: pwsh
        run: |
          cmake --build build-windows --config Release --parallel
          if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

      - name: Run DSP tests
        shell: pwsh
        run: |
          & "build-windows\MidSide670Tests_artefacts\Release\MidSide670Tests.exe"
          if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

      - name: Run VST3 host test
        shell: pwsh
        run: |
          & "build-windows\MidSide670HostSmokeTest_artefacts\Release\MidSide670HostSmokeTest.exe" `
            "build-windows\MidSide670_artefacts\Release\VST3\MidSide-670.vst3"
          if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

      - name: Package Windows distribution
        shell: pwsh
        run: |
          $stage = "stage-windows"
          Remove-Item -Recurse -Force $stage -ErrorAction SilentlyContinue
          New-Item -ItemType Directory -Force $stage | Out-Null

          Copy-Item -Recurse -Force "build-windows\MidSide670_artefacts\Release\VST3\*.vst3" $stage
          Copy-Item -Force "build-windows\MidSide670_artefacts\Release\Standalone\*.exe" $stage
          if (Test-Path "README.md") { Copy-Item -Force "README.md" $stage }
          if (Test-Path "Resources\Lato-OFL.txt") { Copy-Item -Force "Resources\Lato-OFL.txt" $stage }

          $versionMatch = Select-String -Path "CMakeLists.txt" -Pattern 'project\(\s*[A-Za-z0-9_-]+\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)'
          if (-not $versionMatch -or $versionMatch.Matches.Count -eq 0) { throw "Could not read project version from CMakeLists.txt" }
          $version = $versionMatch.Matches[0].Groups[1].Value

          $zip = "MidSide670-Windows-x64-v$version.zip"
          Compress-Archive -Path "$stage\*" -DestinationPath $zip -Force

          $hash = (Get-FileHash $zip -Algorithm SHA256).Hash.ToLower()
          "$hash  $zip" | Out-File -Encoding ascii "MidSide670-Windows-x64-v$version.sha256"
          Get-Item $zip | Format-List Name, Length

      - name: Upload Windows distribution
        uses: actions/upload-artifact@v4
        with:
          name: MidSide670-Windows-x64
          path: |
            MidSide670-Windows-x64-v*.zip
            MidSide670-Windows-x64-v*.sha256
          if-no-files-found: error

  linux-x64:
    name: Linux x64
    runs-on: ubuntu-latest
    timeout-minutes: 40

    steps:
      - name: Check out source
        uses: actions/checkout@v5

      - name: Install Linux build dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y --no-install-recommends \
            build-essential cmake ninja-build pkg-config zip \
            libasound2-dev \
            libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev \
            libxcomposite-dev libxrender-dev libxfixes-dev libxi-dev libsm-dev libice-dev \
            libx11-xcb-dev libxcb1-dev \
            libexpat1-dev zlib1g-dev libpng-dev libbrotli-dev libbz2-dev

          if apt-cache show libfreetype-dev >/dev/null 2>&1; then
            FREETYPE_PACKAGE=libfreetype-dev
          else
            FREETYPE_PACKAGE=libfreetype6-dev
          fi

          if apt-cache show libfontconfig-dev >/dev/null 2>&1; then
            FONTCONFIG_PACKAGE=libfontconfig-dev
          else
            FONTCONFIG_PACKAGE=libfontconfig1-dev
          fi

          sudo apt-get install -y --no-install-recommends "$FREETYPE_PACKAGE" "$FONTCONFIG_PACKAGE"

      - name: Configure
        run: |
          cmake -S . -B build-linux -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DMIDSIDE670_BUILD_SCREENSHOT=OFF \
            -DMIDSIDE670_BUILD_HOST_TEST=ON

      - name: Build
        run: cmake --build build-linux --parallel

      - name: Run DSP tests
        run: ./build-linux/MidSide670Tests_artefacts/Release/MidSide670Tests

      - name: Run VST3 host test
        run: |
          ./build-linux/MidSide670HostSmokeTest_artefacts/Release/MidSide670HostSmokeTest \
            build-linux/MidSide670_artefacts/Release/VST3/*.vst3

      - name: Package Linux distribution
        run: |
          set -euo pipefail

          VERSION="$(sed -nE 's/^project\([A-Za-z0-9_-]+[[:space:]]+VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' CMakeLists.txt)"
          if [ -z "$VERSION" ]; then
            echo "Could not read project version from CMakeLists.txt" >&2
            exit 1
          fi

          STAGE="stage-linux"
          rm -rf "$STAGE"
          mkdir -p "$STAGE"

          cp -a build-linux/MidSide670_artefacts/Release/VST3/*.vst3 "$STAGE/"
          cp build-linux/MidSide670_artefacts/Release/Standalone/* "$STAGE/"
          [ -f README.md ] && cp README.md "$STAGE/"
          [ -f Resources/Lato-OFL.txt ] && cp Resources/Lato-OFL.txt "$STAGE/"

          find "$STAGE" -type f \( -name "*.so" -o -perm -111 \) -exec strip --strip-unneeded {} + 2>/dev/null || true

          ZIP="MidSide670-Linux-x64-v${VERSION}.zip"
          (cd "$STAGE" && zip -q -r "../${ZIP}" .)
          sha256sum "$ZIP" > "MidSide670-Linux-x64-v${VERSION}.sha256"

          ls -lh "$ZIP"
          cat "MidSide670-Linux-x64-v${VERSION}.sha256"

      - name: Upload Linux distribution
        uses: actions/upload-artifact@v4
        with:
          name: MidSide670-Linux-x64
          path: |
            MidSide670-Linux-x64-v*.zip
            MidSide670-Linux-x64-v*.sha256
          if-no-files-found: error

  macos-universal:
    name: macOS Universal
    runs-on: macos-14
    timeout-minutes: 45

    steps:
      - name: Check out source
        uses: actions/checkout@v5

      - name: Install Ninja
        run: brew install ninja

      - name: Configure Universal Binary (arm64 + x86_64)
        run: |
          cmake -S . -B build-macos -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
            -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0" \
            -DMIDSIDE670_BUILD_SCREENSHOT=OFF \
            -DMIDSIDE670_BUILD_HOST_TEST=ON

      - name: Build
        run: cmake --build build-macos --parallel

      - name: Ad-hoc sign macOS bundles
        run: |
          for bundle in build-macos/MidSide670_artefacts/Release/VST3/*.vst3 \
                        build-macos/MidSide670_artefacts/Release/Standalone/*.app; do
            if [ -e "$bundle" ]; then
              codesign --force --deep --sign - "$bundle"
            fi
          done

      - name: Run DSP tests
        run: ./build-macos/MidSide670Tests_artefacts/Release/MidSide670Tests

      - name: Run VST3 host test
        run: |
          ./build-macos/MidSide670HostSmokeTest_artefacts/Release/MidSide670HostSmokeTest \
            build-macos/MidSide670_artefacts/Release/VST3/*.vst3

      - name: Package macOS distribution
        run: |
          set -euo pipefail

          VERSION="$(sed -nE 's/^project\([A-Za-z0-9_-]+[[:space:]]+VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' CMakeLists.txt)"
          if [ -z "$VERSION" ]; then
            echo "Could not read project version from CMakeLists.txt" >&2
            exit 1
          fi

          STAGE="stage-macos"
          rm -rf "$STAGE"
          mkdir -p "$STAGE"

          cp -R build-macos/MidSide670_artefacts/Release/VST3/*.vst3 "$STAGE/"
          cp -R build-macos/MidSide670_artefacts/Release/Standalone/*.app "$STAGE/"
          [ -f README.md ] && cp README.md "$STAGE/"
          [ -f Resources/Lato-OFL.txt ] && cp Resources/Lato-OFL.txt "$STAGE/"

          ZIP="MidSide670-macOS-Universal-v${VERSION}.zip"
          ditto -c -k --sequesterRsrc "$STAGE" "$ZIP"
          shasum -a 256 "$ZIP" > "MidSide670-macOS-Universal-v${VERSION}.sha256"

          ls -lh "$ZIP"
          cat "MidSide670-macOS-Universal-v${VERSION}.sha256"

      - name: Upload macOS distribution
        uses: actions/upload-artifact@v4
        with:
          name: MidSide670-macOS-Universal
          path: |
            MidSide670-macOS-Universal-v*.zip
            MidSide670-macOS-Universal-v*.sha256
          if-no-files-found: error

  publish-release:
    name: Publish release
    needs: [ windows-x64, linux-x64, macos-universal ]
    if: ${{ github.ref_type == 'tag' || (github.event_name == 'workflow_dispatch' && inputs.release_tag != '') }}
    runs-on: ubuntu-latest
    permissions:
      contents: write

    steps:
      - name: Download release packages
        uses: actions/download-artifact@v4
        with:
          pattern: MidSide670-*
          path: release-assets
          merge-multiple: true

      - name: Publish GitHub release
        uses: softprops/action-gh-release@v2
        with:
          tag_name: ${{ inputs.release_tag || github.ref_name }}
          target_commitish: ${{ github.sha }}
          name: MidSide-670 ${{ inputs.release_tag || github.ref_name }}
          files: |
            release-assets/*.zip
            release-assets/*.sha256
          fail_on_unmatched_files: true
          overwrite_files: true
          generate_release_notes: true
          prerelease: ${{ contains(inputs.release_tag || github.ref_name, '-') }}
```

---

## 4. How to Trigger a Release

1. **Tag-Based Release (Recommended)**:
   - Update `project(MidSide670 VERSION X.Y.Z LANGUAGES C CXX)` in `CMakeLists.txt`.
   - Commit, tag, and push:
     ```bash
     git tag v1.0.0
     git push origin main --tags
     ```
   - GitHub Actions builds and runs DSP + VST3 host smoke tests on **Windows x64**, **Linux x64**, and **macOS Universal**, then runs `publish-release` to create the GitHub Release with all three `.zip` archives and `.sha256` checksums attached.
2. **Manual Dispatch Release (`workflow_dispatch`)**:
   - In GitHub -> **Actions** -> **Build Plugins** -> **Run workflow**, enter `v1.0.0` (or `v1.0.0-rc1` for a pre-release) in the `release_tag` input. Leave `release_tag` blank if you only want CI artifacts without creating a public GitHub Release.
