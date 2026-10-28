# SlapLay

**Tape-style slapback delay for guitar.** A short, playable delay (50-200 ms) with a
degraded feedback loop: 12 dB/octave low cut and high cut, tanh tape saturation, and
wow & flutter that modulates the fractional read pointer like a slipping tape transport.

Built with **JUCE 8.0.8** as a **VST3 + Standalone** plug-in for Windows x64, Linux x64,
and macOS Universal (arm64 + x86_64).

---

## Sound engine

```
guitar in ──► write head ──► cubic fractional read ──► tape saturation ──► wet
                  ▲                                          │
                  │                                          ▼
                  └── feedback gain ◄── DC block ◄── low cut ◄── high cut
```

| Stage | Implementation |
| :--- | :--- |
| **Short delay line** | Stereo delay with 4-point Catmull-Rom cubic interpolation. The delay time covers **50.0 - 200.0 ms** in 0.1 ms steps and glides with a 60 ms smoothing time so time changes feel like tape. |
| **High-cut / low-cut** | Two cascaded one-pole high-pass sections and two cascaded one-pole low-pass sections = **12 dB/octave** each. The tone stack lives **only in the feedback path**, so the first repeat is raw tape and each subsequent repeat gets progressively darker and thinner. A permanent 12 Hz DC blocker keeps the loop stable. |
| **Tape saturation** | `tanh(x * drive) / drive` with `drive = 1 + 4 * amount`, blended from the clean tap by the TAPE DRIVE control. The curve has unity small-signal gain, compresses peaks, and adds odd-harmonic warmth. |
| **Wow & flutter** | Two free-running sine LFOs - **wow 0.55 Hz** (up to +/-0.6 % speed deviation) and **flutter 6.3 Hz** (up to +/-0.25 %) - multiply the delay length. Because the deviation is proportional to delay time, longer repeats warble more, exactly like tape. |

The engine is **zero-latency** and allocation-free in `processBlock()`. Parameter changes
are smoothed, and the UI waveform is fed from a lock-free min/max ring buffer.

---

## Controls

| Control | Range | Default | Notes |
| :--- | :--- | :--- | :--- |
| **Delay Time** | 50 - 200 ms | 110 ms | Fractional tape delay length |
| **Feedback** | 0 - 90 % | 36 % | Bounded by the saturator in the loop |
| **Tape Drive** | 0 - 100 % | 38 % | Soft clipping on the delayed signal |
| **Low Cut** | 40 - 900 Hz | 150 Hz | 12 dB/oct in the feedback loop |
| **High Cut** | 1.0 - 12 kHz | 3.8 kHz | 12 dB/oct in the feedback loop |
| **Wow** | 0 - 100 % | 22 % | Slow 0.55 Hz speed drift |
| **Flutter** | 0 - 100 % | 12 % | Fast 6.3 Hz speed shimmer |
| **Input** | -18 - +18 dB | 0 dB | Right mixer bay |
| **Wet / Dry** | 0 - 100 % | 32 % | Right mixer bay |
| **Output** | -18 - +18 dB | 0 dB | Right mixer bay |

**Tactile switches:** `FILTER` (feedback EQ), `TAPE` (saturation), `MOD` (wow/flutter),
`BYPASS` (dry, zero latency).

Presets can be stepped with the `<` / `>` chevrons in the top pill readout or selected
from its popup menu. Ten factory presets range from *Rockabilly '59* to *Ghost Echo*.

---

## UI

The editor is a fixed **1180 x 720** design canvas with smooth proportional scaling. It
renders a brushed gunmetal desktop unit on a dark studio surface, a flush screwless glass
overlay with a muted crimson LED edge-glow, a top pill preset readout, a large live
crimson waveform screen, low-profile knurled metal knobs, red-bordered tactile switches,
and a **full-height right mixer bay** with `INPUT`, `WET / DRY`, `OUTPUT` glass sliders
above a horizontal `OUT METER` with L/R peak-hold bars and a dB scale.

---

## Build (Linux, offline)

Dependencies are reused from the sibling `plugin_poc` checkout. No network access is
needed locally.

```bash
./scripts/setup-linux.sh     # symlinks external/JUCE and deps/sysroot
./scripts/build.sh           # Release build into ./build and ./dist
```

The build creates:

- `dist/SlapLay.vst3`
- `dist/SlapLay` (Standalone)
- `build/SlapLayTests_artefacts/Release/SlapLayTests`
- `build/SlapLayHostSmokeTest_artefacts/Release/SlapLayHostSmokeTest`
- `build/SlapLayScreenshot_artefacts/Release/SlapLay Screenshot`

### Verification

```bash
./build/SlapLayTests_artefacts/Release/SlapLayTests
./build/SlapLayHostSmokeTest_artefacts/Release/SlapLayHostSmokeTest dist/SlapLay.vst3
./build/SlapLayScreenshot_artefacts/Release/"SlapLay Screenshot" --out=dist/slaplay-ui.png
```

The offline DSP suite covers fractional-delay accuracy at 50/200 ms, the 12 dB/octave
feedback EQ response, saturation boundedness and harmonic generation, wow/flutter read
pointer swing, echo timing and feedback decay, maximum-feedback stability, bypass
transparency, state round-trips, and mono/stereo layouts.

---

## Project layout

```
Source/
  Parameters.h / Presets.h
  PluginProcessor.*        APVTS, gain staging, meters, waveform FIFO
  PluginEditor.*           1180x720 fixed-canvas editor + full-height mixer bay
  dsp/
    FractionalDelayLine.h  cubic Hermite stereo delay line
    FeedbackEQ.h           cascaded one-pole 12 dB/oct HP + LP + DC blocker
    TapeSaturation.h       tanh soft clipper with unity small-signal gain
    WowFlutter.h           0.55 Hz wow and 6.3 Hz flutter LFOs
    SlapbackEngine.h       complete delay voice
  ui/
    Theme.*, StudioLookAndFeel.*, Controls.*, WaveformDisplay.*
Tests/DspTests.cpp
Tools/HostSmokeTest.cpp, Tools/RenderEditor.cpp
scripts/setup-linux.sh, scripts/build.sh
.github/workflows/build.yml
```

---

## License

The bundled Lato fonts are licensed under the SIL Open Font License; see
`Resources/Lato-OFL.txt`.
