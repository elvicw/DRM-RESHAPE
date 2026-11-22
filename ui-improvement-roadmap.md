PROJECT: DRM-RESHAPE
COMPANY: YEAHWAAH AUDIO
TASK: Complete visual and interaction redesign of the existing audio plugin.

==================================================
01. DESIGN PHILOSOPHY
==================================================

Transform DRM-RESHAPE into a professional vintage-inspired
studio hardware instrument.

The design should take inspiration from late-1970s through
early-2000s analog studio rack equipment, particularly
vintage FET compressors and professional analog equalizers.

Use the physical design language of equipment such as
the UREI 1176 as inspiration, without copying its exact
appearance, branding, or trade dress.

The interface must communicate:
- Physical hardware.
- Professional studio equipment.
- Engineering precision.
- Analog character.
- Mechanical controls.
- Functional clarity.

This is NOT a generic dark audio plugin redesign.

This is NOT a modern dashboard with vintage colors.

The objective is to make the plugin look and feel like
a physical rack-mounted studio processor.

==================================================
02. OVERALL VISUAL IDENTITY
==================================================

FACEPLATE:
- Matte charcoal-black.
- Subtle brushed-metal finish.
- Restrained surface texture.
- Dark metallic secondary panels.
- Thin engraved-looking separators.
- Minimal metallic screws and fasteners.
- Square or minimally rounded geometry.

COLOR PALETTE:

Main faceplate:       #202124
Secondary panels:     #292B2D
Recessed displays:    #111315
Primary text:         #D8D0BD
Secondary text:       #85847C
Amber indicators:     #D69A38
Steel detailing:      #73777A
Muted blue:           #547A91
Muted green:          #82916B

Use amber sparingly.

Do not use excessive gradients, neon colors,
glowing borders, or decorative lighting.

The interface should have consistent lighting direction
and believable physical depth.

==================================================
03. PHYSICAL ROTARY KNOBS
==================================================

CRITICAL REQUIREMENT:

Physical rotary knobs must be the defining visual
characteristic of DRM-RESHAPE.

Do not use flat circles with a white line.

Do not use emoji, Unicode characters, or generic
circular UI elements as knob substitutes.

Create convincing photorealistic representations
of physical analog potentiometers.

KNOB CONSTRUCTION:

Each knob should feature:

1. Metallic mounting collar.
2. Beveled outer housing.
3. Realistic contact shadow.
4. Cylindrical body with visible depth.
5. Textured or ribbed sidewalls.
6. Matte black bakelite material.
7. Subtle top-surface reflections.
8. Contrasting physical pointer.
9. Graduated markings around the control.
10. Realistic highlights and shading.

Create three reusable knob styles:

A. SKIRTED VINTAGE KNOB
- Wider base.
- Black body.
- Cream-colored pointer.
- Graduated scale.
- For frequency and gain controls.

B. RIBBED BAKELITE KNOB
- Textured sidewalls.
- Dark matte finish.
- Visible pointer.
- For secondary parameters such as Q
  and other existing processing controls.

C. METAL-CAPPED KNOB
- Metallic or brushed-metal top.
- Black sidewall.
- Contrasting position indicator.
- For global controls such as output and mix,
  where applicable.

All knobs must share consistent material treatment,
lighting direction, perspective, and proportions.

Use different physical sizes to establish hierarchy.

Suggested starting dimensions:

Primary controls:   54-64 px
Secondary controls: 42-48 px
Global controls:    58-68 px

Adjust dimensions to fit the actual editor.

INTERACTION:

Preserve existing JUCE slider and parameter behavior.

Support:
- Rotary drag interaction.
- Fine adjustment using Shift.
- Double-click reset to the existing parameter default.
- Mouse wheel adjustment where supported.
- Hover highlighting.
- Tactile visual response while dragging.
- Readable numerical values.

The pointer must always reflect the actual parameter value.

==================================================
04. MECHANICAL SWITCHES AND BUTTONS
==================================================

Replace generic modern buttons with hardware-inspired
mechanical controls.

VISUAL CHARACTERISTICS:

- Physical switch housing.
- Slightly recessed mounting.
- Metallic or dark plastic construction.
- Realistic shadows.
- Mechanical ON/OFF positions.
- Restrained amber indicator illumination.

Apply to existing controls such as:

- EQ band enable/disable.
- TIGHT.
- SMOOTH.
- Bypass.
- Other existing processing switches.

Use different sizes according to importance.

Do not create decorative switches that do not control
actual plugin functionality.

Active states should be visually obvious without
excessive illumination.

==================================================
05. FACEPLATE AND CHASSIS
==================================================

The interface should appear to be one continuous
physical rack processor.

Do not organize everything into floating dashboard cards.

Use:

- Continuous dark metal faceplate.
- Subtle brushed texture.
- Recessed analyzer display.
- Physical-looking control mounting.
- Thin metallic section dividers.
- Engraved technical labels.
- Small model identification markings.
- Restrained mounting screws.

Suggested branding:

YEAHWAAH AUDIO
DRM-RESHAPE
PROFESSIONAL DRUM PROCESSOR

Use branding as part of the hardware design,
not as a giant decorative heading.

==================================================
06. ANALOG METERING
==================================================

The final design should combine physical analog-style
metering with the existing digital spectrum analyzer.

Investigate which actual signal measurements are
available in the current DSP architecture.

Potential meter functions:

- Input level.
- Output level.
- Gain reduction, if actual gain-reduction data exists.

METER APPEARANCE:

- Physical analog meter housing.
- Black meter face.
- Cream-colored scale.
- Amber or warm-white illumination.
- Realistic needle.
- Subtle glass reflection.
- Mechanical mounting details.

The needle must represent actual signal measurements.

Do not fabricate gain reduction, input level,
or output level.

If the DSP does not expose a measurement,
document the limitation instead of inventing one.

Keep meters readable at the intended plugin size.

==================================================
07. DIGITAL SPECTRUM ANALYZER
==================================================

The analyzer should look like a professional
measurement instrument embedded into the hardware.

APPEARANCE:

- Deep black recessed display.
- Fine grey frequency grid.
- Clear dB markings.
- Subtle screen illumination.
- Professional technical labeling.

VISUAL ELEMENTS:

- Actual EQ response curve.
- Existing pre-processing FFT trace.
- Existing post-processing FFT trace.
- Interactive EQ nodes.
- Selected-band indication.

Use restrained trace colors:

EQ response:     Amber
Pre-processing:  Muted blue
Post-processing: Muted green

Preserve actual frequency response calculations
and FFT visualization.

Do not replace accurate DSP-derived visualization
with decorative curves.

Preserve existing analyzer functionality.

==================================================
08. TYPOGRAPHY AND LABELING
==================================================

Use two complementary typography systems.

INDUSTRIAL TYPOGRAPHY:

Suitable examples:
- DIN-style fonts.
- Roboto Condensed.
- Other professional condensed sans-serif fonts.

Use for:
- Section titles.
- Instrument names.
- Parameter labels.
- Hardware markings.

TECHNICAL TYPOGRAPHY:

Suitable example:
- IBM Plex Mono.

Use for:
- Frequency values.
- Gain values.
- Q values.
- Input/output readings.
- Numerical measurements.

STYLE:

- Uppercase section labels.
- Warm ivory primary text.
- Muted grey secondary text.
- Amber active indicators.
- Clear numerical hierarchy.

Suggested technical markings:

INPUT
OUTPUT
FREQUENCY
GAIN
Q
PROCESSING
TIGHT
SMOOTH
MODEL DRM-01

Avoid excessive labels and unnecessary decoration.

==================================================
09. MAIN INTERFACE ORGANIZATION
==================================================

Organize the existing functionality into a cohesive
hardware-style control layout.

Suggested hierarchy:

A. HEADER
- YEAHWAAH AUDIO branding.
- DRM-RESHAPE identification.
- Preset management.
- Existing global controls.
- Bypass and power indication, as appropriate.

B. INSTRUMENT SELECTION
- Kick.
- Snare.
- Toms.
- Overheads.
- Room.
- Full Kit.

Use physical-style selection switches.

Preserve existing instrument-selection behavior.

C. EQUALIZER SECTION
- Preserve the existing number of EQ bands.
- Preserve existing frequency controls.
- Preserve existing gain controls.
- Preserve existing Q controls.
- Preserve band enable/disable behavior.

Use physical rotary knobs and mechanical switches.

D. ANALYZER
- Large recessed digital spectrum display.
- Existing FFT functionality.
- Existing EQ response visualization.

E. TONE CHARACTER
- TIGHT.
- SMOOTH.
- Existing associated controls.

F. OUTPUT / MIX
- Existing wet/dry functionality.
- Existing output controls.
- Actual available metering.

Do not invent additional DSP parameters.

Do not remove existing controls simply to
simplify the visual layout.

==================================================
10. LAYOUT AND DENSITY
==================================================

The overall interface should be dense, similar to
professional analog rack equipment.

Suggested initial design canvas:

Approximately 1050 x 650 px.

Treat this as a starting point, not a fixed requirement.

Prioritize:
- Clear control grouping.
- Readable labels.
- Comfortable knob spacing.
- Large enough analyzer display.
- Consistent alignment.
- Minimal unused space.

Avoid:
- Oversized decorative branding.
- Excessive empty space.
- Tiny controls.
- Modern dashboard-style cards.
- Inconsistent component spacing.

The interface should feel engineered and purposeful.

==================================================
11. JUCE IMPLEMENTATION ARCHITECTURE
==================================================

First inspect the existing source code.

Determine the current:
- Plugin editor architecture.
- Component hierarchy.
- LookAndFeel implementation.
- Parameter attachments.
- Analyzer implementation.
- Metering implementation.
- Rendering approach.

Do not assume the existing architecture matches
a proposed folder structure.

Adapt the implementation to the existing project.

Where appropriate, create reusable components:

UI/Theme/
    ColorPalette
    Typography
    HardwareLookAndFeel

UI/Components/
    VintageKnob
    HardwareToggle
    IndicatorLamp
    AnalogMeter
    SectionPanel

UI/Analyzer/
    SpectrumDisplay

UI/Layout/
    MainEditorLayout

Use JUCE custom LookAndFeel and vector rendering
where appropriate.

For photorealistic controls, evaluate:

OPTION A:
High-quality embedded knob sprite strips.

OPTION B:
Custom vector drawing with layered shading,
metallic highlights, shadows and realistic depth.

OPTION C:
A hybrid approach combining pre-rendered
photorealistic knob artwork with vector overlays.

Choose the approach that provides the best balance
of visual realism, responsiveness and maintainability.

If using image assets:
- Embed assets in the application binary.
- Do not require external runtime asset files.
- Ensure all assets are appropriately licensed
  or original.
- Support the intended scaling and display sizes.

Do not introduce unnecessary third-party dependencies.

==================================================
12. DSP AND FUNCTIONAL SAFETY
==================================================

THIS IS A UI REDESIGN.

Do not modify the DSP unless explicitly required
and separately approved.

STRICT REQUIREMENTS:

- Preserve DSP algorithms.
- Preserve parameter IDs.
- Preserve parameter ranges and units.
- Preserve automation compatibility.
- Preserve preset compatibility.
- Preserve state serialization.
- Preserve existing instrument behavior.
- Preserve analyzer calculations.
- Preserve audio-thread safety.
- Preserve existing plugin formats and compatibility.

Do not replace functional components with decorative
graphics.

All visual controls must remain connected to their
actual parameters.

Host automation must update the corresponding
visual controls.

Preset loading must update the interface correctly.

Avoid blocking the audio thread with GUI operations.

Use appropriate synchronization for meter and analyzer
data exchanged between audio and GUI threads.

==================================================
13. VISUAL QUALITY CONTROL
==================================================

A technically successful build is not automatically
a visually successful redesign.

Use visual checkpoints.

CHECKPOINT 1:
Create a standalone prototype containing:

- Dark faceplate.
- Three knob styles.
- Mechanical switch.
- Indicator lamp.
- Analog meter.
- Recessed analyzer mockup.

Render a screenshot at intended display dimensions.

Inspect the physical realism.

CHECKPOINT 2:
Refine knobs and hardware components.

Do not proceed until the knobs genuinely resemble
physical analog potentiometers.

CHECKPOINT 3:
Build the complete visual layout using the established
hardware design system.

Render screenshots and inspect alignment,
proportions, spacing and readability.

CHECKPOINT 4:
Integrate existing parameter attachments and
functional controls.

CHECKPOINT 5:
Build the plugin and validate all functionality.

CHECKPOINT 6:
Inspect the actual plugin inside a supported host.

Test different window sizes if resizing is supported.

==================================================
14. ACCEPTANCE CRITERIA
==================================================

The redesign is complete only when:

[ ] The interface resembles professional vintage
    studio hardware.

[ ] Rotary knobs have convincing physical depth,
    material and lighting.

[ ] Three distinct knob styles are implemented.

[ ] Knob indicators accurately reflect parameter values.

[ ] Mechanical switches have clear physical states.

[ ] The faceplate has a restrained metal finish.

[ ] Typography is consistent and readable.

[ ] Analog meters represent real available signals.

[ ] Digital analyzer remains accurate and functional.

[ ] Existing instrument selections work.

[ ] All existing EQ controls work.

[ ] TIGHT and SMOOTH retain their existing behavior.

[ ] Presets and state serialization work.

[ ] Host automation works.

[ ] No unintended DSP changes were introduced.

[ ] Plugin builds successfully.

[ ] Actual plugin screenshots have been inspected.

[ ] No major visual inconsistencies remain.

==================================================
15. EXECUTION WORKFLOW
==================================================

Do not implement the entire redesign in one pass.

Follow these phases:

PHASE 1:
Audit the current project and document the existing
UI and DSP architecture.

PHASE 2:
Establish the faceplate, colors, typography and
hardware design system.

PHASE 3:
Develop the three photorealistic knob styles.

PHASE 4:
Develop mechanical switches and indicator lamps.

PHASE 5:
Develop analog meters and the recessed analyzer.

PHASE 6:
Build the complete interface layout.

PHASE 7:
Integrate existing functional controls.

PHASE 8:
Build and validate the plugin.

PHASE 9:
Perform visual refinement based on actual screenshots.

After every phase, report:

- What was implemented.
- Files created or modified.
- Design decisions.
- Build status.
- Outstanding problems.
- Recommended next steps.

Do not silently skip a phase.

Do not claim visual validation without actually
inspecting a rendered screenshot.

The final result must be a cohesive, professional,
vintage-inspired physical studio processor interface.

END OF SPECIFICATION.