#pragma once

#include "esp_camera.h"

// Call once after camera initialization, before starting the HTTP server.
esp_err_t cameraSettingsBegin(framesize_t bufferLimit = FRAMESIZE_QVGA);
esp_err_t cameraSettingsSave();
esp_err_t cameraSettingsRestoreDefaults();
bool cameraSettingsHaveSavedProfile();
int cameraSettingsMaxFramesize();
int cameraSettingsValue(const char *name, int fallback);
esp_err_t cameraSettingsSet(const char *name, int value);
