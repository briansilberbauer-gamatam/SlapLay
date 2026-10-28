# MidSide-670 — DeepSeek Harness (`dsh`) Workspace Instructions

## 1. Mandatory Skill Activation on Turn 1
Before planning, scaffolding, writing C++ DSP/UI code, or running builds in this repository, **immediately load the project skill** using the `skill` tool:
```json
{ "name": "vst3-80s-plugin" }
```
Then read its companion reference files using the `read` tool as needed:
- `.dsh/skills/vst3-80s-plugin/references/look-and-feel-recipes.md` — JUCE 8 `Graphics` code recipes for the Sleek Futuristic Studio Hardware (Brushed Gunmetal, Flush Glass & Muted Crimson) look & feel.
- `.dsh/skills/vst3-80s-plugin/references/github-actions-ci-release.md` — Cross-platform GitHub Actions workflow (`.github/workflows/build.yml`) for Windows x64, Linux x64, macOS Universal (`arm64` + `x86_64`), and automated GitHub Release publishing.

---

## 2. Core Project & Look-and-Feel Mandates
- **Plugin Format & Framework**: JUCE 8.0.8 (`VST3` + `Standalone`), C++17, CMake (`CMAKE_SKIP_RPATH TRUE`).
- **Offline Dependencies**: Reuse `/home/brians/bls/sound/plugin_poc/external/JUCE` and `/home/brians/bls/sound/plugin_poc/deps/sysroot` (copy or symlink into this workspace so everything builds offline inside `workspace-write` sandbox permissions).
- **Sleek Futuristic Studio Hardware Look & Feel (Gunmetal, Flush Glass & Muted Crimson)**:
  - **Style & Form Factor**: Sleek, futuristic desktop studio hardware unit sitting on a dark studio surface (`#12151C` -> `#07080B`) with dimensional three-quarter/chamfered presence and upper-left soft-box studio lighting.
  - **Base & Glass Overlay**: Dark gunmetal gray brushed metal (`#1D2026`) with fine horizontal grain and smoothly chamfered matte edges, topped by a thick, flush-mounted transparent glass panel with **zero visible screws or fasteners** and a subtle, low-profile **crimson red LED edge-glow (`#C82230`)** that casts faint crimson refraction inward.
  - **Displays & Visualization**:
    - **Top Header Display**: Slender, pill-shaped obsidian digital readout (`#090A0D`) at the top center displaying plugin/preset info framed by minimalist `<` `>` chevron buttons.
    - **Main Visualizer**: Large, central obsidian waveform screen embedded under the glass with subtle frequency/dB scale markers and a glowing crimson red real-time audio waveform.
  - **Controls**:
    - **Low-Profile Knobs**: Large, heavy-machined metal primary knobs with fine vertical knurled ridges on the outer rim, recessed low-profile height, subtle crimson LED under-glow, and a tiny minimalist tick mark on the flat brushed cap.
    - **Low-Profile Sliders & Horizontal Output Meter (Right-Side Full-Height Mixer Bay)**: Vertical touch-style / ultra-thin mechanical mixer sliders for **`Input`**, **`Wet/Dry Mix`**, and **`Output`** placed in a dedicated **right-side Mixer Bay** that spans the **full interior height of the component**, integrated smoothly into the glass surface with subtly recessed linear tracks and minimalist tick marks alongside them, with a dedicated **horizontal Output level meter (`OUT METER`) positioned vertically below the mixers** displaying real-time Output level in glowing crimson.
    - **Tactile Buttons**: Rectangular, low-profile tactile buttons (`"Bypass"`, `"Filter"`, `"Comp"`, `"Limiter"`) with crisp sans-serif typography, outlined by a thin, understated red glowing border when active.

---

## 3. DeepSeek V4 Pro & Flash Workflow in `dsh`
- **DeepSeek V4 Pro (`deepseek-pro` / `deepseek-reasoner`)**: Use for DSP math (Mid/Side matrix, 670 variable-mu compression, time constants, drive/tone/filter/limiter stages, latency-compensated bypass, lock-free waveform FIFO) and custom procedural `LookAndFeel` / `Graphics` rendering.
- **DeepSeek V4 Flash (`deepseek-flash`)**: Use for CMake scaffolding, APVTS parameter definitions, presets, unit/host test harnesses, and build execution.
- **Verification**: Every build must run:
  1. `./scripts/build.sh`
  2. `./build/*Tests_artefacts/Release/*Tests` (offline DSP tests)
  3. `./build/*HostSmokeTest_artefacts/Release/*HostSmokeTest dist/*.vst3` (VST3 host scan & state round-trip)
  4. `./build/*Screenshot_artefacts/Release/*Screenshot --out=dist/midside670-ui.png` followed by **`read_image`** (to verify the gunmetal/glass/crimson aesthetic, live red waveform, and check for clipping) and **`present`** (to attach the UI preview card in the DeepSeek Harness Web UI).
