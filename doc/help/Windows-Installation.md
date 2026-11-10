# Windows installation

## Overview

You have a board on a USB cable and a Windows PC. The MSI installer is the standard
install; if you are unsure, use it. The portable ZIP runs without installation or
administrator rights. The Store edition installs without an elevation prompt and updates
through the Microsoft Store.

| Channel         | File name / source                                  | Best for |
|-----------------|-----------------------------------------------------|----------|
| MSI installer   | `Serial-Studio-Pro-<version>-Windows.msi`           | Standard install with a Start Menu entry |
| Portable ZIP    | `Serial-Studio-Pro-<version>-Windows-Portable.zip`  | Running without installation or administrator rights |
| Microsoft Store | Store listing (no download file)                     | Automatic updates through the Microsoft Store |

`<version>` is the release number, for example `4.2.1`. The MSI and the ZIP are on the
[releases page](https://github.com/Serial-Studio/Serial-Studio/releases/latest) under
**Assets**. All three are 64-bit (x64) builds; there is no 32-bit or ARM64 build. The Store package
requires Windows 10 version 1809 or later.

These are the Serial Studio Pro builds. Pro features run with a license or during the free
trial; see [Pro vs Free](Pro-vs-Free.md).

## MSI installer

1. On the [releases page](https://github.com/Serial-Studio/Serial-Studio/releases/latest),
   open **Assets** and download the file ending in `-Windows.msi`.
2. Double-click it and follow the setup wizard.
3. If Windows shows a blue "Windows protected your PC" dialog (SmartScreen, which appears for
   downloaded installers it does not recognize), click **More info**, then **Run anyway**.
4. Press the Windows key, type `Serial Studio Pro` and press Enter. The main window opens.

Setup places the application under `C:\Program Files\Serial Studio Pro` and adds a
**Serial Studio Pro** shortcut directly to the Start Menu. Installing a newer MSI upgrades
the existing installation in place. Uninstall from **Settings → Apps → Installed apps**.

To deploy without the wizard, run `msiexec /i Serial-Studio-Pro-<version>-Windows.msi /qn
/norestart` to install and `msiexec /x` with the same file to remove. The release CI tests
both commands.

## Portable ZIP

Right-click `Serial-Studio-Pro-<version>-Windows-Portable.zip`, choose **Extract All** and put
the folder anywhere (a USB stick works). Double-click `Serial Studio Pro.bat` at the root to
start the application. If Windows shows the "Windows protected your PC" dialog,
use the SmartScreen steps from [MSI installer](#msi-installer).

```text
Serial Studio Pro.bat                launcher: double-click this
License.rtf                          license text
Serial-Studio-Pro-v<version>\
  bin\Serial-Studio-Pro.exe          the application
  bin\ss-config.json                 package stamp used by the update check
  (everything else)                  Qt runtime
```

Extracting needs no administrator rights and writes nothing outside the folder. Running the
application does: settings and license activation are stored with `QSettings`, which on
Windows means the registry of your Windows user, so a copied folder starts unactivated on
another PC and you activate again there.

Deleting the folder uninstalls the program but leaves its settings in the registry. Copying
the folder copies the program, not the activation.

## Microsoft Store

Install [Serial Studio Pro from the Microsoft Store](https://apps.microsoft.com/detail/9n5pzlkjcdp7).
It is the same application packaged as MSIX: Windows installs it without an elevation
prompt and keeps it updated through the Store. Uninstall it like any other Store app.

## First launch

### The application does not start

If the MSI or ZIP build of Serial Studio Pro closes at once on a fresh Windows installation, install the
[Microsoft Visual C++ Redistributable (64-bit)](https://aka.ms/vs/17/release/vc_redist.x64.exe)
and launch it again.

### No COM port appears

Plug the board in, right-click Start, open **Device Manager** and look under
**Ports (COM & LPT)**. A working board appears as an entry such as `USB Serial Device (COM3)`;
the COM number is what you pick in Serial Studio. Boards that talk USB directly, such as the
Leonardo or Pico, usually show up on their own. If the
board is missing, or sits under **Other devices** with a yellow warning icon, install the
driver for its USB-to-serial chip:

- [CH340](http://www.wch-ic.com/downloads/CH341SER_EXE.html): many Arduino clones and ESP32 boards.
- [CP210x](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers): many ESP32 boards.
- [FTDI](https://ftdichip.com/drivers/vcp-drivers/): FTDI adapters and some genuine Arduino boards.

The chip is usually named on the board or in its listing. If you do not know it, search the
board name plus "USB serial driver", or try CP210x and CH340 in turn. Once the port shows up,
continue with [Getting Started](Getting-Started.md); [Troubleshooting](Troubleshooting.md)
covers the connection problems that remain.

## Updates

Serial Studio checks for new releases at startup. An MSI install is offered the new MSI, a
portable install the new ZIP (download it and extract it beside the old folder), and a Store
install updates through the Store. Turn the check on or off under **Settings → Startup →
Updates & News → Automatically Check for Updates** (on by default); the About dialog has a
**Check for Updates** button.

Serial Studio reads the package type from `bin\ss-config.json`, where `"packageType"` is
`msi`, `portable` or `msix`. If the stamp is missing, unreadable or unknown, it checks for
Store package identity (`msix`); if that also fails, it sets no package type and the updater uses
its generic Windows entry.

## See also

- [Getting Started](Getting-Started.md): first connection walkthrough for all platforms.
- [Troubleshooting](Troubleshooting.md): fixes for common connection and parsing problems.
- [Linux Installation](Linux-Installation.md): the same steps for Linux.
- [Command Line Interface](Command-Line-Interface.md): headless runs.
