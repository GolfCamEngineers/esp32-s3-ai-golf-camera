# SVGA frame-rate tuning — November 8, 2026

The camera remained at **800 × 600** throughout this comparison. The installed
PSRAM-buffer firmware from #21 and the same indoor scene were used. XCLK stayed
at 20 MHz, automatic exposure/gain/white balance stayed enabled, exposure level
was -1, and night mode stayed off. Only JPEG quality was varied. Larger values
on this camera's Quality control mean more compression and less fine detail.

`tools/benchmark_camera.py` fully decoded each received MJPEG frame and checked
its dimensions. Each measurement discarded five warm-up frames, then ran for
fifteen seconds. Other stream clients were closed. Temporary controls were
restored after each run; the benchmark does not change the saved profile.

| JPEG quality setting | Received FPS | Decoded measurement frames | Mean JPEG bytes | 95th-percentile frame interval |
| --- | ---: | ---: | ---: | ---: |
| 12 | 12.45 | 188 | 29,102 | 122 ms |
| 16 | 13.86 | 209 | 24,990 | 114 ms |
| 20 | 15.60 | 235 | 22,130 | 94 ms |
| 24 | 16.99 | 256 | 20,313 | 96 ms |
| 28 | 17.97 | 271 | 18,851 | 92 ms |
| 32 | 17.40 | 263 | 17,872 | 93 ms |

All 1,422 measured frames decoded correctly at 800 × 600. Samples were
visually inspected at qualities 12, 24 and 32; private images and raw results
were kept outside the repository.

Quality 24 was selected for longer validation: approximately 36% faster than
the initial quality-12 baseline, with roughly 30% smaller JPEGs. Stronger
compression at 28 or 32 added at most about one frame per second in this sweep.
This gain comes from compression, with a real fine-detail tradeoff; it is not
an increase in pixel resolution or a guarantee of outdoor ball visibility.

A 45-second quality-24 validation returned **15.75 FPS**, with 710 fully decoded
frames, a mean JPEG size of 20,040 bytes, and a 101 ms 95th-percentile interval.
A subsequent 20-second quality-12 control returned **12.21 FPS**, with 246 fully
decoded frames and a 133 ms 95th-percentile interval. Neither run had decode or
dimension errors. The longer comparison therefore supports approximately a
**29% improvement**, rather than assuming the best short-run rate persists.

Quality 24 was applied and saved through the web page. Comparing all 26 image
controls plus XCLK against the pre-test snapshot confirmed that only quality
changed (12 to 24). The page reported successful storage, `/settings` reported
a saved profile, and a subsequent still JPEG decoded at 800 × 600. Persistence
was already power-cycle tested in #21; this settings-only adjustment did not
include another power-cycle test.

These results measure delivered MJPEG throughput for this indoor scene and
network. They do not measure end-to-end latency, motion blur or ball-detection
accuracy. Lighting, scene detail, other clients and Wi-Fi conditions can change
the results. This tuning uses the existing saved Quality setting and does not
require a firmware source change or a clock adjustment.
