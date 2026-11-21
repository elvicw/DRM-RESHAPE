# DRM-RESHAPE

**DRM-RESHAPE** is a drum-focused equalizer and tone shaper by **yeahwaah audio**. It uses an instrument-first workflow with editable controls for Kick, Snare, Toms, Overheads, Room, and Full Kit.

## Build

The project uses CMake and downloads JUCE 8.0.6 during configuration.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The plugin formats are VST3 and Standalone. The GitHub Actions workflow builds Windows x64 artifacts. macOS and AU targets are intentionally deferred.

## DSP architecture

- `DrumShaperDSP` owns a fixed six-stage minimum-phase biquad cascade and per-channel filter state. Frequency response computation is shared with the editor display.
- `DRMRESHAPEAudioProcessor` owns parameters, state serialization, smoothing, host I/O, metering, and wet/dry routing.
- `DRMRESHAPEAudioProcessorEditor` owns the controls and response visualization. Filter state is not owned by the GUI.
- EQ filtering has no intentional latency and does not use oversampling.
- `Tests/DSPTests.cpp` covers selected response values, finite impulse behavior, and repeated coefficient changes.

The instrument selection applies a starting-point frequency set; band frequencies and Q remain editable. TIGHT and SMOOTH can use frequency-selective bandpass detectors, RMS-like envelope followers, and attack/release smoothing to apply extra bell attenuation. Their threshold, timing, and maximum reduction settings are available in the host and the advanced panel. The display combines the filter response with pre/post FFT traces. The analyzer uses a fixed 2048-sample window and lock-free atomic spectrum bins.

## Development status

This is an incremental first implementation. Static and dynamic EQ, instrument defaults, parameter/state support, wet/dry, FFT display, and basic DSP checks are implemented. The validated local build is macOS VST3/Standalone; Windows VST3/Standalone validation is delegated to GitHub Actions. Linear phase and AU are out of scope for this Windows-first release.
