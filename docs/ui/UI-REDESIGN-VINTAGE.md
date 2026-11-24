
# DRM-RESHAPE — Vintage UI Redesign

## Objective

Redesign the DRM-RESHAPE plugin interface to closely reproduce the supplied vintage rack-hardware visual reference.

This is a **UI-only improvement**. The existing audio engine, DSP behavior, parameter meanings, preset behavior, automation compatibility, and plugin stability must not be changed as part of this work.

The target is to get the implementation **as visually close to the reference image as the existing framework and platform allow**.

Do not treat the image as loose inspiration. Inspect it and use it as the primary visual specification throughout implementation and review.

---

# 1. Mandatory Visual Reference

Reference image in the repository:

```text
docs/ui/drm-reshape-vintage-reference.png
```

Before changing any UI code:

1. Confirm that the reference image exists at the path above.
2. Open and inspect the actual image.
3. Do not rely only on this document's description.
4. Compare the current plugin UI against the reference.
5. Identify the UI framework, existing component structure, drawing system, and technical constraints.
6. Use the reference image as the primary visual target for layout, proportions, spacing, typography, colors, materials, controls, and hierarchy.

If the image is missing, do not guess or silently proceed with a substitute. Report the missing file and ask for it to be added.

## Visual Target

The reference depicts a vintage studio rack unit.

The overall appearance should feature:

- Dark, aged charcoal/black metal chassis.
- Subtle distressed metal texture.
- Recessed panels and visible mounting screws.
- Warm cream/off-white typography and markings.
- Restrained amber and red indicator lighting.
- Large machined-metal rotary knobs.
- Clear knob pointer positions and graduated markings.
- Black meter faces with red digital seven-segment readouts.
- A prominent analyzer with a dark background.
- Red/orange spectrum visualization.
- Clear frequency markings and dB scales.
- Clear separation between the header, analyzer, tone controls, dynamics/drive controls, and footer.
- A cohesive physical-hardware appearance rather than a modern flat software dashboard.

Avoid:

- Glossy modern gradients.
- Excessive decorative effects.
- Tiny illegible labels.
- Unnecessary borders.
- Generic modern toggle controls.
- Excessive visual clutter.
- Flat, generic software-dashboard styling.

The result should look like a piece of vintage professional studio hardware.

---

# 2. Required Control Behavior and Presentation

## 2.1 Always-Active Tone Controls

The following controls must remain active at all times.

They must no longer have individual on/off toggle switches:

- LOW
- MID
- HIGH
- TIGHT
- SMOOTH
- AIR

Present each as a rotary knob with its label and a readable value display.

Do not add replacement switches, checkboxes, or hidden enable buttons for these controls.

## 2.2 Level Comp

Level Comp must be the **only dedicated on/off toggle control** in the custom plugin UI.

Requirements:

- Use a physical-looking illuminated push button.
- Match the vintage hardware appearance of the reference.
- The words LEVEL COMP must be integrated into the button itself or directly into its button assembly.
- The label must not look like a detached generic caption.
- Make enabled and disabled states visually unmistakable.
- Use restrained red/orange illumination for the active state.
- Preserve the existing Level Comp DSP behavior and parameter mapping.

The button should feel like a physical switch found on vintage studio equipment.

## 2.3 Bypass

The reference image includes a BYPASS control, but the requested UI behavior is to have no additional toggle buttons.

Level Comp is the only dedicated toggle in the custom UI.

Therefore:

- Do not retain or introduce a second custom bypass toggle.
- Do not delete or damage the plugin's underlying bypass capability.
- Preserve host bypass behavior.
- Preserve existing parameter and automation compatibility.

First inspect how bypass currently works.

If the plugin has a required custom bypass function that cannot be replaced by the host's standard bypass mechanism, stop and report the conflict before making a destructive change.

Do not silently remove functionality or break existing sessions.

## 2.4 Other Rotary Controls

Use rotary knobs for:

- INPUT
- OUTPUT
- MIX
- COMPRESSION
- DRIVE
- LOW
- MID
- HIGH
- TIGHT
- SMOOTH
- AIR

Requirements:

- MIX remains a continuous mix control.
- Do not add an on/off switch for MIX.
- Preserve existing parameter ranges.
- Preserve existing parameter defaults.
- Preserve existing parameter scaling.
- Preserve existing value formatting unless a verified UI-only formatting adjustment is necessary.
- Make knobs large enough to be easily identified and operated.
- Use consistent pointer, tick-mark, and value-display styling.
- Provide useful value feedback without cluttering the panel.

All rotary controls should feel like they belong to the same physical hardware unit.

---

# 3. Meter Redesign

Replace the current analog-style meter presentation with the digital meter treatment shown in the reference.

## Appearance

- Black meter faces.
- Red seven-segment/digital-clock-style numerical readouts.
- Warm red/orange LED-style activity indicators where appropriate.
- Clear INPUT, OUTPUT, and GR labels.
- Recessed meter housings.
- Consistent spacing and alignment.

## Functional Requirements

- Preserve the existing meter data.
- Preserve measurement ranges.
- Preserve meter ballistics.
- Preserve update behavior.
- Ensure the readouts remain legible at normal and minimum supported UI sizes.
- Do not introduce fake or static meter activity.
- The display must represent actual audio measurements.

The meters should look like vintage digital rack equipment, not modern floating numerical displays.

---

# 4. Analyzer Improvements

Improve the analyzer so it feels like part of the same vintage hardware unit while remaining useful and readable.

## Target Appearance

- Dark black display surface.
- Fine, low-contrast grid lines.
- Clear frequency markings across the spectrum.
- Readable dB scale.
- Red/orange spectrum bars or trace.
- Restrained glow around active spectrum elements.
- Clear distinction between PRE and POST information if both are available.
- Dedicated, visually consistent input/output indication if supported by the current analyzer architecture.

## Functional Requirements

- Preserve the current analyzer's actual signal source and data.
- Do not fabricate spectrum data.
- Do not display decorative movement as real analysis.
- Keep frequency and dB labels accurate.
- Maintain useful resolution and responsiveness.
- Avoid excessive smoothing.
- Avoid flicker.
- Avoid visual effects that make the analyzer harder to interpret.
- Keep rendering efficient.
- Do not introduce avoidable CPU/GPU overhead in the audio or UI thread.

If the current analyzer architecture does not support a feature shown in the reference, implement the closest truthful equivalent and document the limitation.

Do not change the DSP engine just to imitate a visual detail.

---

# 5. Layout and Hierarchy

Use the supplied image as the source of truth for the overall arrangement.

The intended hierarchy is:

## 5.1 Top Header

- DRM-RESHAPE identity and product subtitle.
- Preset/processing selector.
- ADVANCED control entry, retaining its existing behavior.
- Three digital meters:
  - INPUT
  - OUTPUT
  - GR

## 5.2 Analyzer Panel

- Large, prominent analyzer display.
- Clear frequency and dB scales.
- PRE/POST or equivalent analyzer selection, where supported.
- Input/output indication where supported.

## 5.3 Equalizer / Tone Panel

Six evenly aligned rotary controls:

- LOW
- MID
- HIGH
- TIGHT
- SMOOTH
- AIR

No individual on/off switches.

## 5.4 Input / Dynamics / Output Control Panel

- INPUT
- OUTPUT
- MIX
- COMPRESSION
- DRIVE
- LEVEL COMP illuminated button.

No custom bypass toggle, subject to the compatibility safeguard described earlier.

## 5.5 Footer

Retain useful technical/plugin information such as:

- Model.
- Phase mode.
- Latency.
- Stereo status.

Only display information that is accurate.

Keep footer text secondary and readable.

## Layout Requirements

Preserve the general proportions, spacing, grouping, and visual balance of the reference.

Do not simply recolor the existing layout if its geometry prevents a close match.

The redesigned UI should feel like a cohesive piece of hardware rather than several unrelated sections placed together.

---

# 6. Visual Fidelity Requirements

The goal is a close reproduction, not a loose interpretation.

Pay particular attention to:

- Panel heights and widths.
- Alignment and spacing between controls.
- Relative knob sizes.
- Meter dimensions and placement.
- Analyzer height and prominence.
- Typography size and weight.
- Capitalization and alignment.
- Cream text against dark metal.
- Red/orange meter and Level Comp illumination.
- Panel borders and separators.
- Screws and restrained surface texture.
- Consistent margins and visual rhythm.

Use textures and lighting carefully.

They must not interfere with:

- Control hit areas.
- Text clarity.
- UI scaling.
- Accessibility.
- Rendering performance.

Do not add decorative elements that are absent from or inconsistent with the reference unless they are needed to communicate an existing function.

## 6.1 Visual Comparison and Iteration

For every meaningful UI milestone:

1. Build and launch the plugin in the supported test host.
2. Capture a screenshot of the actual implemented UI.
3. Compare it side-by-side with:

   `docs/ui/drm-reshape-vintage-reference.png`

4. Identify the most visible differences in:
   - Layout.
   - Proportions.
   - Color.
   - Typography.
   - Controls.
   - Spacing.
   - Overall visual hierarchy.

5. Correct those differences before declaring the milestone complete.

Do not claim visual fidelity based only on source-code inspection.

The rendered plugin must be inspected.

If possible, use screenshot comparison techniques to evaluate layout and proportions rather than relying entirely on visual memory.

---

# 7. Non-Destructive Implementation Rules

This work must not regress the released plugin.

Unless a change is explicitly required for the UI and verified as safe:

- Do not alter DSP algorithms.
- Do not alter audio processing order.
- Do not change parameter IDs.
- Do not rename or remove existing parameters.
- Do not change parameter ranges.
- Do not change parameter defaults.
- Do not break preset loading or saving.
- Do not break host automation.
- Do not break existing project/session recall.
- Do not change plugin formats.
- Do not change plugin identifiers.
- Do not change compatibility metadata.
- Do not change sample-rate behavior.
- Do not change block-size behavior.
- Do not change latency behavior.
- Do not change phase behavior.
- Do not change stereo behavior.
- Do not introduce audio-thread allocations.
- Do not introduce audio-thread locks.
- Do not introduce blocking operations.
- Do not introduce real-time safety regressions.
- Do not remove existing functionality merely to simplify the UI.
- Do not make unrelated refactors.
- Do not perform unnecessary dependency upgrades.

If a requested visual change appears to require a change to any of these areas, stop and explain the issue before proceeding.

The existing released plugin is the baseline that must be protected.

---

# 8. Branch-Per-Milestone Workflow — Mandatory

**Every milestone or distinct implementation step must be performed on its own Git branch.**

Never perform implementation work directly on `main` or the release branch.

Start by checking:

- Repository status.
- Current branch.
- Existing uncommitted changes.
- Current project version.
- Existing branch conventions.
- Existing release conventions.

Do not overwrite, discard, reset, or modify unrelated user changes.

## 8.1 Suggested Branch Structure

The following is an example. Adapt it to the repository's actual branch conventions.

```text
main
  |
  +-- feature/vintage-ui-foundation
  |       |
  |       +-- PR
  |       +-- Review
  |       +-- Merge
  |
  +-- feature/vintage-ui-controls
  |       |
  |       +-- PR
  |       +-- Review
  |       +-- Merge
  |
  +-- feature/vintage-digital-meters
  |       |
  |       +-- PR
  |       +-- Review
  |       +-- Merge
  |
  +-- feature/vintage-analyzer
  |       |
  |       +-- PR
  |       +-- Review
  |       +-- Merge
  |
  +-- feature/vintage-ui-polish
  |       |
  |       +-- PR
  |       +-- Review
  |       +-- Merge
  |
  +-- release/<version>
          |
          +-- Final validation
          +-- Release
```

Each new milestone branch must be created from the latest appropriate merged base branch so that completed work is included.

Do not create every branch from an outdated copy of `main`.

## 8.2 Rules for Every Milestone

For every milestone:

1. Confirm the working tree and base branch.
2. Create a new, clearly named branch.
3. Make only the changes belonging to that milestone.
4. Build the project.
5. Run relevant tests.
6. Inspect the rendered plugin.
7. Capture a screenshot when the milestone affects the visible UI.
8. Review the Git diff for unrelated or destructive changes.
9. Commit the work to that milestone branch.
10. Open or prepare a PR against the appropriate base branch.
11. Review the PR for correctness, compatibility, and regressions.

Do not skip these steps merely because the change appears small.

## 8.3 Agent Authorization to Review and Merge

The agent is authorized to review and merge its own PR **only when all applicable acceptance checks pass and the change is demonstrably non-destructive**.

The agent may proceed with the merge when:

- The build succeeds.
- Relevant tests pass.
- The UI has been visually inspected.
- Existing functionality remains intact.
- Parameter compatibility is preserved.
- The Git diff contains no unrelated changes.
- No destructive changes are present.
- The implementation is consistent with the reference.

Do not merge if:

- Tests or builds fail.
- The UI has not been visually inspected.
- Existing behavior may have regressed.
- Parameter, preset, automation, or session compatibility is uncertain.
- The diff contains unrelated changes.
- The implementation departs materially from the reference without a documented technical reason.
- The agent cannot verify that the change is safe.

If a change is risky, destructive, ambiguous, or cannot be verified:

- Leave the branch intact.
- Leave the PR unmerged.
- Document the issue.
- Report what requires human review.

Never force-push shared branches.

Never rewrite protected history.

Never bypass repository protections.

Never merge an unverified change just to keep the workflow moving.

## 8.4 Suggested Milestones

### Milestone 1 — UI Foundation and Chassis

Branch:

`feature/vintage-ui-foundation`

Tasks:

- Establish the new vintage panel layout.
- Implement dark metal surfaces.
- Implement panel divisions.
- Implement typography.
- Implement spacing and alignment.
- Implement hardware details.
- Preserve existing controls and behavior while the new layout is established.
- Compare the first rendered screenshot against the reference.

Open a PR, review, and merge only when safe.

### Milestone 2 — Rotary Control Redesign

Branch:

`feature/vintage-ui-controls`

Tasks:

- Convert the specified controls to the new rotary style.
- Remove individual EQ/tone on/off switches.
- Make MIX a continuous rotary control.
- Implement the single Level Comp illuminated button.
- Resolve the custom bypass presentation according to the compatibility safeguard.

Open a PR, review, and merge only when safe.

### Milestone 3 — Digital Meters

Branch:

`feature/vintage-digital-meters`

Tasks:

- Implement red digital readouts on black meter faces.
- Preserve actual input measurements.
- Preserve actual output measurements.
- Preserve actual gain-reduction measurements.
- Verify readability.
- Verify meter behavior.

Open a PR, review, and merge only when safe.

### Milestone 4 — Analyzer Redesign

Branch:

`feature/vintage-analyzer`

Tasks:

- Implement the vintage analyzer appearance.
- Improve grid presentation.
- Improve scales.
- Improve labels.
- Improve spectrum rendering.
- Improve supported PRE/POST presentation.
- Verify that the analyzer continues to display real signal data.

Open a PR, review, and merge only when safe.

### Milestone 5 — Visual Polish and Regression Checks

Branch:

`feature/vintage-ui-polish`

Tasks:

- Refine proportions.
- Refine alignment.
- Refine typography.
- Refine textures.
- Refine colors.
- Refine spacing.
- Compare the actual UI against the reference.
- Test resizing.
- Test supported UI scaling.
- Check CPU/UI rendering behavior.
- Run the full relevant build and regression test suite.
- Verify plugin behavior in the supported host.

Open a PR, review, and merge only when safe.

### Milestone 6 — Release Preparation

Branch:

`release/<version>`

Tasks:

- Start a dedicated release branch from the fully merged, validated work.
- Update version metadata according to the project's existing release process.
- Update release metadata.
- Prepare the release screenshot.
- Prepare the release notes.
- Build release artifacts.
- Validate release artifacts.
- Verify the release notes and screenshot.
- Publish only after every release requirement is satisfied.

A milestone may be split into smaller branches if that makes the work safer.

Do not combine unrelated milestones merely to reduce the number of PRs.

---

# 9. Acceptance Checklist

The UI redesign is complete only when all applicable items below are satisfied.

## 9.1 Visual Acceptance

- [ ] The agent inspected the actual reference image in the repository.
- [ ] The rendered UI closely matches the reference's overall layout.
- [ ] The rendered UI closely matches the reference's proportions.
- [ ] Dark vintage rack chassis is implemented.
- [ ] Cream typography is implemented.
- [ ] Restrained red/orange accents are implemented.
- [ ] The analyzer has the intended vintage presentation.
- [ ] The analyzer remains readable.
- [ ] INPUT uses a black-faced red digital readout.
- [ ] OUTPUT uses a black-faced red digital readout.
- [ ] GR uses a black-faced red digital readout.
- [ ] All specified continuous controls are rotary knobs.
- [ ] LOW has no individual on/off switch.
- [ ] MID has no individual on/off switch.
- [ ] HIGH has no individual on/off switch.
- [ ] TIGHT has no individual on/off switch.
- [ ] SMOOTH has no individual on/off switch.
- [ ] AIR has no individual on/off switch.
- [ ] MIX is continuous and has no on/off switch.
- [ ] Level Comp is the only dedicated custom UI toggle.
- [ ] Level Comp has its label integrated into the button assembly.
- [ ] No second custom bypass toggle remains, unless the compatibility safeguard required the agent to stop and request a decision.
- [ ] The actual plugin screenshot has been compared against the reference.
- [ ] Visible differences have been refined.

## 9.2 Functional and Compatibility Acceptance

- [ ] Existing DSP/audio behavior is unchanged.
- [ ] Existing parameter IDs are preserved.
- [ ] Automation compatibility is preserved.
- [ ] Presets work.
- [ ] Session recall works.
- [ ] Host bypass and plugin functionality remain intact.
- [ ] Meter data is real and accurate.
- [ ] Analyzer data is real and accurate.
- [ ] No audio-thread real-time safety regression was introduced.
- [ ] Supported plugin formats build.
- [ ] Supported plugin formats load.
- [ ] Relevant tests pass.
- [ ] No unrelated changes are included.

## 9.3 Git and Review Acceptance

- [ ] Every implementation milestone was completed on its own branch.
- [ ] Each milestone was reviewed before merging.
- [ ] Only verified, non-destructive PRs were merged.
- [ ] The release branch was created from the fully merged and validated result.
- [ ] The final release diff contains no unrelated changes.
- [ ] No unreviewed changes are included in the release.

Do not mark the project complete while any required item remains unchecked.

Report blockers clearly instead of claiming completion.

---

# 10. Release Notes Requirements

Once all acceptance criteria are met, prepare and publish the release using the project's existing release process.

The release notes must contain the following sections in this order.

## 10.1 Actual UI Screenshot

Include a screenshot of the **real, running plugin UI** showing the completed redesign.

Requirements:

- Capture it from the built plugin in the supported host.
- Do not use the generated reference image as the release screenshot.
- Do not use a mockup.
- Do not use an edited concept image.
- Do not use a screenshot of an unfinished build.
- Store the screenshot in the repository's established screenshot location, or another sensible documented location.
- Embed the screenshot in the GitHub release notes so it is visible in the release description.
- Use a valid repository image path or uploaded release asset URL.
- Confirm that the image actually renders in the published release notes.

The release screenshot must show the final implemented UI.

## 10.2 Brief Quality-of-Life Explanation

Write a short paragraph explaining:

- What DRM-RESHAPE does for the user.
- What practical quality-of-life benefits it brings.
- How its drum tone-shaping workflow helps users work with their drum sound.

Keep this concise and grounded in the plugin's actual functionality.

Do not invent features.

Do not make unsupported performance claims.

Do not claim audio capabilities that have not been implemented.

The explanation should communicate the practical value of the plugin rather than simply describing its appearance.

## 10.3 Release Improvements

Below the quality-of-life paragraph, list the improvements included in this release.

Organize them into concise bullets or categories.

Examples:

- Vintage interface redesign.
- Rotary control presentation.
- Simplified always-active tone controls.
- Dedicated Level Comp button.
- Digital input/output/gain-reduction meters.
- Improved analyzer presentation.
- Readability improvements.
- Layout refinements.
- Verified bug fixes.
- Other verified improvements.

Only list work that is actually present in the release.

Do not include planned, incomplete, or unmerged work.

## 10.4 Release Notes Structure

Use the following structure, adapting the wording to the actual release:

```markdown
# DRM-RESHAPE — <Release Version>

![DRM-RESHAPE redesigned interface](<valid screenshot asset URL or repository image path>)

## Quality of Life

<Brief explanation of what DRM-RESHAPE helps users do and how the plugin improves their drum tone-shaping workflow.>

## Improvements

- <Verified improvement>
- <Verified improvement>
- <Verified improvement>

## Compatibility

<Accurate compatibility notes, if relevant.>
```

Do not publish:

- Placeholder text.
- Broken screenshot links.
- Unverified claims.
- Unmerged improvements.
- Features that do not exist.

The release screenshot is mandatory.

Do not publish the release until the screenshot has been uploaded and its visibility in the release notes has been verified.

---

# 11. Final Agent Report

When the work is complete, provide a concise report containing:

## Implementation

- Branches created.
- Milestones completed.
- PRs opened.
- PRs reviewed.
- PRs merged.
- Any branches or PRs intentionally left unmerged, including reasons.

## Validation

- Build results.
- Test results.
- Plugin loading results.
- Compatibility checks.
- Confirmation that the rendered UI was compared against the reference.
- Confirmation that visual refinements were completed.

## Release

- Final UI screenshot location.
- Release version.
- Published release URL.
- Release notes status.
- Any known limitations or follow-up work.

Do not report a task as completed if it has not actually been completed and verified.

---

# 12. Final Instructions

Prioritize:

1. Visual fidelity.
2. Non-destructive implementation.
3. Existing plugin compatibility.
4. Verified functionality.
5. Release quality.

The supplied reference image is the primary visual target.

Inspect it.

Reproduce it as closely as the framework allows.

Validate the result in the actual plugin.

Work incrementally.

**One milestone, one branch, one review at a time.**

Never sacrifice the existing working audio plugin merely to achieve a visual change.

Do not publish a release until all required acceptance criteria have been met.
