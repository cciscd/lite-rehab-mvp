# LiteRehab-Fusion dual-board MVP

This folder contains the complete MVP for a home upper-limb rehabilitation recorder.

## System

```text
MYOSA ESP32 wearable                       ESP32-S3 receiver
MPU6050 + OLED + local counting ── BLE ──> CRC check + LED/buzzer
                                                     │ USB serial
                                                     ▼
Laptop webcam + MediaPipe + Python dashboard + session CSV
```

Recognized states:

- `idle`
- `forearm_rotation`
- `elbow_flexion`

Feedback states:

- `ok`
- `too_fast`
- `insufficient_range`
- visual `trunk_compensation`

## Folder structure

| Folder | Contents |
|---|---|
| `shared` | CRC-protected BLE packet and motion state machine |
| `wearable` | MYOSA ESP32 WROOM-32E firmware (`esp32`) |
| `receiver` | ESP32-S3 N16R8 receiver firmware (`esp32s3`) |
| `python` | Dashboard, data collection, tests, and optional 1D-CNN |
| `tests` | Host-side C tests |
| `WIRING_GUIDE.md` | Wire-by-wire assembly instructions |
| `COMPONENTS.md` | Bilingual component list |

## 1. Test and build everything

```bash
cd /Users/yuedonghan/Desktop/BMEG3920_project/lite_rehab_mvp
./scripts/test_all.sh
```

## 2. Flash the MYOSA wearable

Connect the MYOSA USB-C cable, find its port with `ls /dev/cu.*`, then run:

```bash
cd /Users/yuedonghan/Desktop/BMEG3920_project/lite_rehab_mvp/wearable
. /Users/yuedonghan/.espressif/v6.0.2/esp-idf/export.sh
idf.py -p /dev/cu.usbserial-PORT flash monitor
```

Keep the forearm module still during the first two seconds after reset.

## 3. Flash the ESP32-S3 receiver

Connect the native ESP32-S3 USB port:

```bash
cd /Users/yuedonghan/Desktop/BMEG3920_project/lite_rehab_mvp/receiver
. /Users/yuedonghan/.espressif/v6.0.2/esp-idf/export.sh
idf.py -p /dev/cu.usbmodem-PORT flash monitor
```

Exit the monitor with `Ctrl-]` before opening the Python dashboard; only one program can use the serial port at a time.

## 4. Prepare Python

MediaPipe currently works most reliably with Python 3.12. A clean Conda environment is recommended:

```bash
conda create -n literehab python=3.12 -y
conda activate literehab
cd /Users/yuedonghan/Desktop/BMEG3920_project/lite_rehab_mvp/python
python -m pip install -r requirements.txt
```

## 5. Start the dashboard

```bash
python run_dashboard.py --port auto --camera 0 --output sessions/demo.csv
```

Controls:

- `q` or `Esc`: quit;
- `b`: capture a new neutral torso baseline;
- `r`: reset the visual elbow-range history.

If MediaPipe or the camera is unavailable, the application continues in IMU-only mode.

## 6. Collect training data

Collect separate 30-second recordings for every subject and class:

```bash
python collect_data.py --port auto --subject S01 --label idle --seconds 30
python collect_data.py --port auto --subject S01 --label forearm_rotation --seconds 30
python collect_data.py --port auto --subject S01 --label elbow_flexion --seconds 30
```

Repeat for at least three people. Keep one complete participant out of training.

## 7. Train the optional 1D-CNN

```bash
python train_1d_cnn.py --data data --holdout-subject S03 \
  --output models/imu_1dcnn.pt

python run_dashboard.py --port auto --model models/imu_1dcnn.pt
```

The dashboard uses the firmware's deterministic classifier unless a valid model is supplied. Do not report model performance until it has been measured on a held-out participant.

## Threshold calibration

Initial thresholds are defined by `motion_default_config()` in `shared/motion_logic.c`. If the axes are reversed by physical mounting, change the two gyro inputs passed to `motion_logic_update()` in `wearable/main/app_main.c`. Record raw data before changing thresholds.

## Safety and scope

This is an engineering prototype. It does not diagnose an injury, prescribe treatment, predict a clinical score, or replace a physiotherapist.
