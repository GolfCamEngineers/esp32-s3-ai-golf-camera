
# Example Code

This example uses the Waveshare ESP32-S3-CAM-OVxxxx camera sample code from their GitHub repository:

- [Waveshare CameraWebServer Example](https://github.com/waveshareteam/ESP32-S3-CAM-OVxxxx/tree/main/examples/Arduino-v3.2.0/examples/02_CameraWebServer)

## Save camera settings

The web page has **Save Settings** and **Restore Defaults** buttons above the
image controls. Adjust the picture, click **Save Settings**, and wait for the
confirmation before disconnecting power. The saved image controls are loaded
from the camera's flash memory on the next startup, regardless of which browser
or computer is used. Slider changes remain temporary until explicitly saved.

**Restore Defaults** applies the camera's startup image controls and clears the
saved profile. Wi-Fi credentials and the firmware are not changed. The page
reloads afterward; click **Start Stream** again to resume viewing.

Saved controls include resolution, JPEG quality, brightness, contrast,
saturation, sharpness, denoise, exposure, gain, white balance, image correction,
effects, mirror and flip. Advanced clock, PLL, window and direct register
settings are temporary and are not part of the saved profile.

This build allocates its camera buffer for **QVGA (320 x 240)**. Larger
resolutions are disabled in the menu and rejected by `/control`; enabling them
requires a separate buffer configuration change and capture tests. The setting
store validates its version, sensor and value ranges before applying a profile.
Writes happen only when Save Settings or Restore Defaults is clicked.

### Developing and checking the settings controls

Edit `web/camera_settings.js`, then run `python tools/build_camera_ui.py` to
regenerate the embedded script and vendor page headers. Generation is
repeatable and preserves the vendor pages apart from the added script tag.

Run `node --test test/settings_ui.test.cjs` and `pio run` before upload.
On hardware, verify Save confirmation, reject a resolution above QVGA, restart
the camera and compare `/status`, check a still capture, then verify Restore
Defaults clears `/settings`'s `saved` flag. Save the desired profile again after
testing. The settings API is `GET /settings`, `POST /settings/save`, and
`POST /settings/defaults`.

# Build Environment

## Visual Studio Code (VS Code)

Install [Microsoft Visual Studio Code](https://code.visualstudio.com/).

## PlatformIO Extension

Install the [PlatformIO extension](https://docs.platformio.org/en/latest/integration/ide/vscode.html) for VS Code.

PlatformIO allows you to build, upload, and monitor Arduino/ESP32 projects directly from a familiar IDE, such as Visual Studio Code.

The PlatformIO toolbar appears in the VS Code bottom toolbar and provides quick access to commonly used commands. Hover your mouse over any button to see its description.

![PlatformIO Toolbar](README_Images/image-1.png)

### Build

1. Click the **Build** button on the PlatformIO toolbar to compile the project.
2. Wait for the build to complete. You should see a message similar to:

   `==== [SUCCESS] Took ??.?? seconds ====`

### Upload

1. Make sure the camera is disconnected from your laptop.
2. Press and hold the camera's **BOOT** button.
3. While holding the button, connect the camera to your laptop using a USB cable.
4. Once connected and you hear the connection tone, release the **BOOT** button.
5. Click the **Upload** button on the PlatformIO toolbar and wait for the process to finish.
6. You should see a success message similar to:

   `==== [SUCCESS] Took ??.?? seconds ====`

7. Disconnect the camera.

### Run and Monitor the Camera

1. Start the PlatformIO Serial Monitor by clicking the **plug icon pointing upward** on the toolbar.

   ![PlatformIO Serial Monitor](README_Images/image.png)

2. Connect the camera to your laptop.
3. Watch the Serial Monitor for a message similar to:

   `Camera Ready! Use 'http://192.168.1.82' to connect`

   - Your IP address will likely be different.
   - If you miss the initial **Camera Ready!** message, the sample code periodically prints the IP address. Look for a message similar to:

     `192.168.1.82 Looping...6`

4. Open a web browser and navigate to the displayed IP address to access the camera.

   You can use your preferred browser or the VS Code integrated browser described below.

## VS Code Integrated Browser

VS Code includes an integrated browser that lets you open web pages in a tab within the IDE.

To open it:

1. Open the Command Palette:
   - **Windows/Linux:** `Ctrl+Shift+P`
   - **Mac:** `Cmd+Shift+P`
2. Type `Browser: Open Integrated Browser`.
3. Select the command and enter the camera's IP address in the browser's address bar.
