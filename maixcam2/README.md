# LiteRehab MaixCAM 2 Camera

`main.py` turns MaixCAM 2 into the LiteRehab vision source. It uses the built-in camera only; it does not connect to either ESP32 GPIO.

## Recommended: USB UVC

1. On MaixCAM 2, open **Settings → USB Settings** and enable **UVC**.
2. Connect MaixCAM 2 to the laptop with a Type-C **data** cable.
3. Open `main.py` in MaixVision and run it with `MODE = "uvc"`.
4. On the laptop, from the LiteRehab repository, run:

   ```bash
   PYTHONPATH=python python scripts/probe_cameras.py
   ./scripts/start_maixcam2_demo.sh <maixcam-index>
   ```

Use the comparison method printed by `probe_cameras.py` to identify the MaixCAM 2 index. The dashboard must show `Camera: connected` and `Mode: Fusion` once the right shoulder, elbow, wrist, and hip are visible.

## Backup: RTSP

1. Connect MaixCAM 2 and the laptop to the same Wi-Fi network, or use the MaixCAM 2 USB virtual network adapter.
2. Change `MODE` in `main.py` to `"rtsp"` and run it in MaixVision.
3. Copy the URL printed in the MaixVision terminal, normally `rtsp://<device-ip>:8554/live`.
4. On the laptop, run:

   ```bash
   ./scripts/start_maixcam2_demo.sh rtsp://<device-ip>:8554/live
   ```

## Placement

- Place MaixCAM 2 horizontally, 1.5–2.0 m from the participant, at chest height.
- Keep the whole upper body and hips in frame. For right-arm rehabilitation, keep the right shoulder, elbow, wrist, and right hip unobstructed.
- Use `b` in the focused dashboard window while standing in a neutral posture to refresh the trunk baseline.

## Official references

- [MaixCAM 2 quick start](https://wiki.sipeed.com/maixpy/doc/en/README_MaixCAM2.html)
- [MaixPy UVC streaming](https://wiki.sipeed.com/maixpy/doc/en/video/uvc_streaming.html)
- [MaixPy RTSP streaming](https://wiki.sipeed.com/maixpy/doc/zh/video/rtsp_streaming.html)
- [MaixPy camera configuration](https://wiki.sipeed.com/maixpy/doc/en/vision/camera.html)

