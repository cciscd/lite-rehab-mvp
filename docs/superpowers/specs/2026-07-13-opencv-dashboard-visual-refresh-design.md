# OpenCV Dashboard Visual Refresh Design

## Goal

Refresh the existing OpenCV desktop dashboard so that device health, exercise
state, repetition progress, and safety feedback are understandable at a glance.
The change preserves the current camera, telemetry, inference, synchronization,
CSV logging, and keyboard-control behavior.

## Visual Direction

Use a restrained dark monitoring interface with high-contrast content and a
small semantic color system:

- green: healthy, connected, or correct movement;
- blue: active movement and neutral live measurements;
- amber: recoverable coaching warnings such as excessive speed or limited ROM;
- red: trunk compensation, disconnected devices, or unavailable inputs;
- gray: idle, disabled, unavailable, or warming-up states.

The interface remains English because OpenCV's built-in text renderer does not
reliably support Chinese glyphs without adding a new font-rendering dependency.
Labels will use short, presentation-friendly wording rather than raw internal
identifiers.

## Layout

The dashboard uses one fixed OpenCV canvas divided into three regions:

1. A compact header spans the full width. It shows the LiteRehab session title
   and health chips for serial, camera, and fusion mode.
2. The camera occupies the main left region. Pose landmarks remain visible, but
   the existing nine-line diagnostic overlay is removed. Only the current
   exercise and the highest-priority coaching message appear over the video.
3. A right information rail contains the repetition count, current exercise,
   ROM, confidence or model warm-up state, and the IMU gyroscope chart.

The result should preserve the current information while separating system
health, patient-facing guidance, and technical measurements.

## Components and Boundaries

Drawing will be split into small functions with explicit inputs:

- theme and text helpers centralize colors, typography scale, spacing, and
  conversion of internal identifiers to display labels;
- status-chip drawing renders a label, state, and semantic color;
- metric-card drawing renders a title, primary value, and optional secondary
  text;
- feedback-banner drawing selects the most important message and severity;
- chart drawing renders grid lines, zero line, axis curves, scale, and legend;
- dashboard composition arranges the header, video, information rail, and
  footer shortcut hint without changing sensor or model state.

These helpers remain in `python/run_dashboard.py` unless implementation makes a
small dedicated rendering module clearly beneficial. The data-processing loop
continues to compute the same values and passes them into the drawing layer.

## Status and Feedback Rules

Device chips map existing runtime strings to a small number of display states:

- connected and healthy are green;
- connecting, reconnecting, warming up, and IMU-only operation are amber;
- unavailable, failed, or disconnected inputs are red;
- disabled optional models are gray.

Feedback priority is safety first: trunk compensation overrides range and speed
coaching; other explicit warnings override normal movement guidance. The primary
banner uses short messages such as `AVOID TRUNK COMPENSATION`, `SLOW DOWN`,
`INCREASE RANGE`, and `GOOD FORM`. Raw feedback text remains available to the
mapping layer as a fallback so an unknown future state is still shown.

## Interaction and Failure Behavior

Existing controls remain unchanged:

- `B` recaptures the posture baseline;
- `R` resets the current repetition ROM tracker;
- `Q` or `Esc` exits.

A compact footer displays these shortcuts. If the camera is unavailable, its
region shows a clear IMU-only empty state instead of an unstyled black frame.
Missing pose, ROM, confidence, or prediction values render as `--` or a concise
warm-up label and never break dashboard composition.

## Verification

Rendering helpers will be testable without opening a GUI window. Tests will
cover semantic status mapping, display-label formatting, feedback priority, and
successful composition into an image with the expected dimensions. Existing
dashboard state and CLI tests must continue to pass. The final verification will
run the focused Python tests and the existing headless dashboard smoke test.

## Scope Limits

This refresh does not create a browser frontend, change clinical algorithms,
alter model decisions, add persistence, or redesign firmware feedback. It does
not introduce custom image assets or a new UI framework.
