# ESP32-S3 AI Golf Camera

## Project Overview

This project uses an ESP32-S3 AI camera module to create a portable Topgolf-style chipping game. The system monitors a golf target and determines the result of each shot.

## Requirements

* Detect the golf ball
* Detect the golf target
* Determine whether the golf ball lands inside or outside the target
* Track and record each golf shot

## Hardware

* ESP32-S3 AI camera module
* Golf target
* Golf ball dispenser
* Golf mat
* Pitching wedge
* Golf balls

## Team Roles

* Camera and ESP32 firmware
* Golf-ball detection
* Target detection and scoring logic
* Testing, shot tracking, and documentation

## Repository Workflow

1. Create an issue before starting a task.
2. Create a separate branch for each feature or fix.
3. Do not push unfinished work directly to `main`.
4. Open a pull request when work is ready.
5. Have at least one teammate review the pull request before merging.

## Current Status

* [ ] Set up ESP32-S3 camera stream
* [ ] Detect golf ball
* [ ] Detect golf target
* [ ] Determine inside/outside result
* [ ] Record shot history
* [ ] Test system outdoors

## Resources
* [ESPConnect](https://thelastoutpostworkshop.github.io/ESPConnect)  is a browser-based toolkit for ESP32 devices. It brings common maintenance tasks together so you can connect, inspect and update your board without installing desktop utilities.
  * Video overview: [ESPConnect: The Browser Tool Every ESP32 Developer Needs to Know About
](https://www.youtube.com/watch?v=YUSDPqDLyX8&t=832s)
