# LiteRehab Fusion One-Page Pitch LaTeX Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Produce an editable, human-sounding, evidence-bounded one-page English pitch in LaTeX and a verified A4 PDF for LiteRehab Fusion.

**Architecture:** Keep the final copy and claim audit in a focused Markdown source, then typeset the approved copy in one self-contained LaTeX file. Compile with `latexmk`, render the PDF to PNG, and verify page count, embedded fonts, text extraction, and visual layout before delivery.

**Tech Stack:** LaTeX with pdfLaTeX, Latin Modern, `geometry`, `microtype`, `xcolor`, `tabularx`, `enumitem`, and TikZ; Poppler `pdfinfo`, `pdffonts`, `pdftotext`, and `pdftoppm` for verification.

## Global Constraints

- The final document is one A4 portrait page.
- Body text is Latin Modern and remains at least 9 pt.
- The layout is an academic product brief, not a poster or conference-paper template.
- Use restrained dark blue and teal accents; do not use decorative fonts, gradients, shadows, stock icons, or dense boxed layouts.
- State that LiteRehab Fusion is an engineering prototype, not a medical device.
- Make no claim of clinical effectiveness, diagnostic accuracy, improved patient outcomes, market size, price, partnerships, clinical trials, or regulatory approval.
- Keep existing user source-code changes and unrelated untracked files out of all commits.

---

### Task 1: Draft, humanize, and audit the pitch copy

**Files:**
- Create: `docs/pitch/literehab_one_page_pitch_copy.md`

**Interfaces:**
- Consumes: verified capabilities and constraints in `README.md`, `DEMO_GUIDE.md`, and `docs/superpowers/specs/2026-07-14-one-page-pitch-design.md`.
- Produces: final English copy organized under `The gap`, `Our response`, `How it works`, `What the MVP demonstrates`, `Value`, `Route to use`, `Limits`, and `What we need next`, plus a demonstrated/proposed/limitation claim audit.

- [ ] **Step 1: Write the complete copy**

Create concise copy with this narrative and evidence:

```markdown
# LiteRehab Fusion

*Real-time guidance for more consistent upper-limb rehabilitation practice at home.*

## The gap

A patient may leave a physiotherapy appointment knowing which exercises to practise, but still be unsure whether each repetition at home is controlled, large enough, or compensated by trunk movement. Feedback often arrives at the next appointment, after the practice has already happened.

## Our response

LiteRehab Fusion is a wearable and camera-based prototype that gives immediate, simple feedback during prescribed upper-limb exercises. It detects the demonstrated movement, counts repetitions, flags common practice errors, and records the session for later review.

## How it works

An MPU6050 on the forearm samples motion at 50 Hz. An ESP32 wearable sends the data over BLE to an ESP32-S3 receiver. A MaixCAM2 video stream supplies posture information to the computer dashboard. The dashboard combines both inputs and returns prompts such as "Move more slowly", "Increase movement range", or "Avoid trunk compensation".

## What the MVP demonstrates

- Elbow flexion and forearm rotation recognition
- Immediate feedback for speed, range, and trunk compensation
- OLED repetition count, LED and buzzer feedback, and synchronized session logging
- A classroom CNN-BiGRU baseline with rule-based fallback
- 70 Python tests, three C host tests, a model-loading smoke test, and successful builds for both ESP32 targets

## Value

For patients, LiteRehab makes practice feedback immediate and easy to understand. For physiotherapists, recorded sessions could provide a clearer view of practice between appointments without handing clinical decisions to the device.

## Route to use

The proposed first step is a supervised pilot with physiotherapists. A future version could be supplied through rehabilitation providers as part of a prescribed home-exercise programme, with the physiotherapist choosing the exercises and reviewing the records.

## Limits

LiteRehab Fusion is an engineering prototype, not a medical device. It does not diagnose, prescribe treatment, score recovery, or replace professional supervision. The current model uses a small public dataset, supports a limited exercise set, and makes no clinical accuracy claim.

## What we need next

We are looking for physiotherapist partners to test usability, refine the feedback, and collect representative movement data under professional supervision.

## Claim audit

| Claim | Status | Basis |
|---|---|---|
| 50 Hz IMU sensing, BLE transfer, camera posture input, feedback, and logging | Demonstrated | Current repository and demo workflow |
| Two demonstrated exercises and three feedback categories | Demonstrated | Current firmware, dashboard, and demo guide |
| Automated test and build totals | Demonstrated | `scripts/test_all.sh` verification |
| Later physiotherapist review and clinic-supplied use | Proposed | Product direction, not a deployed workflow |
| Clinical effectiveness, diagnostic accuracy, and improved outcomes | Not claimed | Requires representative and clinical validation |
```

- [ ] **Step 2: Apply the Humanizer audit**

Read the copy aloud and remove promotional language, inflated impact claims, vague attribution, repetitive three-part phrasing, excessive em dashes, filler, and long academic sentences. Preserve the technical terms only where they explain the working system.

Expected result: the copy sounds like a student team speaking to a mixed academic and industry panel, while every claim remains supported or explicitly labelled as proposed.

- [ ] **Step 3: Check claim boundaries**

Run:

```bash
rg -n -i "clinically proven|improves recovery|prevents injury|diagnoses|approved|market size|revenue|partnership" docs/pitch/literehab_one_page_pitch_copy.md
```

Expected: no unsupported positive claims; `diagnoses` may appear only inside the explicit limitation statement.

- [ ] **Step 4: Commit the copy and audit**

```bash
git add docs/pitch/literehab_one_page_pitch_copy.md
git commit -m "docs: draft one-page pitch copy"
```

### Task 2: Typeset the A4 LaTeX pitch

**Files:**
- Create: `output/pdf/literehab_one_page_pitch.tex`
- Create through compilation: `output/pdf/literehab_one_page_pitch.pdf`

**Interfaces:**
- Consumes: the humanized pitch copy and claim audit from Task 1.
- Produces: a self-contained A4 LaTeX source and its compiled one-page PDF.

- [ ] **Step 1: Create the LaTeX document**

Use `\documentclass[10pt,a4paper]{article}`, `\usepackage[T1]{fontenc}`, `\usepackage{lmodern}`, and `\usepackage[protrusion=true,expansion=true]{microtype}`. Set `geometry` to 13 mm left/right and 12 mm top/bottom. Define dark blue `PitchNavy`, teal `PitchTeal`, and muted grey `PitchGrey`.

Build the page in this order:

1. A compact title band with product name, evidence-safe tagline, and `BMEG3920 Engineering Prototype` label.
2. A two-column row containing `The gap` and `Our response`.
3. A full-width TikZ system flow: `Wearable IMU -> BLE receiver -> Dashboard <- MaixCAM2`, followed by feedback and session record outputs.
4. A two-column row containing `What the MVP demonstrates` and `Who benefits`.
5. A bottom row containing `Route to use`, `Limits`, and a teal call-to-action strip.

Use 9.2 pt body text with approximately 11 pt leading, 12-13 pt section headings, and a 24-27 pt title. Keep bullets compact and do not reduce body text below 9 pt to force the page to fit.

- [ ] **Step 2: Compile with strict warnings**

Run:

```bash
mkdir -p output/pdf
latexmk -pdf -interaction=nonstopmode -halt-on-error -file-line-error -cd output/pdf/literehab_one_page_pitch.tex
```

Expected: exit code 0 and `output/pdf/literehab_one_page_pitch.pdf` exists.

- [ ] **Step 3: Check the LaTeX log**

Run:

```bash
rg -n "Overfull|Underfull|LaTeX Warning|undefined references|Font Warning" output/pdf/literehab_one_page_pitch.log
```

Expected: no overfull boxes, unresolved references, or font warnings. Fix meaningful underfull boxes instead of hiding them globally.

- [ ] **Step 4: Commit the source and PDF**

```bash
git add output/pdf/literehab_one_page_pitch.tex output/pdf/literehab_one_page_pitch.pdf
git commit -m "docs: typeset LiteRehab one-page pitch"
```

### Task 3: Verify the final PDF visually and mechanically

**Files:**
- Verify: `output/pdf/literehab_one_page_pitch.pdf`
- Create temporarily: `tmp/pdfs/literehab_pitch-1.png`

**Interfaces:**
- Consumes: the compiled PDF from Task 2.
- Produces: verification evidence that the PDF is one readable A4 page with embedded Latin Modern fonts and intact content.

- [ ] **Step 1: Verify page geometry and count**

Run:

```bash
pdfinfo output/pdf/literehab_one_page_pitch.pdf | rg "Pages|Page size"
```

Expected:

```text
Pages:           1
Page size:       595.276 x 841.89 pts (A4)
```

- [ ] **Step 2: Verify embedded fonts**

Run:

```bash
pdffonts output/pdf/literehab_one_page_pitch.pdf
```

Expected: Latin Modern fonts are listed and every font has `emb` set to `yes`.

- [ ] **Step 3: Verify extractable text and required claims**

Run:

```bash
pdftotext output/pdf/literehab_one_page_pitch.pdf - | rg "LiteRehab Fusion|50 Hz|70 Python tests|not a medical device|physiotherapist partners"
```

Expected: all five required strings are present.

- [ ] **Step 4: Render and inspect the page**

Run:

```bash
mkdir -p tmp/pdfs
pdftoppm -png -r 180 -f 1 -singlefile output/pdf/literehab_one_page_pitch.pdf tmp/pdfs/literehab_pitch
```

Inspect `tmp/pdfs/literehab_pitch.png` at full-page scale. Confirm there is no clipping, collision, unreadably small text, excessive empty space, broken TikZ output, or accidental second-page content.

- [ ] **Step 5: Remove temporary render and run the project regression check**

Run:

```bash
rm -rf tmp/pdfs
./scripts/test_all.sh
```

Expected: 70 Python tests, three C host tests, the dashboard smoke test, and both ESP-IDF builds pass.

- [ ] **Step 6: Record final verification**

If visual inspection requires LaTeX changes, repeat Task 2 Steps 2-4 and Task 3 Steps 1-4 before delivery. Commit only the revised pitch source and PDF:

```bash
git add output/pdf/literehab_one_page_pitch.tex output/pdf/literehab_one_page_pitch.pdf
git commit -m "docs: refine one-page pitch layout"
```
