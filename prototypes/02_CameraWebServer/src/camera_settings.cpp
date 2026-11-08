#include "camera_settings.h"

#include <Preferences.h>
#include <cstring>

namespace {
constexpr char kNamespace[] = "golfcam";
constexpr char kProfileKey[] = "profile";
constexpr uint32_t kVersion = 1;

struct Control {
  const char *name;
  int (*read)(const sensor_t *);
  int (*write)(sensor_t *, int);
  int minimum;
  int maximum;
};

// Keep this order stable, or increment kVersion. Manual values precede the
// automatic controls so restoring a profile leaves the requested mode active.
#define CONTROL(name, field, setter, low, high) \
  {name, [](const sensor_t *s) -> int { return s->status.field; }, \
   [](sensor_t *s, int v) -> int { return s->setter ? s->setter(s, v) : -1; }, low, high}

const Control kControls[] = {
  {"framesize", [](const sensor_t *s) -> int { return s->status.framesize; },
   [](sensor_t *s, int v) -> int { return s->set_framesize(s, static_cast<framesize_t>(v)); }, 0, FRAMESIZE_QVGA},
  CONTROL("quality", quality, set_quality, 4, 63),
  CONTROL("brightness", brightness, set_brightness, -3, 3),
  CONTROL("contrast", contrast, set_contrast, -3, 3),
  CONTROL("saturation", saturation, set_saturation, -4, 4),
  CONTROL("sharpness", sharpness, set_sharpness, -3, 3),
  CONTROL("denoise", denoise, set_denoise, 0, 8),
  CONTROL("special_effect", special_effect, set_special_effect, 0, 6),
  CONTROL("wb_mode", wb_mode, set_wb_mode, 0, 4),
  CONTROL("aec_value", aec_value, set_aec_value, 0, 65535),
  CONTROL("agc_gain", agc_gain, set_agc_gain, 0, 64),
  {"gainceiling", [](const sensor_t *s) -> int { return s->status.gainceiling; },
   [](sensor_t *s, int v) -> int { return s->set_gainceiling(s, static_cast<gainceiling_t>(v)); }, 0, 511},
  CONTROL("awb_gain", awb_gain, set_awb_gain, 0, 1),
  CONTROL("ae_level", ae_level, set_ae_level, -5, 5),
  CONTROL("aec2", aec2, set_aec2, 0, 1),
  CONTROL("awb", awb, set_whitebal, 0, 1),
  CONTROL("aec", aec, set_exposure_ctrl, 0, 1),
  CONTROL("agc", agc, set_gain_ctrl, 0, 1),
  CONTROL("bpc", bpc, set_bpc, 0, 1),
  CONTROL("wpc", wpc, set_wpc, 0, 1),
  CONTROL("raw_gma", raw_gma, set_raw_gma, 0, 1),
  CONTROL("lenc", lenc, set_lenc, 0, 1),
  CONTROL("hmirror", hmirror, set_hmirror, 0, 1),
  CONTROL("vflip", vflip, set_vflip, 0, 1),
  CONTROL("dcw", dcw, set_dcw, 0, 1),
  CONTROL("colorbar", colorbar, set_colorbar, 0, 1),
};
#undef CONTROL

constexpr size_t kControlCount = sizeof(kControls) / sizeof(kControls[0]);
struct Profile {
  uint32_t version;
  uint32_t sensor;
  int32_t values[kControlCount];
};

Profile current = {};
Profile defaults = {};
bool ready = false;
bool saved = false;

bool valid(const Profile &profile) {
  if (profile.version != kVersion || profile.sensor != defaults.sensor) return false;
  for (size_t i = 0; i < kControlCount; ++i) {
    if (profile.values[i] < kControls[i].minimum || profile.values[i] > kControls[i].maximum) return false;
  }
  return profile.values[0] <= defaults.values[0];
}

esp_err_t apply(const Profile &profile) {
  sensor_t *sensor = esp_camera_sensor_get();
  if (!ready || !sensor) return ESP_ERR_INVALID_STATE;
  if (!valid(profile)) return ESP_ERR_INVALID_ARG;
  const Profile previous = current;
  for (size_t i = 0; i < kControlCount; ++i) {
    if (profile.values[i] == current.values[i]) continue;
    if (kControls[i].write(sensor, profile.values[i]) < 0) {
      // Roll back the controls already applied instead of reporting success
      // with only part of the saved profile restored.
      for (size_t j = 0; j <= i; ++j) {
        if (profile.values[j] != previous.values[j]) {
          kControls[j].write(sensor, previous.values[j]);
        }
      }
      return ESP_FAIL;
    }
  }
  current = profile;
  return ESP_OK;
}
}  // namespace

esp_err_t cameraSettingsBegin() {
  sensor_t *sensor = esp_camera_sensor_get();
  if (!sensor) return ESP_ERR_INVALID_STATE;
  current.version = kVersion;
  current.sensor = sensor->id.PID;
  for (size_t i = 0; i < kControlCount; ++i) current.values[i] = kControls[i].read(sensor);
  defaults = current;
  ready = true;
  saved = false;

  Preferences storage;
  if (!storage.begin(kNamespace, true)) return ESP_ERR_NOT_FOUND;
  if (!storage.isKey(kProfileKey)) { storage.end(); return ESP_ERR_NOT_FOUND; }
  Profile profile = {};
  bool read = storage.getBytesLength(kProfileKey) == sizeof(profile)
    && storage.getBytes(kProfileKey, &profile, sizeof(profile)) == sizeof(profile);
  storage.end();
  if (!read || !valid(profile)) return ESP_ERR_INVALID_ARG;
  esp_err_t result = apply(profile);
  saved = result == ESP_OK;
  return result;
}

esp_err_t cameraSettingsSet(const char *name, int value) {
  if (!ready) return ESP_ERR_INVALID_STATE;
  for (size_t i = 0; i < kControlCount; ++i) {
    if (strcmp(name, kControls[i].name)) continue;
    if (value < kControls[i].minimum || value > kControls[i].maximum
        || (i == 0 && value > defaults.values[0])) return ESP_ERR_INVALID_ARG;
    sensor_t *sensor = esp_camera_sensor_get();
    if (!sensor) return ESP_ERR_INVALID_STATE;
    if (kControls[i].write(sensor, value) < 0) return ESP_FAIL;
    current.values[i] = value;
    return ESP_OK;
  }
  return ESP_ERR_NOT_FOUND;
}

int cameraSettingsValue(const char *name, int fallback) {
  if (!ready) return fallback;
  for (size_t i = 0; i < kControlCount; ++i) {
    if (!strcmp(name, kControls[i].name)) return current.values[i];
  }
  return fallback;
}

esp_err_t cameraSettingsSave() {
  if (!ready || !valid(current)) return ESP_ERR_INVALID_STATE;
  Preferences storage;
  if (!storage.begin(kNamespace, false)) return ESP_FAIL;
  // One versioned blob is committed only on an explicit Save, not each slider
  // movement. Preferences::putBytes commits NVS before returning success.
  bool written = storage.putBytes(kProfileKey, &current, sizeof(current)) == sizeof(current);
  storage.end();
  if (written) saved = true;
  return written ? ESP_OK : ESP_FAIL;
}

esp_err_t cameraSettingsRestoreDefaults() {
  if (!ready) return ESP_ERR_INVALID_STATE;
  const Profile previous = current;
  esp_err_t result = apply(defaults);
  if (result != ESP_OK) return result;
  Preferences storage;
  bool cleared = storage.begin(kNamespace, false);
  if (cleared) {
    cleared = !storage.isKey(kProfileKey) || storage.remove(kProfileKey);
    storage.end();
  }
  if (!cleared) { apply(previous); return ESP_FAIL; }
  saved = false;
  return ESP_OK;
}

bool cameraSettingsHaveSavedProfile() { return saved; }
int cameraSettingsMaxFramesize() { return ready ? defaults.values[0] : FRAMESIZE_QVGA; }
