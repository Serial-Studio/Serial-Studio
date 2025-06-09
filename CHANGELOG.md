<a id="v3.1.1"></a>
# [Release 3.1.1 (v3.1.1)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.1.1) - 2025-06-09

Released just hours after v3.1.0, this update adds support for **binary protocol parsing**, no more converting raw bytes to strings before processing.

**Binary (Direct) Decoder:**
You can now select **Binary (Direct)** as a decoder method in the Project Editor.
This passes `QByteArray` data directly to the JavaScript parser, allowing high-performance parsing of binary protocols, including custom formats and raw sensor payloads.

**Technical Benefits:**
- Avoids unnecessary UTF-8, hex, or Base64 conversions.
- Enables use of raw binary frame data in JS scripts.
- Perfect for low-level byte-structured protocols.

**Example use in JS:**

```js
function parse(frame) {
    // Convert each byte to 0–5V scale
    let volts = [];
    for (let i = 0; i < frame.length; ++i)
        volts.push(frame[i] * 5.0 / 255);
    return volts;
}
```

Existing decoder modes (**Hexadecimal**, **Base64**, and **Plain Text**) continue to convert frame bytes into strings before passing them to JS. This behavior is preserved intentionally to ensure compatibility with all previous projects and scripts.

Only the new **Binary (Direct)** mode bypasses this conversion step and gives full byte-level access.

👉🏻 **Get Serial Studio Pro** https://store.serial-studio.com/buy/ba46c099-0d51-4d98-9154-6be5c35bc1ec.

Full Changelog: [v3.1.0 → v3.1.1](https://github.com/Serial-Studio/Serial-Studio/compare/v3.1.0...v3.1.1)

### Checksums

| File | SHA256 Checksum |
|------|-----------------|
| `Serial-Studio-3.1.1-Linux-arm64.AppImage` | `722be766340b1588b13f1d354232b11a2c19aa9cf9eed26dbf01c5e2b0fc8d60` |
| `Serial-Studio-3.1.1-Linux-x86_64.AppImage` | `a6017ab1bfcf50df6f59f343d040ad249747d1d4b8763d002c54a26fa09ccf99` |
| `Serial-Studio-3.1.1-Linux-x86_64.deb` | `c572e67d9c2dc11445f0a161a272ffc93ee2b79844da97bbeb9db110b7a0ac97` |
| `Serial-Studio-3.1.1-Windows-x86_64.msi` | `3ed32a4eeb8a65bcd3e9d60e340412acd201031bd5c1d2fa4728071f2798da62` |
| `Serial-Studio-3.1.1-macOS-Universal.dmg` | `143c3f3708e33df5b238b442a78692464db0c3a5d7c74d753cebf443742b9354` |

[Changes][v3.1.1]


<a id="v3.1.0"></a>
# [Release 3.1.0 (v3.1.0)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.1.0) - 2025-06-08

This release brings some major updates, including a redesigned dashboard experience, powerful new plotting tools, expanded platform support, and the ability to unlock Serial Studio Pro. Here’s what’s new:

## Major Features

### New OS-style Dashboard
Dashboards now mimic a Windows-like desktop: groups behave like workspaces, and windows can be tiled or freely moved/resized. Ideal for building and navigating complex dashboards fast.

<img width="1463" alt="screenshot" src="https://github.com/user-attachments/assets/ef657e37-79ab-4134-9d4b-da8262591b3f" />

### Serial Studio Pro is Here
You can now unlock Pro features with a license key via **About → License Management**.
Pro adds advanced widgets like the new 3D Plot and XY Plots, plus full MQTT support.

👉🏻 **Get Serial Studio Pro** https://store.serial-studio.com/buy/ba46c099-0d51-4d98-9154-6be5c35bc1ec.

###  New Website Launched
Learn more, get docs, and download builds directly at: https://serial-studio.com/

###  New Translations
New languages: Portuguese, Korean, Ukrainian, Italian, Polish, Czech

### Other features
- Ribbon UI in Project Editor.
- Centralized Preferences.
- New Plot Features.
- Toggle between scatter/interpolated plots.
- Show area under curves.
- Zoom, pan, and use crosshair tracking.
- Pause/resume without disconnecting.
- MQTT 5.0 support, hex frame delimiters, and binary data actions now supported.
- COM port persistence, Raspberry Pi (arm64) builds, and toolbar controls for widgets.

## Bug fixes
- External windows no longer minimize with main window.
- Compass no longer spins when changing from 359° to 0° [[#291](https://github.com/Serial-Studio/Serial-Studio/issues/291)].
- Accelerometer widget no longer crashes on save [[#289](https://github.com/Serial-Studio/Serial-Studio/issues/289)].
- Fixed dropped bytes every 81st frame (thanks [@BerndDonner](https://github.com/BerndDonner)).

## License Update
The source code is now licensed under GPLv3. Official binaries remain under a commercial license. More details [here](https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md).

| Feature / Use Case         | GPL Version *(Build it yourself)* | Free Version *(Official binary)* | Pro Version *(Commercial license)* |
|---------------------------|------------------------------------|----------------------------------|-------------------------------------|
| Commercial Use            | ✅ If GPL terms are respected      | ❌ Personal use only             | ✅ Fully licensed                   |
| Official Support          | ❌ Community only                  | ❌ None                          | ✅ Priority support                |
| Pro Features *(MQTT, XY, 3D)* | ❌ Not included                | ❌ Not included                  | ✅ Included                        |
| Usage Restrictions        | Must release source code           | No commercial use               | Licensed use only                 |
| Precompiled Binary        | ❌ Build required                  | ✅ Included                      | ✅ Included                        |
| Qt Licensing              | Must use open-source Qt            | Handled by developer            | Handled by developer              |
| Activation                | ❌                                 | ❌                               | ✅ License key required           |
| Business Use              | ✅ If GPL compliant                | ❌ Not allowed                   | ✅ Allowed                         |
| Best For                  | Open source devs, students         | Hobbyists, evaluation           | Businesses, professional teams    |

## Contributions
- LTE Example updates from [@tarman3](https://github.com/tarman3)
- AppStream & install path fixes from [@JamesBrosy](https://github.com/JamesBrosy)
- Byte drop bug fix from [@BerndDonner](https://github.com/BerndDonner)

## New Contributors
- [@BerndDonner](https://github.com/BerndDonner)
- [@JamesBrosy](https://github.com/JamesBrosy)

Full Changelog: [v3.0.6 → v3.1.0](https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.6...v3.1.0)

### Cheskums

| File | SHA256 Checksum |
|------|-----------------|
| `Serial-Studio-3.1.0-Linux-arm64.AppImage` | `604d6883057ad9a570d3997b4cd0d5f98a1fd6181e2d9693bc3aa4ab29615173` |
| `Serial-Studio-3.1.0-Linux-x86_64.AppImage` | `017c9fced5473c5af836f52fab935fa7981d0ade1a5a38005be9db2053f6404d` |
| `Serial-Studio-3.1.0-Linux-x86_64.deb` | `6b0a98ca4c33870256ee2bd2ce7e2483c45c386840456bab5e37f81be8bb1fdb` |
| `Serial-Studio-3.1.0-Windows-x86_64.msi` | `400df0962eb8c32f978f492dc38c70b8b3701b7a571814ceed26efae8329c27b` |
| `Serial-Studio-3.1.0-macOS-Universal.dmg` | `e5919ca0952aba29b838446910f30e714f17f3ce106ebd6e068c052dd8ef0dd4` |


[Changes][v3.1.0]


<a id="v3.0.6"></a>
# [Release v3.0.6](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.0.6) - 2024-11-27

### Bug Fix Release

This update addresses a critical crash that occurs immediately after clicking the “Ok” button in this message during startup:

![390274454-f53d3a3d-7fb4-4e91-9f79-816d66c807ee](https://github.com/user-attachments/assets/ab716ea7-a2c5-49f7-87c5-ebe7c9070c26)

The crash occurs only when Serial Studio is configured to load a saved project file from version 3.0.4 or earlier. Upgrading to this release is strongly recommended to avoid such issues.

In addition to the bug fix, this release introduces an experimental feature related to [#233](https://github.com/Serial-Studio/Serial-Studio/issues/233). Under macOS and Linux, users can now manually specify the serial port device they want to connect to. Please note that this feature is experimental and may not work as expected in all cases.

Given the urgency to prevent crashes for users with saved projects from older versions, this update was prioritized and released quickly. A big thank you to [@tarman3](https://github.com/tarman3) for quickly reporting this issue in [#235](https://github.com/Serial-Studio/Serial-Studio/issues/235).

See all changes: [v3.0.5…v3.0.6](https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.5...v3.0.6)

Thanks, and sorry for the inconvenience!
Alex

---

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/12046074390

[Changes][v3.0.6]


<a id="v3.0.5"></a>
# [Release v3.0.5](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.0.5) - 2024-11-26

I’m pleased to announce the release of **Serial Studio v3.0.5**, which includes several bug fixes, new features, and improvements.

#### Bug Fixes
- Improved hexadecimal send widget in the console. ([#229](https://github.com/Serial-Studio/Serial-Studio/issues/229))
- Updated CI build pipeline for macOS. ([#228](https://github.com/Serial-Studio/Serial-Studio/issues/228))
- Added support for more standard baud rates. ([#227](https://github.com/Serial-Studio/Serial-Studio/issues/227))
- Improved BLE module, now allowing users to select device characteristics. ([#220](https://github.com/Serial-Studio/Serial-Studio/issues/220))
- Fixed Chinese character rendering in the console. ([#215](https://github.com/Serial-Studio/Serial-Studio/issues/215))
- Fixed font rendering on Windows when using 100% scaling.

#### New Features & Improvements
- Added Debian and RPM package support for Linux.
- Improved plotting performance with SIMD instructions (SSE2 for Intel/AMD, SVE for ARM).
- Make use of [ring buffers](https://en.wikipedia.org/wiki/Circular_buffer) to process incoming data and store console data.
- Added legends to multiplot widgets. ([#221](https://github.com/Serial-Studio/Serial-Studio/issues/221))
- Added more options for frame delimiters in project files.
- Improved the frame parser code editor widget.
- Switched to native system fonts for the UI.
- Improved UI responsiveness during high-frequency frame updates.
- Linux builds are now done using the [Intel oneAPI Compiler (icx)](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler.html).
- Use `QStandardPaths` for better file path handling. ([#219](https://github.com/Serial-Studio/Serial-Studio/pull/219))

#### Documentation & Examples
- Added an LTE modem example. ([#223](https://github.com/Serial-Studio/Serial-Studio/pull/223))
- Added a [Data Flow explanation](https://github.com/Serial-Studio/Serial-Studio/wiki/Data-Flow-in-Serial-Studio) to the wiki.
- Added a Function Generator example for stress testing Serial Studio.

#### New Contributors
Thank you to the new contributors for their help:

- [@CloverGit](https://github.com/CloverGit)
- [@tarman3](https://github.com/tarman3)
- [@dsm](https://github.com/dsm)

#### Full Changelog
See all changes: [v3.0.4…v3.0.5](https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.4...v3.0.5)

---

⚠️ **Important:** The frame parser function format has changed slightly to allow direct specification of a separator sequence within the function implementation. Serial Studio will attempt to automatically update your project if it used the default frame parser function. If you’ve customized the implementation, you’ll need to update the function manually to match the new format. Serial Studio will notify you if this applies.

---

### If you find Serial Studio helpful, please consider [making a donation](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) via PayPal. Your support is greatly appreciated!

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/12033556287

[Changes][v3.0.5]


<a id="v3.0.4"></a>
# [Release v3.0.4](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.0.4) - 2024-11-05

Yet another minor bug-fix release. This update smooths out a few issues in the CSV player. Moving backward and forward in plots now works as expected, and you can now use keyboard shortcuts: Space for play/pause, and Left/Right arrows to navigate frames—just like in a regular media player. Interval ticks in the plots are also more readable, now preferring integer values over floating points in each tick whenever possible.

For the complete list of changes, please visit: [`v3.0.3...v3.0.4`](https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.3...v3.0.4)

Thanks again for all your support and feedback, and a special shoutout to [@arturodrt](https://github.com/arturodrt) for identifying this issue!

### If Serial Studio has been helpful for your work, please consider [supporting me](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) through a donation via PayPal!

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/11678045800

[Changes][v3.0.4]


<a id="v3.0.3"></a>
# [Release v3.0.3](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.0.3) - 2024-11-04

Another minor bug-fix release. This update resolves an issue where widget or dataset colors would default to black when the dataset index surpassed the available color range in the current theme. With this fix, widget color assignments now loop correctly, ensuring consistent and accurate visual representation across all datasets. Additionally, this release suppresses quick-plot warnings for non-numerical data, minimizing distractions when working with interactive CLI interfaces.

For the complete list of changes, please visit: [`v3.0.2...v3.0.3`](https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.2...v3.0.3)

Again, thank you all for the continued support and feedback!

### If Serial Studio has been helpful for your work, please consider [supporting me](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) through a donation via PayPal!

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/11658597852

[Changes][v3.0.3]


<a id="v3.0.2"></a>
# [Release v3.0.2](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.0.2) - 2024-11-03

Minor bug-fix release. This update resolves an issue that caused a crash when the FFT plot count differed from the linear plot count.

For the complete list of changes, please visit: [`v3.0.1...v3.0.2`](https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.1...v3.0.2)

Thank you all for the continued support and feedback! Looking forward to seeing what you build with this update. 

### If Serial Studio has been helpful for your work, please consider [supporting me](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) through a donation via PayPal!

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/11653591443

[Changes][v3.0.2]


<a id="v3.0.1"></a>
# [Release v3.0.1](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.0.1) - 2024-11-03

Hey everyone! I'm pleased to share the **v3.0.1** release. This update introduces several new themes and addresses a minor issue where widget view checkboxes didn’t align with widget colors when switching themes.

Check out the full changelog here: [`v3.0.0...v3.0.1`](https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.0...v3.0.1)

Thank you all for the continued support and feedback! Looking forward to seeing what you build with this update. 

### If Serial Studio has been a helpful tool for your projects, please consider [making a donation](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) via PayPal!

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/11649221639

[Changes][v3.0.1]


<a id="v3.0.0"></a>
# [Release v3.0.0](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v3.0.0) - 2024-10-29

Hey everyone! I'm pleased to share the **v3.0.0** release, packed with significant updates to the UI, performance enhancements, and new features designed to make Serial Studio faster, smoother, and more intuitive. I’d also be grateful for any support—whether through [donations](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE), recommending Serial Studio to friends and colleagues, writing blog posts, creating videos, or connecting with us on Instagram at [@serialstudio.app](https://www.instagram.com/serialstudio.app). Let's build a community, showcase projects, and share new ideas!

## What's Changed
* **Complete UI Redesign**: The interface has been fully revamped for a cleaner look and easier navigation. Widgets now render on the GPU instead of the CPU, providing a notable boost in performance and responsiveness.
* **New Project Editor**: The Project Editor has been completely reworked with a modern layout, better customization options, and an improved user experience.
* **User-Defined Actions**: Create custom buttons with text and icons to send commands directly to your microcontroller, simplifying interaction and control, more information [here](https://github.com/Serial-Studio/Serial-Studio/issues/205#issuecomment-2375438008).
* **Support for Hexadecimal and Base64 data frames:** Added more flexibility for handling different data formats with Hexadecimal and Base64 support. A custom [frame parser](https://github.com/Serial-Studio/Serial-Studio/wiki/Project-Editor#frame-parser-function-view) function will be required, example available [here](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples/HexadecimalADC).
* **Quick Plot Mode**: Quickly visualize comma-separated frames and generate CSV files without needing to create a project file. Learn more [here](https://github.com/Serial-Studio/Serial-Studio/wiki/Getting-Started#connecting-a-device-quick-plot-mode).
* **Expanded VT-100 Emulation Support**: Enhanced terminal emulation provides a more usable console experience when connecting to interactive CLI interfaces (such as TELNET servers).
* **Additional Themes & Translations**: Added more language support and new themes, making the app more accessible and customizable.
* **Improved Terminal Widget**: Rebuilt from scratch, the terminal now features word wrapping, custom rendering, and overall better performance.
* **FFT Plot Enhancements**: Added dB magnitude calculations and adjustable sampling rates, allowing for more flexible data analysis.
* **Upgraded to Qt 6.8 LTS**: This upgrade ensures stability and compatibility with long-term support.

## Other Enhancements
* Improved CSV player and export handling.
* Better MQTT ClientID management and extended BLE device support.
* Added [example projects](https://github.com/Serial-Studio/Serial-Studio/tree/master/examples), including MPU6050 and TinyGPS integrations.
* Rewrote the [wiki](https://github.com/Serial-Studio/Serial-Studio/wiki) to keep documentation up-to-date and accessible.

⚠️ You might need to adjust some project settings. The [Project Editor](https://github.com/Serial-Studio/Serial-Studio/wiki/Project-Editor) is **highly** recommended for managing configurations instead of manual JSON editing. 

**Full Changelog**: https://github.com/Serial-Studio/Serial-Studio/compare/v2.0.0...v3.0.0

Thank you all for the continued support and feedback! Looking forward to seeing what you build with this update.

### New Contributors

- [@ZhiyuanYuanNJ](https://github.com/ZhiyuanYuanNJ) improved the serial port list for a more user-friendly experience ([#191](https://github.com/Serial-Studio/Serial-Studio/issues/191)).
- [@recursivenomad](https://github.com/recursivenomad) resolved a long-standing issue with serial port connections on Windows ([#198](https://github.com/Serial-Studio/Serial-Studio/issues/198)) and fixed several bugs in the console widget ([#200](https://github.com/Serial-Studio/Serial-Studio/issues/200)).

### If you find Serial Studio useful for your projects, please consider [donating](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) through PayPal!

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/11567548857

[Changes][v3.0.0]


<a id="v2.0.0"></a>
# [Release v2.0.0](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v2.0.0) - 2024-03-21

Hey everyone! I apologize for my limited activity recently. My daily responsibilities have kept me quite busy, leaving me less free time than desired...I've observed that many new issues have emerged from versions 1.1.6 and 1.1.7, and I intend to concentrate on the latest codebase instead of re-directing users to download the continuous release. Wishing all of you the best and thank you for your understanding!

## What's Changed
* Upgraded and renamed the JSON editor to "Project Editor" with newer functionality.
* Introduced a new feature allowing custom parsing code implementation, ideal for processing binary data or handling multiple frame formats within the same project.
* Added support for interacting with Bluetooth Low Energy (BLE) devices. Let me know if you are able to plot data from your smartwatch... 😆 
* Streamlined the process for selecting device types.
* Implemented other minor user interface enhancements.
* Grammatical and Conciseness Rewording by [@DanPark13](https://github.com/DanPark13) in [#99](https://github.com/Serial-Studio/Serial-Studio/pull/99)
* Update README.md by [@ABHISAHN](https://github.com/ABHISAHN) in [#114](https://github.com/Serial-Studio/Serial-Studio/pull/114)
* add portable application by [@zfb132](https://github.com/zfb132) in [#126](https://github.com/Serial-Studio/Serial-Studio/pull/126)
* update chinese translations by [@zfb132](https://github.com/zfb132) in [#128](https://github.com/Serial-Studio/Serial-Studio/pull/128)
* Update several zh.ts wording by [@Tetrapod1206](https://github.com/Tetrapod1206) in [#179](https://github.com/Serial-Studio/Serial-Studio/pull/179)

⚠️ You **will** need to re-generate your project files, it is **highly** recommended to use the Project Editor instead of editing the JSON code yourself.

## New Contributors
* [@DanPark13](https://github.com/DanPark13) made their first contribution in [#99](https://github.com/Serial-Studio/Serial-Studio/pull/99)
* [@ABHISAHN](https://github.com/ABHISAHN) made their first contribution in [#114](https://github.com/Serial-Studio/Serial-Studio/pull/114)
* [@zfb132](https://github.com/zfb132) made their first contribution in [#126](https://github.com/Serial-Studio/Serial-Studio/pull/126)
* [@Tetrapod1206](https://github.com/Tetrapod1206) made their first contribution in [#179](https://github.com/Serial-Studio/Serial-Studio/pull/179)

**Full Changelog**: https://github.com/Serial-Studio/Serial-Studio/compare/v1.1.7...continuous

[Changes][v2.0.0]


<a id="v1.1.7"></a>
# [Serial Studio v1.1.7 (bug fix)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.7) - 2022-02-11

## Changes from [v1.1.6](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.6):
- Fix a regression that caused issues [#77](https://github.com/Serial-Studio/Serial-Studio/issues/77) and [#78](https://github.com/Serial-Studio/Serial-Studio/issues/78) to appear again (unmatched dashboard items for indexes greater than 9)

### If you find Serial Studio useful for your projects, please consider [donating](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) through PayPal!

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/1827301276

[Changes][v1.1.7]


<a id="v1.1.6"></a>
# [Serial Studio 1.1.6 (v1.1.6)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.6) - 2022-02-07

## Changes from [v1.1.4](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.4):

- Allow enabling/disabling custom window decorations.
- Fix memory leaks ([#85](https://github.com/Serial-Studio/Serial-Studio/issues/85)).
- Fix FFT issue in JSON editor.
- Minor `clang-tidy` fixes ([#85](https://github.com/Serial-Studio/Serial-Studio/issues/85)).
- Re-enable JS expression parsing (quick fix for [#88](https://github.com/Serial-Studio/Serial-Studio/issues/88)).
- Allow parsing received UDP datagrams as frames without using data delimiters ([#90](https://github.com/Serial-Studio/Serial-Studio/issues/90)).
- Improve widget rendering process ([#85](https://github.com/Serial-Studio/Serial-Studio/issues/85)).
- Fix glitches in Qwt-based widgets when displayed on HiDPI screens.
- Use references instead of pointers for singleton classes.
- Add translations for message box buttons.
- Redesign splash-screen & application icon.
- Fix freezed toolbar buttons.
- Remove legacy `JFI` module.
- Add partial support for Raspberry Pi (https://github.com/Serial-Studio/Serial-Studio/discussions/81).
- Fixes for bugs & leaks detected with the address sanitizer ([#85](https://github.com/Serial-Studio/Serial-Studio/issues/85)).
- Compact JSON map file before frame processing to improve dashboard generation speeds ([#85](https://github.com/Serial-Studio/Serial-Studio/issues/85)).
- Work on issue [#82](https://github.com/Serial-Studio/Serial-Studio/issues/82) (e.g. when frames are received with partial data).
- Export dataset titles in CSV file ([#89](https://github.com/Serial-Studio/Serial-Studio/issues/89)). 
- Change default theme to "flat light".

### If you find Serial Studio useful for your projects, please consider [donating](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) through PayPal!

Build log: https://github.com/Serial-Studio/Serial-Studio/actions/runs/1805795371

[Changes][v1.1.6]


<a id="v1.1.4"></a>
# [Release v1.1.4](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.4) - 2021-12-07

## Changes from [v1.1.2](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.2):

- Use frameless windows for better integration with selected themes.
- QML optimizations.
- Fix slow behavior [#85](https://github.com/Serial-Studio/Serial-Studio/issues/85).
- Add support for SSL/TLS for MQTT connections [#34](https://github.com/Serial-Studio/Serial-Studio/issues/34).
- Fix CSV crash when using a modified JSON file [#82](https://github.com/Serial-Studio/Serial-Studio/issues/82).
- Fix data parsing issue [#86](https://github.com/Serial-Studio/Serial-Studio/issues/86) and [#79](https://github.com/Serial-Studio/Serial-Studio/issues/79).
- Add GPS map widget again (thanks to QMapControl.
- Downgrade to Qt 5 (Qt 6 works, but it still has some issues).
- Force non-threaded rendering to avoid crashes and high CPU usage.
- Add more descriptive JSON keys/tags, old JSON maps should be automatically converted without any issues [#83](https://github.com/Serial-Studio/Serial-Studio/issues/83). 
- Misc UI bug fixes & improvements.
- Add new MQTT setup dialog.
- Cleanup setup panel.
- Add more thoughtful animations when transitioning between console and dashboard tab.
- Allow dragging the window when clicking the toolbar.
- Allow customizing number of decimal digits to display.
- Allow customizing widget size.
- Add integrated downloader for the automatic updater.
- Add relative scale to multiplot widget datasets (just define min/max for each dataset).

🗒️ **Note:** this version shall be released with tag 1.1.4 so that continuous users get the update notification. From now on, odd version number releases (such as 1.1.3 or 1.1.5) shall be reserved for development (continuous) releases, and even version number releases shall be reserved for regular/stable builds.

⚠️ **Warning:** CSV and JSON map files generated with this version will not work with older versions of Serial Studio. Also, if your JSON maps do not define datasets in the same order as the frame fields, you might experience some issues while trying to "play" CSV files generated with previous releases of Serial Studio.

### If you find Serial Studio useful for your projects, please consider [donating](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) through PayPal!

You can tip whatever amount you find appropriate, have a great holiday season!🎄 

[Changes][v1.1.4]


<a id="v1.1.2"></a>
# [Serial Studio 1.1.2 (v1.1.2)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.2) - 2021-11-01

## Changes from [v1.1.1](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.1):

- Fix issues with JSON maps with duplicate dataset indexes.
- Allow enabling/disabling multithreaded frame parsing.
- Add console window in dashboard pane.
- Misc UI improvements.
- Allow users to control widget size hints.
- Allow users to control number of decimal places displayed on widgets.
- Default to software rendering to avoid glitches with OpenGL or native render engines.
- Allow multiple plot widgets to specify min/max values.
- Increase graphical cache buffer in JSON editor window.
- Changed splash screen.

[Changes][v1.1.2]


<a id="v1.1.1"></a>
# [Serial Studio 1.1.1 (v1.1.1)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.1) - 2021-10-25

## Changes from [v1.1.0](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.0):

- Fix UDP socket issues ([#74](https://github.com/Serial-Studio/Serial-Studio/issues/74) and [#75](https://github.com/Serial-Studio/Serial-Studio/issues/75)).
- Fix JSON map parsing for "skipped" indexes ([#77](https://github.com/Serial-Studio/Serial-Studio/issues/77)).
- Add LED status panel for binary sensor and project status flags ([#47](https://github.com/Serial-Studio/Serial-Studio/issues/47)).
- Make console window detachable.
- Increase width of setup panel.
- Redesign UDP socket UI options:
    - You can now select both the local and the remote port.
    - Type "0" in the local port field to let the operating system assign the local port automatically.

## Sponsors for this release

- Marco Brianza ([@marcobrianza](https://github.com/marcobrianza))

Want to become a sponsor? You can [donate](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) through Paypal or [sponsor me directly](https://github.com/sponsors/alex-spataru) on GitHub!



[Changes][v1.1.1]


<a id="v1.1.0"></a>
# [Serial Studio 1.1.0 (v1.1.0)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.1.0) - 2021-10-20

Changes from [v1.0.23](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.23):
- Unify dashboard & widgets page.
- Add splash screen.
- Allow users to display/hide multiple widget groups at once.
- Move the `Serial Studio` folder to `Documents` & save JSON map files by default in that directory.
- Implement logarithmic plots ([#55](https://github.com/Serial-Studio/Serial-Studio/issues/55)).
- Implement FFT (Fast Fourer Transform) plots ([#49](https://github.com/Serial-Studio/Serial-Studio/issues/49)).
- Add alarm level warning in bar widgets.
- Implement multiple data plots ([#33](https://github.com/Serial-Studio/Serial-Studio/issues/33)).
- Transition to Qt 6.
- Allow users to select UI rendering engine (e.g. OpenGL, DirectX, Software rendering, etc.).
- Add optional data integrity checks for incoming frames ([#30](https://github.com/Serial-Studio/Serial-Studio/issues/30)).
- Fix UDP socket issues ([#74](https://github.com/Serial-Studio/Serial-Studio/issues/74)).
- Allow connecting to UDP multicast groups ([#74](https://github.com/Serial-Studio/Serial-Studio/issues/74)).
- Add "Noir" theme.
- Add JSON project tree display in the JSON Editor window.
- Redesign widget & plots architecture (now all widgets are implemented with Qwt/C++).
- Improve frame parsing code (many thanks to [@jpnorair](https://github.com/jpnorair)).
- Add auto-reconnect function for serial devices ([#72](https://github.com/Serial-Studio/Serial-Studio/issues/72)).
- Redesign compass widget ([#44](https://github.com/Serial-Studio/Serial-Studio/issues/44)).
- Add widget-related notes & tips in the JSON Editor.

**Warnings:**
- The integrated map widget was temporally modified to display the latitude/longitude/altitude values because Qt 6.2 still does not have support for map SDKs.
- GNU/Linux users should use a distro with  `glibc 2.28` (e.g. Ubuntu 20.04 and onward).  
- Windows users will be prompted to uninstall the previous version of Serial Studio in order to avoid mixing Qt5 and Qt6 libraries in the same directory.

### Issues related with this release shall be discussed [here](https://github.com/Serial-Studio/Serial-Studio/discussions/75).

[Changes][v1.1.0]


<a id="v1.0.23"></a>
# [Serial Studio 1.0.23 (v1.0.23)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.23) - 2021-09-23

Changes from [v1.0.22](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.22):
- Fix issue [#71](https://github.com/Serial-Studio/Serial-Studio/issues/71) .
- Minor optimizations.
- Update AppImage build environment to Ubuntu 18.04 (see [this](https://github.blog/changelog/2021-04-29-github-actions-ubuntu-16-04-lts-virtual-environment-will-be-removed-on-september-20-2021/#:~:text=April%2029%2C%202021-,GitHub%20Actions%20%3A%20Ubuntu%2016.04%20LTS%20virtual%20environment%20will%20be%20removed,Actions%20on%20September%2020%2C%202021.) for more info).

[Changes][v1.0.23]


<a id="v1.0.22"></a>
# [Serial Studio 1.0.22 (v1.0.22)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.22) - 2021-09-19

### Changes from [v1.0.21](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.21):

- Add JSON editor window, which lets you easily create, edit and save Serial Studio project files.
- Allow custom data separator sequence (e.g. use `;` instead of `,`).
- JSON map files can now establish frame start/end sequences & data separator sequence.
- Add support for themes (new theme submissions welcome, [check](https://github.com/Serial-Studio/Serial-Studio/blob/master/assets/themes/0_Dark.json) this file to get started).
- Add Russian translations.
- Add donation links & QR code in about window.
- Allow user to enable/disable widgets on the fly.
- Change application font to Roboto (to avoid UI glitches in non-tested platforms & system themes).
- Change terminal font to Roboto Mono.
- Fix issues with gauge widgets not being displayed in the "Widgets" tab.
- Add macOS Touch Bar buttons.
- Allow users to set the UI refresh rate (e.g. if you need to use higher sampling rates to visualize your data correctly, or if you want to reduce CPU usage).

### Screenshots

![Screen Shot 2021-09-18 at 19 19 48](https://user-images.githubusercontent.com/4225542/133911839-08e32aa7-9134-454f-aa5e-73f6ce976e46.png)

_Serial Studio now supports themes, you can change the theme by going to `Setup` → `Settings` → `Theme`._

![Screen Shot 2021-09-18 at 19 19 48](https://user-images.githubusercontent.com/4225542/133911861-f692d2c0-c078-4767-8367-305daa57d6cb.png)

_The JSON Editor feature in action_




[Changes][v1.0.22]


<a id="v1.0.21"></a>
# [Serial Studio 1.0.21 (v1.0.21)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.21) - 2021-09-04

Changes from [v1.0.20](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.20):

- Add [gauge](https://github.com/Serial-Studio/Serial-Studio/wiki/Introduction-to-widgets#gauge) widget ([#64](https://github.com/Serial-Studio/Serial-Studio/issues/64)).
- Fix bug in binary/hex console display ([#66](https://github.com/Serial-Studio/Serial-Studio/issues/66)).
- Fix Chinese translations ([#57](https://github.com/Serial-Studio/Serial-Studio/issues/57)).
- Handle DNS lookups automatically in network & MQTT tabs.
- Improve VT-100 emulation code & add support for more commands. 

[Changes][v1.0.21]


<a id="v1.0.20"></a>
# [Serial Studio 1.0.20 (v1.0.20)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.20) - 2021-03-14

Changes from [v1.0.19](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.19):
- Add support for compass widget ([#44](https://github.com/Serial-Studio/Serial-Studio/issues/44))
- TCP-based plugin system, you can now interact with Serial Studio by connecting to TCP port 7777 in your applications/scripts.

An example of a "plugin" application (in early stages of development) can be found [here](https://github.com/Kaan-Sat/CC2021-Control-Panel).

[Changes][v1.0.20]


<a id="v1.0.19"></a>
# [Serial Studio 1.0.19 (v1.0.19)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.19) - 2021-03-11

Changes from [v1.0.18](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.18):
- Add support for Serial Studio to act as an MQTT publisher & subscriber (issue [#34](https://github.com/Serial-Studio/Serial-Studio/issues/34))
    - An instance of Serial Studio can be directly connected to the MCU and publish data to the network/internet
    - Another instance of Serial Studio can display this data remotely
- Add acknowledgements dialog
- Update about dialog
 

[Changes][v1.0.19]


<a id="v1.0.18"></a>
# [Serial Studio 1.0.18 (v1.0.18)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.18) - 2021-02-21

Changes from [v1.0.17](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.17):
- Add menubar & organize UI for issue [#21](https://github.com/Serial-Studio/Serial-Studio/issues/21)  (if you don't like it, you can hide the menubar using the `View` menu or by right-clicking the console).
- Add option to print console data.
- Fix issue [#35](https://github.com/Serial-Studio/Serial-Studio/issues/35) by creating a [new frame identification/sorting system](https://github.com/Serial-Studio/Serial-Studio/blob/master/src/JSON/FrameInfo.h#L34).
- Filter `\n` and `\r` characters in CSV generation code.
- Performance improvements.
- Allow users to enter full-screen mode in all platforms.
- Don't delete previous settings with Windows installer.

[Changes][v1.0.18]


<a id="v1.0.17"></a>
# [Serial Studio 1.0.17 (v1.0.17)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.17) - 2021-02-17

Changes from [v1.0.16](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.16):
- Fix minor autoscroll issue ([#21](https://github.com/Serial-Studio/Serial-Studio/issues/21))
- Automatically add spaces to generate byte-oriented HEX view in "send data" textfield ([#21](https://github.com/Serial-Studio/Serial-Studio/issues/21))
- Sort JSON frames based on RX/TX time before writing them in the CSV file.
- Auto-generate the UI controls when a CSV file is opened (do not wait until user presses play button to do that).
- Add basic VT-100 emulation support (e.g. for telnet shells).
- Add support for connecting to TCP & UDP network ports for extended functionality ([#17](https://github.com/Serial-Studio/Serial-Studio/issues/17), see footnote).
- Add built-in DNS address resolution/lookup feature.
- UI changes in setup pane.
- Fix issue with empty console text on first run.

**Footnote:** Serial Studio can now interface with both serial devices and network devices (e.g. a device on your LAN or any server on the internet), you can select socket type (TCP/UDP) and port number. Basic VT-100 functionality has been added to the console widget in order to make interfacing with remote shell programs bearable. For example, you can now watch ASCII Star Wars (episode 4) directly from Serial Studio:

![Death Star](https://user-images.githubusercontent.com/4225542/108137622-3f04a980-708a-11eb-9dd8-5739a4e75d8c.png)

Server information:
- Address: `towel.blinkenlights.nl`
- Port: `23`




[Changes][v1.0.17]


<a id="v1.0.16"></a>
# [Serial Studio 1.0.16 (v1.0.16)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.16) - 2021-02-14

Changes from [v1.0.15](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.15):
- Multithreading support for JSON frame interpretation.
- Improvements in serial console ([#21](https://github.com/Serial-Studio/Serial-Studio/issues/21)).
- Fix scrolling issues in serial console ([#21](https://github.com/Serial-Studio/Serial-Studio/issues/21)).
- Allow custom start/end sequences ([#31](https://github.com/Serial-Studio/Serial-Studio/issues/31)).
- Allow executing Javascript code in JSON value (`v`) fields (e.g. `Math.cos(%1)`, for more info check [#29](https://github.com/Serial-Studio/Serial-Studio/issues/29)).
- Add console widget shortcuts ([#21](https://github.com/Serial-Studio/Serial-Studio/issues/21)).
- Unlimited console buffer size (for save/export operations, UI scrollback is limited to 12'000 lines, [#21](https://github.com/Serial-Studio/Serial-Studio/issues/21)).  
- Major QML performance improvements based on QML-profiler observations.
- Add better CSV-export timings (get date/time as soon as raw frame is read in serial manager module).
- Minor UI changes in sub-windows.
- Add global timer-based events to improve performance and reduce chance of overloading the UI thread in high-freq. usage cases.
- Fix issue where console stopped displaying data while the terminal control was not visible  ([#22](https://github.com/Serial-Studio/Serial-Studio/issues/22)).
- Do not add extra window flags to graph, group and widget windows  ([#22](https://github.com/Serial-Studio/Serial-Studio/issues/22)).
- Fix CSV player button icon sync issue ([#22](https://github.com/Serial-Studio/Serial-Studio/issues/22)).

[Changes][v1.0.16]


<a id="v1.0.15"></a>
# [Serial Studio 1.0.15 (Bug fix release) (v1.0.15)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.15) - 2021-02-08

Changes from [v1.0.14](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.14):
- Fixes issue where CSV playback does not display any graphs or widgets ([#22](https://github.com/Serial-Studio/Serial-Studio/issues/22)).
- Minor UI changes.
- **Edit:** fix command history not working because of conflicting shortcut handling methods.

[Changes][v1.0.15]


<a id="v1.0.14"></a>
# [Serial Studio 1.0.14 (v1.0.14)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.14) - 2021-02-08

Changes from [v1.0.13](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.13):
- Use local time in exported CSV file (issue [#22](https://github.com/Serial-Studio/Serial-Studio/issues/22))
- Implement console/terminal features requested in issue [#21](https://github.com/Serial-Studio/Serial-Studio/issues/21). 
- History selection in the "send command" text field (issue [#20](https://github.com/Serial-Studio/Serial-Studio/issues/20)).
- Allow selecting & copying terminal text.
- Allow changing the baud rate without disconnecting from the device.
- Fix input mask errors in the "send command" text field when sending binary/hex data.
- Change software architecture to allow the implementation of multiple data sources (e.g. ethernet) in the future.
- Allow users to show groups, graphs & widgets in separate windows by double-clicking the "small" widget.
- Add support for shortcuts (e.g. `Ctrl + D` shows dashboard, `Ctrl + T` shows the console and `Ctrl + ,` toggle setup panel).
- Allow exporting console data to external file (note: console scrollback is limited to 5000 lines to avoid performance issues).
- Add CLI options `--version` and `--reset` (mostly for testing CI builds).
- Performance improvements in dashboard & widget sections.

[Changes][v1.0.14]


<a id="v1.0.13"></a>
# [Serial Studio 1.0.13 (v1.0.13)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.13) - 2021-02-02

Changes from [v1.0.12](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.12):
- Fix issue with GPS maps on Windows ([#19](https://github.com/Serial-Studio/Serial-Studio/issues/19)).
- Allow users to input custom baud rates ([#12](https://github.com/Serial-Studio/Serial-Studio/issues/12)).
- Add connect/disconnect button ([#18](https://github.com/Serial-Studio/Serial-Studio/issues/18)).
- Reduce UI clutter in the main toolbar.
- Move "logs" button to about window.
- Allow users to zoom & tilt GPS maps using sliders.
- Fix issue where Visual Studio Redistributable installer triggered an automatic reboot ([#14](https://github.com/Serial-Studio/Serial-Studio/issues/14)).


[Changes][v1.0.13]


<a id="v1.0.12"></a>
# [Serial Studio 1.0.12 (v1.0.12)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.12) - 2021-01-30

Changes from [v1.0.11](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.11):
- Change layout of setup pane to be more compact
- Allow users to select from three console visualization modes (plain text, remove control characters & HEX)
- Allow users to send HEX data to serial device
- Reduce minimum window size (to support smaller displays)
- Fix issue where sent data was not displayed on the console window

[Changes][v1.0.12]


<a id="v1.0.11"></a>
# [Serial Studio 1.0.11 (v1.0.11)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.11) - 2021-01-24

Changes from [v1.0.10](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.10):
- Fix issue when dealing with CSV files that where created from a computer with another locale (use month numbers instead of month names in first column of CSV file).
- Add German translation (many thanks to [@fusedFET](https://github.com/fusedFET)).
- Improve console buffer handling (only use resources for displaying visible text).
- Fix compilation errors on Qt 5.12.
- Rename 'device manager' to 'setup'.
- Add discrete toolbar gradient.
- Add option to clear console output.
- Allow more baud rates.
- Add support for drag-and-drop for JSON & CSV files.

**NOTE:** If you are using the CSV player feature, you will need to manually change the first column of your current CSV documents. For example, `2021/Jan/11/ 12:02:19::601` must be changed to `2021/01/11/ 12:02:19::601`. Sorry for the inconvenience 😕 

[Changes][v1.0.11]


<a id="v1.0.10"></a>
# [Serial Studio 1.0.10 (v1.0.10)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.10) - 2021-01-20

Changes from [v1.0.9](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.9):
- Display non-printable characters as hexadecimal in console.
- Fix crash when trying to open an unavailable serial port (related to issue [#3](https://github.com/Serial-Studio/Serial-Studio/issues/3)).
- Add handler function for serial port errors.
- Fix Chinese translations (many thanks to [@jiangjiangjun](https://github.com/jiangjiangjun)).
- Add option control Read / Write open mode of serial port.


[Changes][v1.0.10]


<a id="v1.0.9"></a>
# [Serial Studio 1.0.9 (v1.0.9)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.9) - 2021-01-18

Changes from [v1.0.8](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.8):
- Fix issue where JSON map file could not be selected when using Chinese translation.
- Fix issue where console output would not be automatically removed when receiving invalid characters.
- Only interpret serial frames when data is received (having a separate loop is reduces UI performance under heavy load).

**Edit:** upload new Windows build that includes [OpenSSL](https://www.openssl.org) binaries, which are need for the updater to work and for downloading GPS map images.

[Changes][v1.0.9]


<a id="v1.0.8"></a>
# [Serial Studio 1.0.8 (v1.0.8)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.8) - 2021-01-17

Changes from [v1.0.7](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.7):
- Modify JSON/serial validation procedure
- Add serial port error handlers
- Fix scrollbar issues in group windows
- Add initial Chinese translations
- Create log file in temp. folder
- Save maximized window state between runs
- Fix slow performance caused by console window

[Changes][v1.0.8]


<a id="v1.0.7"></a>
# [Serial Studio 1.0.7 (v1.0.7)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.7) - 2021-01-14

Changes from [v1.0.6](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.6):
- Fixes issue [#1](https://github.com/Serial-Studio/Serial-Studio/issues/1)
- Resize CSV Player dialog to fit contents
- Ask users if they want Serial Studio to check for updates automatically on second run
- Improve algorithm used to check if the JSON map matches the format of the received serial data frame


[Changes][v1.0.7]


<a id="v1.0.6"></a>
# [Serial Studio 1.0.6 (v1.0.6)](https://github.com/Serial-Studio/Serial-Studio/releases/tag/v1.0.6) - 2021-01-14

Changes from initial release:
- Fix window flags on About & CSV replay dialogs (which only affected Windows users).
- Fix minor UI glitches in GNU/Linux.
- Use MSVC compiler to build Serial Studio on Windows.
- Check that the format of the received data frames corresponds to the format specified by the JSON map file.
- Show application icon & size in Control Panel / Settings on Windows.
- Set compilation flags to optimize app for execution speed.
- All builds are now generated from GitHub.


[Changes][v1.0.6]


[v3.1.1]: https://github.com/Serial-Studio/Serial-Studio/compare/v3.1.0...v3.1.1
[v3.1.0]: https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.6...v3.1.0
[v3.0.6]: https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.5...v3.0.6
[v3.0.5]: https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.4...v3.0.5
[v3.0.4]: https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.3...v3.0.4
[v3.0.3]: https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.2...v3.0.3
[v3.0.2]: https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.1...v3.0.2
[v3.0.1]: https://github.com/Serial-Studio/Serial-Studio/compare/v3.0.0...v3.0.1
[v3.0.0]: https://github.com/Serial-Studio/Serial-Studio/compare/v2.0.0...v3.0.0
[v2.0.0]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.1.7...v2.0.0
[v1.1.7]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.1.6...v1.1.7
[v1.1.6]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.1.4...v1.1.6
[v1.1.4]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.1.2...v1.1.4
[v1.1.2]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.1.1...v1.1.2
[v1.1.1]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.1.0...v1.1.1
[v1.1.0]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.23...v1.1.0
[v1.0.23]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.22...v1.0.23
[v1.0.22]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.21...v1.0.22
[v1.0.21]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.20...v1.0.21
[v1.0.20]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.19...v1.0.20
[v1.0.19]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.18...v1.0.19
[v1.0.18]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.17...v1.0.18
[v1.0.17]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.16...v1.0.17
[v1.0.16]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.15...v1.0.16
[v1.0.15]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.14...v1.0.15
[v1.0.14]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.13...v1.0.14
[v1.0.13]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.12...v1.0.13
[v1.0.12]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.11...v1.0.12
[v1.0.11]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.10...v1.0.11
[v1.0.10]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.9...v1.0.10
[v1.0.9]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.8...v1.0.9
[v1.0.8]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.7...v1.0.8
[v1.0.7]: https://github.com/Serial-Studio/Serial-Studio/compare/v1.0.6...v1.0.7
[v1.0.6]: https://github.com/Serial-Studio/Serial-Studio/tree/v1.0.6

<!-- Generated by https://github.com/rhysd/changelog-from-release v3.9.0 -->
