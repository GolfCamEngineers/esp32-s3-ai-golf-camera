# Indoor resolution comparison — November 8, 2026

Hardware: Waveshare ESP32-S3-CAM-OV5640, 8 MB PSRAM. Firmware uses two
PSRAM JPEG buffers allocated for QSXGA, then applies the requested resolution.
The active `/settings` buffer limit was 23 (QSXGA), and the existing saved
profile restored successfully after uploading and reconnecting power.

The same indoor scene was used at JPEG quality 12 and XCLK 20 MHz, with
automatic exposure/gain/white balance, exposure level -1 and night mode off.
Other live streams were closed. These are received MJPEG rates measured by
`tools/benchmark_camera.py`, not the server's static frame-rate header.
Every frame was fully decoded and checked for the expected dimensions.
Five warm-up frames were discarded before each ten-second measurement.

| Resolution | Received FPS | Decoded measurement frames | Mean JPEG bytes | 95th-percentile frame interval |
| --- | ---: | ---: | ---: | ---: |
| QVGA 320 × 240 | 18.44 | 186 | 7,627 | 93 ms |
| VGA 640 × 480 | 14.68 | 148 | 21,214 | 113 ms |
| SVGA 800 × 600 | 12.09 | 122 | 29,292 | 126 ms |
| XGA 1024 × 768 | 6.07 | 62 | 43,415 | 369 ms |
| HD 1280 × 720 | 7.08 | 73 | 47,576 | 241 ms |
| UXGA 1600 × 1200 | 3.00 | 32 | 113,854 | 427 ms |
| FHD 1920 × 1080 | 3.60 | 37 | 103,850 | 331 ms |
| QSXGA 2560 × 1920 | 1.26 | 14 | 322,038 | 862 ms |

There were no decode or dimension errors across the 774 measured frames
(plus 40 warm-up frames). Private sample photographs and raw results were
kept outside the repository.

Twenty-second repeat measurements returned 16.24 FPS at VGA (326 decoded
frames) and 12.69 FPS at SVGA (255 decoded frames), with no errors. SVGA at
quality 12 was selected and saved as the detail/streaming-speed compromise:
6.25 times the pixel count of QVGA while retaining about 12–13 FPS in both
measurements. A subsequent still capture decoded at 800 × 600. Requesting
frame-size value 24, above the allocated limit, returned HTTP 400 without
changing the active setting.

After a physical power disconnect and normal reconnect, all 26 saved image
controls matched the pre-restart values, including SVGA, quality 12 and
exposure level -1. A still JPEG fully decoded at 800 × 600, and `/settings`
reported the saved profile and PSRAM limit 23. One reconnect initially entered
the USB download mode; reconnecting with both buttons released restored normal
startup without another firmware upload or settings change.

This verifies streaming behavior for this scene and network only. It does
not measure outdoor ball visibility, motion blur, detection accuracy, video
latency or long-term stability. Different lighting, scene detail, Wi-Fi and
other clients can change throughput. The internal-memory fallback was
reviewed in code but was not forced on this PSRAM-equipped board.
