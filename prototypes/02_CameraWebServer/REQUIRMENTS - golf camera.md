# ESP32-S3 AI Golf Camera: Coding Requirements

<img src="Target and Balls.png" alt="Golf Target and Balls" style="width:400px;"/>

## 1. Project Overview

* **Objective:** Build a portable, Topgolf-style chipping game powered by an ESP32-S3 AI camera module.
* **Functionality:** The system monitors a field target, detects incoming golf shots, scores them based on proximity, and displays the running total.
* **Target Architecture:** Single-player mode with a single target for the initial release.

---

## 2. System Modes

### A. Setup Mode

1. **Physical Placement:** Mount the ESP32-S3 camera on a tripod facing the chipping field.
2. **UI Initialization:** Select **Setup** mode on the touch display to start the live camera stream.
3. **Target Calibration:**
* Aim the camera toward the target on the field.
* Run initial computer vision/AI routines to detect and highlight the target in the frame.



### B. Play Mode

1. **Game Initialization:**
* Select **Play** mode from the UI.
* Reset the total session score to `0`.
* Capture a reference still image to lock in target coordinates and background tracking.


2. **Core Game Loop:**
```text
Loop:
  ├── Capture current still image / frame
  ├── Detect if a new golf ball has landed
  ├── Calculate score for the new ball based on landing position
  ├── Add ball score to running total
  ├── Update touch display UI
  └── Sleep for [TBD] ms to conserve resources

```



---

## 3. Scoring System (Provisional Rules)

* **Center Bullseye:** 5 points
* **Outer Ring:** 3 points
* **Close Outside:** 1 point
* **Far Outside / Miss:** 0 points

---

## 4. Future Roadmap

* Support for multiple players
* Support for multiple targets across the field
