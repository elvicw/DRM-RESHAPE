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
- `DrumShaperCharacter` is a separate post-EQ stage with a stereo-linked wideband compressor macro and a normalized `tanh` saturation drive. Both controls are smoothed; the compressor's measured gain reduction is published for the analog meter.
- `DRMRESHAPEAudioProcessor` owns parameters, state serialization, smoothing, host I/O, metering, and wet/dry routing.
- `DRMRESHAPEAudioProcessorEditor` owns the hardware-style controls and response visualization. Custom JUCE LookAndFeel drawing supplies three rotary treatments and mechanical toggles. Filter state is not owned by the GUI.
- EQ filtering has no intentional latency and does not use oversampling.
- Saturation currently uses a low-cost memoryless waveshaper without oversampling; high drive can produce aliasing on bright material. The linear EQ remains separate and unchanged.
- `Tests/DSPTests.cpp` covers selected response values, finite impulse behavior, repeated coefficient changes, dynamic reduction, compressor attenuation, saturation behavior, and a bounded CPU run.

The instrument selection applies a starting-point frequency set; band frequencies and Q remain editable. TIGHT and SMOOTH can use frequency-selective bandpass detectors, RMS-like envelope followers, and attack/release smoothing to apply extra bell attenuation. Their threshold, timing, and maximum reduction settings are available in the host and the advanced panel. The display combines the filter response with pre/post FFT traces. The analyzer uses a fixed 2048-sample window and lock-free atomic spectrum bins.

The **Compression** macro ranges from 0–100%, mapping to a 1:1–8:1 ratio above a fixed −18 dBFS threshold with 5 ms attack and 100 ms release. Detection is stereo-linked. **Drive** ranges from 0–24 dB and feeds a normalized `tanh` waveshaper; its output is normalized at full scale, while the global output and wet/dry controls remain available for level matching.

## Development status

The editor now uses a charcoal hardware faceplate, recessed analyzer, physical-style rotary controls, mechanical toggles, and actual input/output/compressor-reduction meters. The new Compression and Drive parameters are stored in APVTS state and exposed to host automation. Static/dynamic EQ, instrument defaults, wet/dry, FFT display, and DSP checks remain implemented. The current local build is macOS VST3/Standalone; Windows VST3/Standalone validation is delegated to GitHub Actions. Linear phase and AU are out of scope for this Windows-first project.
