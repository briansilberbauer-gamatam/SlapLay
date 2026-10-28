---
name: vst3-80s-plugin
description: >-
  JUCE 8 VST3 audio plugin architecture, offline Linux CMake build/test workflow,
  GitHub Actions CI/release pipelines (Windows, Linux, macOS Universal), DeepSeek
  Harness (dsh) Flash & Pro orchestration, and Futuristic Studio Hardware Look &
  Feel (brushed gunmetal chassis, flush transparent glass overlay with crimson
  LED edge-glow, low-profile knurled metal knobs, right-side full-height
  Input/Mix/Output glass mixer sliders with a horizontal Output level meter
  positioned below the mixers, and central glowing red waveform
  visualizer).
user-invocable: true
---

# VST3 Plugin Development & Futuristic Studio Hardware UI Skill (DeepSeek Harness)

This skill defines the engineering workflow, JUCE 8.0.8 conventions, **DeepSeek Harness (`dsh`)** tool & model orchestration (**DeepSeek V4 Pro** vs **DeepSeek V4 Flash**), cross-platform **GitHub Actions CI & Release workflows (Windows, Linux, macOS)**, and the **Sleek Futuristic Studio Hardware (Brushed Gunmetal + Flush Glass Overlay + Muted Crimson Red LED Edge-Glow)** visual design system for building VST3 plugins in `/home/brians/bls/sound/MidSide-670`.

When this skill is loaded in DeepSeek Harness, also read the companion reference files using the `read` tool as needed:
- `references/look-and-feel-recipes.md` — Copy-pasteable JUCE 8 `Graphics` drawing recipes for the brushed gunmetal chassis, crimson-edged glass overlay, low-profile knurled metal knobs, ultra-thin vertical sliders, glowing red border tactile buttons, top pill readout, and central real-time red waveform visualizer.
- `references/github-actions-ci-release.md` — Complete `.github/workflows/build.yml` for Windows x64, Linux x64, macOS Universal (`arm64` + `x86_64`), and automated GitHub Release publishing.

---

## 1. DeepSeek Harness (`dsh`) Execution & Model Routing

### A. DeepSeek V4 Pro vs. Flash Division of Labor
Whether switching models manually in the harness or orchestrating child agents via the `workflow` / `subagent` tools, split tasks by reasoning depth:

| Task Area | Target Model | Responsibilities |
| :--- | :--- | :--- |
| **DSP & Audio Architecture** | **DeepSeek V4 Pro** (`deepseek-pro` / `deepseek-reasoner`) | Mid/Side encoding/decoding (`M = (L+R)*0.5`, `S = (L-R)*0.5`), 670-style variable-mu / program-dependent compression curves, time-constant networks, drive/tone shaping, filter/limiter stages, bypass latency compensation, and lock-free UI metering + real-time waveform ring buffer atomics. |
| **Custom `LookAndFeel` & Vector Art** | **DeepSeek V4 Pro** (`deepseek-pro` / `deepseek-reasoner`) | Writing procedural JUCE `Graphics` rendering in `LookAndFeel`, `Theme`, and custom visualizer components: three-quarter/dimensional desktop unit on a dark studio surface, horizontal brushed gunmetal grain, flush screwless glass overlay with crimson LED edge-glow and upper-left soft-box reflections, low-profile heavy-machined knurled knobs with crimson under-glow, ultra-thin glass vertical sliders, thin red-bordered tactile buttons, pill-shaped preset header readout, and central glowing red waveform visualizer. |
| **Scaffolding & Parameter Plumbing** | **DeepSeek V4 Flash** (`deepseek-flash`) | Creating `CMakeLists.txt`, `Source/Parameters.h`, `Source/Presets.h`, APVTS parameter layout, attachment wiring, value formatters (`dB`, `ms`, `Hz`, `%`, ratios), and component `resized()` layout. |
| **Tests, Tooling & Build Loop** | **DeepSeek V4 Flash** (`deepseek-flash`) | Writing `Tests/DspTests.cpp`, `Tools/HostSmokeTest.cpp`, `Tools/RenderEditor.cpp`, `scripts/setup-linux.sh`, `scripts/build.sh`, `.github/workflows/build.yml`, and executing CMake builds and test binaries. |

### B. DeepSeek Harness Tooling Rules
1. **Task Tracking (`todo_write` / `create_goal`)**:
   - At the beginning of a multi-step build, call `todo_write` to maintain a clear checklist (`Setup JUCE & sysroot`, `Implement Parameters & DSP + Waveform FIFO`, `Implement Brushed Gunmetal & Crimson Glass UI`, `Build & Run DSP/Host Tests`, `Render & Inspect UI Screenshot`).
2. **Parallel Subagents & Multi-Model `workflow`**:
   - Use `subagent` / `subagent_fork` to parallelize independent C++ modules (for example, one agent implementing `Source/dsp/` + `Tests/DspTests.cpp` while another implements `Source/ui/`) so long C++ files and compiler outputs do not bloat the main session context.
   - When using the `workflow` tool to coordinate models explicitly, pass `model` in `agent(prompt, { model: "deepseek-pro" | "deepseek-flash", label, phase })`.
3. **File Observation Policy (`read` before `edit` or `write`)**:
   - `dsh` enforces a strict read-before-write policy on existing files. Always call `read` on an existing file before calling `edit` or overwriting it with `write`.
4. **Sandbox Permissions (`workspace-write` vs `danger-full-access`)**:
   - By default, `bash` runs in `workspace-write` mode: writing is permitted inside `/home/brians/bls/sound/MidSide-670` and `/tmp`, while reading is permitted across the local filesystem (including `/home/brians/bls/sound/plugin_poc`).
   - Always pass `description` before `command` in `bash` calls, and set `workdir: "/home/brians/bls/sound/MidSide-670"`.
   - Symlinking or copying `/home/brians/bls/sound/plugin_poc/external/JUCE` and `/home/brians/bls/sound/plugin_poc/deps/sysroot` into this workspace works inside default `workspace-write` mode—do **not** download packages from the internet when the sibling `plugin_poc` cache exists.
   - Only request `sandbox_permissions: "danger-full-access"` (with a clear `justification`) if copying the built `.vst3` bundle outside the workspace (e.g., to `~/.vst3`).
5. **Visual Inspection & Presentation (`read_image` + `present`)**:
   - Never declare UI work finished based solely on a zero exit code from CMake.
   - Run the `RenderEditor` screenshot binary (using `xvfb-run -a` if headless) while feeding synthetic audio so the central waveform visualizer shows an active glowing red waveform, outputting a 2x PNG (`dist/midside670-ui.png`).
   - Call **`read_image`** on `dist/midside670-ui.png` to visually inspect the rendered plugin and verify:
     - Dark brushed gunmetal chassis (`#1D2026`) sitting on a dark studio surface with chamfered matte edges and **zero visible screws or fasteners**.
     - Flush transparent glass overlay with subtle crimson LED edge-glow/refraction (`#C82230`) and upper-left soft-box specular highlights.
     - Top-center pill-shaped digital preset readout framed by minimalist `<` `>` chevrons.
     - Central obsidian screen with a glowing red real-time audio waveform and subtle frequency/dB scale markers.
     - Low-profile heavy-machined knurled metal knobs with tiny cap tick marks and subtle crimson LED under-glow, and rectangular tactile buttons outlined by a thin red glowing border when active.
     - **Right-Side Full-Height Mixer Bay & Horizontal Output Meter Below the Mixers**: Vertical ultra-thin recessed glass sliders for **`Input`**, **`Wet/Dry Mix`**, and **`Output`** positioned on the **right side of the component** inside the full-height right-side Mixer Bay, with a dedicated **horizontal Output level meter (`OutputMeter`) positioned vertically below the mixers** displaying real-time Output levels in glowing crimson.
     - Zero text truncation or overlapping controls.
   - Call **`present`** on the PNG file path and embed `![MidSide-670 UI Preview](<dist/midside670-ui.png>)` in the final response so the user sees the rendered UI card in the DeepSeek Harness Web UI.

---

## 2. Critical JUCE 8.0.8 & Real-Time Audio Guardrails

1. **Strict JUCE 8.0.8 API Compliance**:
   - **Fonts**: Never use the deprecated `juce::Font(typeface, height, style)` constructor. Always construct fonts via `juce::FontOptions`:
     ```cpp
     juce::Font result { juce::FontOptions (typeface).withHeight (height) };
     ```
   - **Preprocessor Definitions**: Never define `JUCE_DISPLAY_SPLASH_SCREEN=0` in `CMakeLists.txt` (removed in JUCE 8.0.8 and triggers a `#error`).
   - **Plugin Host Test**: When building a host test that scans `.vst3` bundles, set `JUCE_PLUGINHOST_VST3=1` in `target_compile_definitions`.
2. **Real-Time Audio Thread Safety & Lock-Free Waveform Telemetry**:
   - Zero heap allocations (`new`, `malloc`, `std::vector::push_back`, `juce::String`), zero mutex locks, and zero file/OS calls inside `processBlock()`.
   - Read parameters once per block via raw atomic pointers (`apvts.getRawParameterValue(id)->load()`) and smooth sensitive gain/filter/compression changes.
   - Expose UI telemetry (Input/Output L/R peaks, Mid/Side levels, Gain Reduction dB) via `std::atomic<float>` and feed the **Main Waveform Visualizer** using a pre-allocated, lock-free fixed-size ring buffer (`std::array<std::atomic<float>, 512>` or `juce::AbstractFifo`) polled by a `30–60 Hz` `juce::Timer` on the message thread.
3. **Constant Latency Across Bypass**:
   - If any stage uses oversampling or lookahead, report the latency via `setLatencySamples()` in `prepareToPlay()` and route bypassed audio through a matching `juce::dsp::DelayLine` so bypassing never causes phase/timing jumps in the host.
4. **Offline Linux Sysroot & JUCE Bootstrap**:
   - Reuse `/home/brians/bls/sound/plugin_poc/external/JUCE` (JUCE 8.0.8) and `/home/brians/bls/sound/plugin_poc/deps/sysroot` (Ubuntu 24.04 rootless dev headers/libs + `xvfb-run`).
   - Configure `PKG_CONFIG_PATH`, `PKG_CONFIG_SYSROOT_DIR`, `CMAKE_CXX_FLAGS`, and `CMAKE_EXE_LINKER_FLAGS` / `CMAKE_SHARED_LINKER_FLAGS` identically to `../plugin_poc/scripts/build.sh`, with `CMAKE_SKIP_RPATH TRUE` so built binaries do not embed restricted sysroot RPATHs.

---

## 3. Project Layout & CMake Architecture

Use the proven four-target CMake structure (`CMAKE_CXX_STANDARD 17`, `CMAKE_SKIP_RPATH TRUE`):

```text
.
├── .github/
│   └── workflows/
│       └── build.yml           # Windows x64, Linux x64, macOS Universal CI + GitHub Release job
├── .gitignore                  # Excludes external/, deps/, build*/, dist/, stage*/, *.zip
├── AGENTS.md                   # Always-on DeepSeek Harness project instructions
├── CMakeLists.txt              # Plugin (VST3+Standalone), DspTests, HostSmokeTest, Screenshot
├── Resources/                  # Bundled clean geometric sans TTF fonts + OFL license
├── Source/
│   ├── Parameters.h            # Stable parameter IDs & ranges
│   ├── Presets.h               # Deterministic factory presets (reset all to default, then apply)
│   ├── PluginProcessor.h/.cpp  # APVTS, M/S 670 DSP chain, atomic meters & lock-free waveform FIFO
│   ├── PluginEditor.h/.cpp     # Fixed 1180x720 design canvas + AffineTransform resizer
│   ├── dsp/                    # Header-only or modular DSP classes
│   └── ui/
│       ├── Theme.h/.cpp        # Gunmetal, Obsidian, Glass & Muted Crimson palette + surface helpers
│       ├── StudioLookAndFeel.* # Low-profile knurled metal knobs, thin glass sliders, glowing buttons
│       ├── Controls.h/.cpp     # KnurledKnob, GlassSlider, TactileButton, HeaderPillBar
│       └── WaveformDisplay.*   # Embedded central obsidian screen + glowing red real-time waveform
├── Tests/
│   └── DspTests.cpp            # Offline deterministic DSP test runner
├── Tools/
│   ├── HostSmokeTest.cpp       # Instantiates built .vst3 in JUCE VST3 host & round-trips state
│   └── RenderEditor.cpp        # Feeds synthetic signal & exports 2x PNG screenshot of UI
└── scripts/
    ├── setup-linux.sh          # Symlinks/copies ../plugin_poc/external/JUCE & deps/sysroot
    └── build.sh                # Configures, builds Release, and populates dist/
```

---

## 4. Look & Feel Specification: Sleek Futuristic Studio Hardware (Gunmetal, Glass & Muted Crimson)

### 1. Core Concept & Aesthetic
- **Style**: Sleek, futuristic studio hardware meets high-end software GUI. Blends tactile analog instrumentation with modern, minimalist digital elements.
- **Form Factor**: Desktop audio unit sitting on a dark studio surface, rendered with dimensional depth (three-quarter / chamfered desktop unit framing where the matte chamfered gunmetal base, thick layered glass top plate, and upper-left soft-box studio shadow are clearly perceived).
- **Overall Vibe**: Premium, professional, moody, and precise.

### 2. Materials & Surface Textures
- **Base Chassis**: Dark, gunmetal gray brushed metal (`#1D2026`) with a fine horizontal grain texture. The outer edges are smoothly chamfered (`#252A32` -> `#121419`) and matte-finished.
- **Overlay Layer**: A thick, premium transparent glass (or polished acrylic) panel layered over the brushed metal base.
- **Glass Edges**: The vertical and perimeter edges of the glass feature a subtle, low-profile **crimson red LED edge-glow (`#C82230`)** that casts a faint crimson refraction inward through the clear material.
- **Fasteners**: Completely seamless and flush-mounted—**no visible screws, rivets, or mounting hardware** anywhere on the top glass surface.
- **Lighting**: Studio soft-box ambient lighting from the **upper left**, casting soft shadows toward the bottom-right and creating crisp, glossy specular reflections and highlights across the polished glass surface.

### 3. Color Palette Constants (`Source/ui/Theme.h`)

Use this exact palette in `Theme.h`:

```cpp
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace studio::ui
{
    // --- Dark Studio Surface & Ambient Soft-Box Backdrop ---
    inline const juce::Colour studioSurfaceLight { 0xff12151c }; // Upper-left soft-box lit desk surface
    inline const juce::Colour studioSurfaceDark  { 0xff07080b }; // Lower-right shadowed studio surface
    inline const juce::Colour dropShadowDeep     { 0xff020305 }; // Desktop unit cast shadow

    // --- Brushed Gunmetal Base Chassis & Chamfered Edges ---
    inline const juce::Colour gunmetalLight      { 0xff2b3038 }; // Upper-left brushed metal highlight
    inline const juce::Colour gunmetalBase       { 0xff1d2026 }; // Primary gunmetal gray chassis
    inline const juce::Colour gunmetalDeep       { 0xff13151a }; // Lower-right chassis shade & recesses
    inline const juce::Colour chamferLight       { 0xff343a44 }; // Upper-left matte chamfered edge catch
    inline const juce::Colour chamferDark        { 0xff0e1014 }; // Lower-right chamfered skirt extrusion

    // --- Obsidian Black Screens & Recessed Cavities ---
    inline const juce::Colour obsidianScreen     { 0xff090a0d }; // Main visualizer & top pill readout background
    inline const juce::Colour obsidianWell       { 0xff0d0f13 }; // Low-profile knob & slider mounting recess
    inline const juce::Colour obsidianGrid       { 0xff181b22 }; // Subtle dB / frequency scale grid hairlines

    // --- Transparent Glass Overlay & Soft-Box Reflections ---
    inline const juce::Colour glassTintTop       { 0x1affffff }; // Upper-left soft-box glass sheen
    inline const juce::Colour glassTintBottom    { 0x26050608 }; // Deep smoked glass depth tint
    inline const juce::Colour glassSpecular      { 0x14ffffff }; // Glossy diagonal soft-box reflection band
    inline const juce::Colour glassRimHighlight  { 0x33ffffff }; // Crisp 1px polished glass top/left bevel

    // --- Muted, Sophisticated Crimson Red Accents ---
    inline const juce::Colour crimsonAccent      { 0xffc82230 }; // Primary muted crimson LED & active border
    inline const juce::Colour crimsonBright      { 0xffef3b49 }; // Hot core of waveform trace & edge-glow center
    inline const juce::Colour crimsonGlow        { 0xff8c141f }; // Diffused crimson refraction & knob under-glow
    inline const juce::Colour crimsonSubtle      { 0xff3d1218 }; // Dormant/dim crimson scale & track undertone

    // --- Clean Minimalist Sans-Serif Typography ---
    inline const juce::Colour textPrimary        { 0xffeceff4 }; // Active labels, pill readout & cap tick marks
    inline const juce::Colour textSecondary      { 0xff929aa8 }; // Control titles ("Drive", "Tone", "Mix", "Output")
    inline const juce::Colour textMuted          { 0xff545b68 }; // Subtle dB/Hz scale markers & inactive buttons
}
```

---

### 4. Controls & Interface Elements (Component Specifications)

#### A. Desktop Chassis, Brushed Gunmetal Base & Crimson-Edged Glass Overlay
- **Three-Quarter / Dimensional Desktop Presence**:
  - Inset the hardware unit slightly from the outer window (`14px–18px` margin) so it rests visibly on the dark studio surface (`studioSurfaceLight` -> `studioSurfaceDark`).
  - Render a slightly offset bottom-right chamfered extrusion base (`chamferDark` + multi-pass `dropShadowDeep`) so the unit reads as a physical desktop hardware instrument lit from the upper-left soft-box.
- **Horizontal Brushed Gunmetal Texture**:
  - Fill the chassis body with a directional gradient (`gunmetalLight` at top-left to `gunmetalDeep` at bottom-right) and draw deterministic `1px` horizontal brush grain lines across the entire width using alternating subtle alpha strokes (`0x08ffffff` and `0x12000000`).
- **Flush Glass Panel with Crimson LED Edge-Glow (Zero Screws)**:
  - Layer a seamless, screwless glass rectangle over the brushed metal base.
  - Along the vertical and perimeter edges of the glass, paint a soft inward crimson refraction gradient (`crimsonGlow.withAlpha(0.22f)` fading over `10px`) paired with a crisp `1.2f` perimeter light-pipe stroke (`crimsonAccent`, brightest along the left/right vertical edges and top-left corner).
  - Paint a crisp diagonal soft-box glass reflection (`glassSpecular`) sweeping across the upper-left quadrant of the glass surface.

#### B. Top Header Pill-Shaped Digital Readout (`HeaderPillBar`)
- **Geometry**: A slender, pill-shaped (`cornerRadius = height * 0.5f`) digital readout screen positioned at the **top center** of the glass panel.
- **Visuals**:
  - Filled with `obsidianScreen` (`#090A0D`), outlined by a `1px` polished glass rim (`glassRimHighlight` top-left, `crimsonAccent.withAlpha(0.45f)` subtle crimson refraction along the sides).
  - Displays the plugin/preset identifier in clean, tracked uppercase sans-serif typography (`textPrimary`, e.g., `MIDSIDE-670 // MASTER GLUE`), with subtle crimson status dot or mode tag.
  - Framed on the left and right inside the pill by minimalist chevron navigation buttons (`<` and `>`) that highlight with a thin crimson border on hover/press.

#### C. Main Central Waveform Visualizer (`WaveformDisplay`)
- **Geometry**: A large, central display screen embedded beneath the glass panel (`obsidianScreen` `#090A0D`).
- **Scale & Grid**:
  - Draw ultra-fine horizontal dB scale markers (`0 dB`, `-6 dB`, `-12 dB`, `-18 dB`, `-24 dB`) and vertical time/frequency divisions in `obsidianGrid` (`#181B22`), labeled with crisp `7.5pt` minimalist sans-serif markers in `textMuted`.
- **Glowing Red Real-Time Audio Waveform**:
  - Render the live audio waveform (and optional subtle gain-reduction trace) using a three-layer crimson glow pass:
    1. **Subtle Filled Envelope & Wide Halo**: Filled waveform body in `crimsonGlow.withAlpha(0.12f)` + outer stroke (`5.5f` width) in `crimsonGlow.withAlpha(0.24f)`.
    2. **Primary Muted Crimson Trace**: Stroke (`2.0f` width) in `crimsonAccent` (`#C82230`).
    3. **Crisp Specular Core**: Stroke (`0.9f` width) in `crimsonBright` (`#EF3B49`).
  - Overlay a subtle upper-left glass reflection across the visualizer window so it sits cohesively under the main glass sheet.

#### D. Low-Profile Heavy-Machined Knurled Metal Knobs (`drawRotarySlider`)
- **Design**: Large, heavy-machined metal primary knobs (e.g., `"Drive"`, `"Tone"`, `"Threshold"`, `"Time Constant"`) recessed into the glass/metal chassis with a **flat, low-profile height**:
  1. **Recessed Well & Subtle Crimson LED Under-Glow**:
     - Draw a recessed circular cavity (`obsidianWell`) just outside the knob rim.
     - Render a soft, diffused crimson LED under-glow (`crimsonGlow` / `crimsonAccent` radial halo + subtle active arc) inside the recess gap so crimson light appears to bleed from *underneath* the machined metal skirt.
  2. **Fine Knurled Vertical Ridges on Outer Rim**:
     - Draw the outer machined metal bevel (`radius * 0.86f` to `radius * 0.74f`) with **64 fine radial knurling ridges** (`0.9f` lines alternating upper-left specular `gunmetalLight.brighter(0.25f)` and dark groove `obsidianScreen`) to simulate precision CNC vertical knurling on the outer rim.
  3. **Flat Low-Profile Brushed Metal Cap**:
     - Fill the wide flat cap (`radius * 0.74f`) with an upper-left to lower-right metallic gradient (`#303540` -> `#1E2229` -> `#14171C`), overlaid with fine horizontal brushed grain lines clipped to the cap circle and a `1px` top-left specular bevel.
  4. **Tiny Minimalist Tick Mark**:
     - Near the outer perimeter of the flat cap (`radius * 0.56f` to `radius * 0.70f`), draw a **tiny, surgical minimalist tick mark** (`1.6f` wide) in crisp `textPrimary` (`#ECEFF4`) or `crimsonBright`. Never draw bulky plastic wedges.

#### E. Low-Profile Vertical Touch / Ultra-Thin Mechanical Sliders, Horizontal Output Meter & Right-Side Full-Height Mixer Bay (`drawLinearSlider` + `OutputMeter`)
- **Mandatory Right-Side Full-Height Placement (`Input`, `Wet/Dry Mix`, `Output` + Horizontal Output Meter Below Mixers)**:
  - The mixer sliders for **`Input`** (input gain in `dB`), **`Wet/Dry Mix`** (`0%–100%` blend), and **`Output`** (output level in `dB`), together with a **horizontal Output level meter positioned vertically below the mixers**, **must** be placed on the **right side of the component** inside a dedicated **Right-Side Mixer Bay** that spans the **full interior height** of the glass panel (from the top inner margin to the bottom inner margin).
  - In `resized()`, always slice the Right-Side Mixer Bay off `innerGlass` **first** (`auto rightMixerBay = innerGlass.removeFromRight (204);`) before slicing the left/central workspace into the Top Header Pill-Shaped Digital Readout, Central Waveform Visualizer, and bottom/lower-deck Knurled Metal Knobs & Tactile Buttons.
  - Separate the Right-Side Mixer Bay from the left/central workspace with a full-height vertical glass/crimson divider hairline (`obsidianGrid` + subtle `crimsonAccent.withAlpha(0.28f)` top-to-bottom accent line).
  - **Vertical Stack Inside `rightMixerBay` (Mixers on Top, Horizontal Output Meter Below)**:
    1. **Lower Section — Horizontal Output Level Meter (`removeFromBottom(72)`)**:
       - Carve the lower section of `rightMixerBay` (`auto outputMeterArea = rightMixerBay.removeFromBottom (72);`) directly **vertically below the three mixer sliders**, spanning the full width of `rightMixerBay`, to house the **Horizontal Output Meter** (`OutputMeter`) for real-time Output level monitoring.
       - Render a recessed obsidian glass well (`obsidianScreen` `#090A0D` with `1px` `glassRimHighlight` top-left catch and subtle crimson inner edge refraction) containing:
         - **Top Header Row**: Left-aligned tracked sans-serif `"OUT METER"` title (`textSecondary`) and right-aligned live tabular numeric peak readout (`"-1.8 dB"`, `textPrimary` or `crimsonBright` near `0 dB`).
         - **Horizontal Stereo (`L` / `R`) Output Level Bars**: Twin horizontal recessed meter tracks (`obsidianWell`) filling from **left (`-60 dB`) to right (`0 dB`)**, rendered with glowing muted crimson (`crimsonAccent` `#C82230`) transitioning to bright crimson (`crimsonBright` `#EF3B49`) near `0 dB`, a soft outer crimson bloom (`crimsonGlow.withAlpha(0.32f)`), and ballistic peak-hold vertical hairlines.
         - **Bottom dB Scale Markings**: Minimalist vertical tick marks and `7.5pt` horizontal scale labels (`-48`, `-36`, `-24`, `-12`, `-6`, `0`) in `textMuted` along the bottom of the horizontal meter bars.
    2. **Upper Section — Three Vertical Mixer Columns (`Input`, `Mix` / `Wet/Dry`, `Output`)**:
       - Divide the remaining upper expanse of `rightMixerBay` into **three equal vertical columns** (`Input`, `Mix` / `Wet/Dry`, `Output`). Each column contains:
         - **Top Header Label (`22px`)**: Crisp tracked uppercase sans-serif title (`"INPUT"`, `"MIX"`, `"OUTPUT"`) in `textSecondary`.
         - **Vertical Slider Track (Middle)**: A `juce::Slider::LinearVertical` control occupying the full remaining vertical span between the top label and bottom readout.
         - **Bottom Live Value Readout (`22px`)**: Clean tabular numeric readout (`"-0.0 dB"`, `"100 %"`, `"-0.0 dB"`) in `textPrimary`.
- **Slider Visual Design**:
  - **Recessed Linear Track**: A narrow (`4px` wide) vertical recessed slot (`obsidianWell`) integrated smoothly into the glass surface with a `1px` glass edge highlight and a subtle muted crimson active fill (`crimsonAccent`) rising from the bottom to the thumb position.
  - **Minimalist Tick Marks**: Hairline horizontal tick marks (`7px` major / `4px` minor, `1px` thick in `textMuted.withAlpha(0.55f)`) arrayed cleanly along both sides of the recessed track.
  - **Ultra-Thin Flush Slider Thumb**: A sleek, low-profile machined metal/glass touch bar (`width = 24px`, `height = 9px`, `cornerRadius = 2.0f`) with an upper-left specular edge and a thin `1.2px` glowing `crimsonBright` center indicator line.

#### F. Low-Profile Rectangular Tactile Buttons (`drawToggleButton`)
- **Design**: Rectangular, low-profile tactile buttons (`"Bypass"`, `"Filter"`, `"Comp"`, `"Limiter"`, `"M/S Mode"`, `"Link"`) flush-mounted in the glass surface with crisp, clean sans-serif typography:
  - **Inactive (Off) State**:
    - Flush dark gunmetal/glass fill (`#161920` -> `#12141A`), `1px` understated border (`#252A34`), and clean sans-serif label in `textSecondary`.
  - **Active (On) State**:
    - Outlined by a **thin, understated red glowing border**:
      - Soft outer crimson bloom (`crimsonGlow.withAlpha(0.32f)`, `3.0f` rounded rect stroke)
      - Crisp `1.2f` border in `crimsonAccent` (`#C82230`) with a subtle `crimsonBright` catchlight on the upper-left corner
      - Faint interior crimson glass tint (`crimsonGlow.withAlpha(0.10f)`) and crisp `textPrimary` (`#ECEFF4`) typography.

---

## 5. Fixed Design Canvas, Spatial Layout & Resizable Scaling Pattern

### A. Spatial Layout Architecture (Main Workspace + Right-Side Full-Height Mixer Bay with Horizontal Output Meter Below Mixers)

Within the flush glass overlay (`innerGlass`), the layout is partitioned horizontally first so the **Right-Side Mixer Bay spans the full height of the component**, with the **`Input`, `Wet/Dry Mix`, and `Output` sliders in the upper section** and the **Horizontal Output Level Meter (`OUT METER`) positioned vertically below the mixers**:

```text
+-----------------------------------------------------------------------------------+
|  Brushed Gunmetal Chassis (#1D2026) + Flush Glass Overlay + Crimson Edge-Glow     |
|  +--------------------------------------------------------+--------------------+  |
|  |         [ <   PLUGIN // PRESET READOUT PILL   > ]      |  INPUT  MIX   OUT  |  |
|  +--------------------------------------------------------+   | |   | |   | |  |  |
|  |                                                        |   | |   | |   | |  |  |
|  |       MAIN CENTRAL WAVEFORM VISUALIZER (#090A0D)       |   | |   | |   | |  |  |
|  |     Glowing Crimson Real-Time Audio Waveform + dB Grid |  -===-  | |  -===- |  |
|  |                                                        |   | |  -===-  | |  |  |
|  +--------------------------------------------------------+   | |   | |   | |  |  |
|  |    ( ( DRIVE ) )    ( ( TONE ) )     ( ( COMP/DECAY ) )|   | |   | |   | |  |  |
|  |      Knurled          Knurled            Knurled       |  0.0dB  100%  0.0dB|  |
|  |                                                        +--------------------+  |
|  |   [ BYPASS ]    [ FILTER ]    [ COMP ]    [ LIMITER ]  | OUT METER  -1.8 dB |  |
|  |                                                        | L [==========|   ] |  |
|  |                                                        | R [=========|    ] |  |
|  |                                                        |  -48 -24 -12 -6 0  |  |
|  +--------------------------------------------------------+--------------------+  |
|  <------------- Left / Main Workspace -------------------> <- Full-Height ----->  |
|                                                               Right Mixer Bay     |
+-----------------------------------------------------------------------------------+
```

### B. Reference `resized()` Implementation for the Full-Height Right-Side Mixer Bay + Horizontal Output Meter Below Mixers

```cpp
// Inside the 1180x720 ContentComponent::resized():
auto unitBounds  = getLocalBounds().reduced (18, 16);
auto metalBase   = unitBounds.reduced (5);
auto glassPanel  = metalBase.reduced (6);
auto innerGlass  = glassPanel.reduced (18, 16);

// 1. Carve out the Right-Side Mixer Bay FIRST so it spans the FULL HEIGHT of the component
constexpr int mixerBayWidth = 204;
auto rightMixerBay = innerGlass.removeFromRight (mixerBayWidth);
innerGlass.removeFromRight (14); // Gap + vertical divider clearance

// 1a. Carve out the Horizontal Output Meter vertically BELOW the mixer sliders
constexpr int outputMeterHeight = 72;
auto outputMeterArea = rightMixerBay.removeFromBottom (outputMeterHeight);
outputMeter.setBounds (outputMeterArea.reduced (6, 4));

rightMixerBay.removeFromBottom (8); // Separation gap between mixer sliders and horizontal Output meter below

// 1b. Lay out Input, Wet/Dry Mix, and Output vertical sliders across the upper section of rightMixerBay
const int numMixerSliders = 3; // 0: Input, 1: Wet/Dry Mix, 2: Output
const int colWidth = rightMixerBay.getWidth() / numMixerSliders;
for (int i = 0; i < numMixerSliders; ++i)
{
    auto col = rightMixerBay.removeFromLeft (i == numMixerSliders - 1 ? rightMixerBay.getWidth() : colWidth)
                            .reduced (4, 0);
    auto titleArea = col.removeFromTop (22);
    auto valueArea = col.removeFromBottom (22);
    mixerTitleLabels[i]->setBounds (titleArea);
    mixerValueLabels[i]->setBounds (valueArea);
    mixerSliders[i]->setBounds (col); // Occupies the full vertical slider span above the horizontal Output meter
}

// 2. Lay out the Left / Main Workspace (Top Pill Readout, Central Waveform, Knobs & Tactile Buttons)
auto topHeaderRow = innerGlass.removeFromTop (38);
headerPillBar.setBounds (topHeaderRow.withSizeKeepingCentre (360, 32));

innerGlass.removeFromTop (12);
auto bottomControlsArea = innerGlass.removeFromBottom (250);
innerGlass.removeFromBottom (14);

// Central Waveform Visualizer occupies the remaining middle expanse
waveformDisplay.setBounds (innerGlass);

// Bottom deck: Low-Profile Knurled Metal Knobs + Tactile Buttons row
auto buttonRow = bottomControlsArea.removeFromBottom (42);
bottomControlsArea.removeFromBottom (10);
auto knobRow   = bottomControlsArea;
```

### C. Resizable Editor Scaling Pattern

Always lay out `PluginEditor` in a fixed design coordinate system (`1180 x 720`) inside a child content component, and scale it smoothly in `AudioProcessorEditor::resized()`:

```cpp
constexpr int designWidth = 1180;
constexpr int designHeight = 720;

// In Editor constructor:
setResizable (true, true);
setResizeLimits (885, 540, 1770, 1080);
if (auto* c = getConstrainer())
    c->setFixedAspectRatio (static_cast<double> (designWidth) / static_cast<double> (designHeight));
setSize (designWidth, designHeight);

// In Editor::resized():
const float scale = juce::jmin (static_cast<float> (getWidth()) / static_cast<float> (designWidth),
                                static_cast<float> (getHeight()) / static_cast<float> (designHeight));
const float offsetX = (static_cast<float> (getWidth()) - static_cast<float> (designWidth) * scale) * 0.5f;
const float offsetY = (static_cast<float> (getHeight()) - static_cast<float> (designHeight) * scale) * 0.5f;
content->setBounds (0, 0, designWidth, designHeight);
content->setTransform (juce::AffineTransform::scale (scale).translated (offsetX, offsetY));
```

---

## 6. Cross-Platform GitHub Actions CI & Release Publishing (Windows, Linux, macOS)

When creating or updating `.github/workflows/build.yml` and `CMakeLists.txt`, read `references/github-actions-ci-release.md` and follow these mandatory rules:

1. **Hybrid Local (`external/JUCE`) vs. CI (`FetchContent`) Resolution in `CMakeLists.txt`**:
   - Check `if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")` first so local offline builds in `dsh` use the symlinked/copied JUCE 8.0.8 tree, and fall back to `FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.8 GIT_SHALLOW TRUE)` on GitHub Actions runners.
   - Ensure `.gitignore` excludes `external/`, `deps/`, `build*/`, `dist/`, `stage*/`, `*.zip`, and `*.sha256`.
   - Do **not** define `JUCE_DISPLAY_SPLASH_SCREEN=0` in `CMakeLists.txt`.
2. **Three OS Build & Test Jobs**:
   - **Windows x64 (`runs-on: windows-2022`)**: Pin `windows-2022`, configure with `-A x64 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded`, run DSP & Host tests, and package `.vst3` + Standalone `.exe` into `MidSide670-Windows-x64-v<VERSION>.zip` + `.sha256`.
   - **Linux x64 (`runs-on: ubuntu-latest`)**: Install ALSA, X11, FreeType, and Fontconfig `-dev` packages, build with `-G Ninja -DCMAKE_BUILD_TYPE=Release`, run DSP & Host tests, strip debug symbols (`strip --strip-unneeded`), and package `MidSide670-Linux-x64-v<VERSION>.zip` + `.sha256`.
   - **macOS Universal (`runs-on: macos-14`)**: Configure a Universal Binary (`-G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0"`), ad-hoc sign (`codesign --force --deep --sign -`) both `*.vst3` and `*.app` before running `MidSide670HostSmokeTest`, and package with `ditto -c -k --sequesterRsrc` into `MidSide670-macOS-Universal-v<VERSION>.zip` + `.sha256`.
3. **Automated GitHub Release Job (`publish-release`)**:
   - Depends on `needs: [ windows-x64, linux-x64, macos-universal ]`.
   - Triggers when pushing a version tag (`v*`) or via `workflow_dispatch` (`inputs.release_tag != ''`), downloads all artifacts (`actions/download-artifact@v4`), and publishes the GitHub Release via `softprops/action-gh-release@v2` with `permissions: contents: write`.

---

## 7. Local Verification Checklist (Mandatory Before Completion)

Every feature or UI iteration in DeepSeek Harness must pass all four automated checks:

1. **Build Release Binaries**:
   ```bash
   ./scripts/build.sh
   ```
2. **Run Offline DSP Unit Tests**:
   ```bash
   ./build/*Tests_artefacts/Release/*Tests
   ```
3. **Run VST3 Host Smoke Test**:
   ```bash
   ./build/*HostSmokeTest_artefacts/Release/*HostSmokeTest dist/*.vst3
   ```
4. **Render, Inspect (`read_image`), and Present (`present`) 2x UI Screenshot**:
   ```bash
   ./build/*Screenshot_artefacts/Release/*Screenshot --out=dist/midside670-ui.png
   ```
   - Call `read_image` on `dist/midside670-ui.png` to verify the brushed gunmetal base, flush screwless glass overlay with crimson LED edge-glow, pill header readout, central glowing red waveform display, low-profile knurled metal knobs, ultra-thin vertical glass sliders, thin red-bordered tactile buttons, and zero text clipping/overlap.
   - Call `present` with `/home/brians/bls/sound/MidSide-670/dist/midside670-ui.png` and include `![MidSide-670 UI Preview](<dist/midside670-ui.png>)` in the final reply.
