# Linux installation

## Overview

You downloaded a file and a terminal is waiting. Not sure which one to pick? Take the AppImage: one `chmod`, one command, no root.

```bash
cd ~/Downloads
chmod +x Serial-Studio-Pro-*-Linux-x64.AppImage
./Serial-Studio-Pro-*-Linux-x64.AppImage
```

`No such file`? Run `uname -m`: `x86_64` means `x64`, `aarch64` means `arm64` (Raspberry Pi). Swap `x64` for `arm64` in every file name on this page. The `*` is a wildcard that matches the version number, so delete older downloads first.

The main window opens. To see your Arduino's `Serial.println` values, follow [Getting Started](Getting-Started.md).

The commands on this page run from the folder the file landed in, usually `~/Downloads`.

If the port later refuses to open with "permission denied", see [Serial port permissions](#serial-port-permissions).

Release files by format:

| Format   | File name                                  | Best for |
|----------|--------------------------------------------|----------|
| AppImage | `Serial-Studio-Pro-<version>-Linux-<arch>.AppImage` | Any mainstream desktop distribution (Ubuntu, Fedora, Debian, Arch, ...), no root required |
| DEB      | `Serial-Studio-Pro-<version>-Linux-<arch>.deb`      | Debian, Ubuntu, Mint, and derivatives |
| RPM      | `Serial-Studio-Pro-<version>-Linux-<arch>.rpm`      | Fedora, RHEL, openSUSE, and derivatives |

All three come from the [releases page](https://github.com/Serial-Studio/Serial-Studio/releases/latest) for x86_64 (`x64`) and ARM64 (`arm64`) and contain the same application. Packages run on Debian 10 and RHEL 8 or newer ([Supported distributions](#supported-distributions)).

The release packages are the Serial Studio Pro build, which is why the file names say Pro; the installed binary is `serial-studio-pro`. The free GPL-3.0 edition builds from source ([Getting Started](Getting-Started.md#building-from-source-gpl)), or comes from Flathub as `flatpak install flathub com.serial_studio.Serial-Studio` ([Flathub package](https://flathub.org/apps/com.serial_studio.Serial-Studio)), which follows Flatpak's own signing and updates. The Pro features need the release packages.

## RPM (Fedora, RHEL, openSUSE)

`rpm`, `dnf`, and `zypper` verify the header signature of standalone `.rpm` files. Import the [release key](#signing-key) once, before the first install:

```bash
sudo rpm --import https://serial-studio.com/gpg-key.asc
```

Verify the download:

```bash
rpm -K Serial-Studio-Pro-*-Linux-x64.rpm
```

Expected output:

```text
Serial-Studio-Pro-<version>-Linux-x64.rpm: digests signatures OK
```

If it does not print `OK`, do not install: re-download the file and check again.

Install with your package manager so dependencies resolve automatically:

```bash
sudo dnf install ./Serial-Studio-Pro-*-Linux-x64.rpm     # Fedora, RHEL
sudo zypper install ./Serial-Studio-Pro-*-Linux-x64.rpm  # openSUSE
```

Uninstall with `sudo dnf remove serial-studio-pro` or `sudo zypper remove serial-studio-pro`. To start it, open **Serial Studio Pro** from the application menu or run `serial-studio-pro`.

## DEB (Debian, Ubuntu, Mint)

```bash
sudo apt install ./Serial-Studio-Pro-*-Linux-x64.deb
```

The leading `./` matters: it tells `apt` to install a local file instead of searching the
configured repositories. Uninstall with `sudo apt remove serial-studio-pro`.

To start it, open **Serial Studio Pro** from the application menu or run `serial-studio-pro`.

The `.deb` carries an embedded origin signature that `apt` and `dpkg` ignore. You do not need to do anything; the [Signing key](#signing-key) section has the detail for auditing tools.

## AppImage

The AppImage runs without installation: run it as shown in the [Overview](#overview).

If it prints a message mentioning FUSE or `libfuse.so.2` and exits, your system lacks `libfuse2` (Ubuntu 22.04 and newer no longer include it; `sudo apt install libfuse2` fixes it). The AppImage also works without FUSE through self-extraction, which unpacks into `./squashfs-root`:

```bash
./Serial-Studio-Pro-*-Linux-x64.AppImage --appimage-extract
./squashfs-root/AppRun
```

To skip the extra folder, set `APPIMAGE_EXTRACT_AND_RUN=1` and the runtime extracts to a temporary location on each launch:

```bash
APPIMAGE_EXTRACT_AND_RUN=1 ./Serial-Studio-Pro-*-Linux-x64.AppImage
```

The AppImage embeds a GPG signature from the release key. `--appimage-signature` prints it; it does not verify it:

```bash
./Serial-Studio-Pro-*-Linux-x64.AppImage --appimage-signature
```

## Serial port permissions

The adapter is plugged in, the port shows up as `/dev/ttyACM0` or `/dev/ttyUSB0`, and opening it fails with "permission denied". Linux assumes you did not mean to talk to hardware until you join the group that may: your user is not in the `dialout` group yet (`uucp` on Arch-based systems).

Find the port with `ls /dev/ttyACM* /dev/ttyUSB*`, then add yourself to the group:

```bash
sudo usermod -a -G dialout $USER
```

Log out and back in (or reboot), then check with `groups`: `dialout` should be listed.

## Supported distributions

Serial Studio runs on Debian 10, RHEL 8 and Rocky Linux 8 or newer, on x86_64 and ARM64. Musl-based systems such as Alpine are not supported. Every release is smoke-tested on RHEL 8 / Rocky Linux 8 and Debian 10.

Each package carries its own glibc and launches Serial Studio through it, so the glibc on your system does not matter. Programs that Serial Studio starts, for example through the Process driver, still use your system's libraries.

Error `version 'GLIBC_2.34' not found`: you have a release up to 4.0.3, which used the host glibc. Download the latest release.

## Signing key

Optional: checking the key is not required to install. RPM, DEB and AppImage downloads are all signed with the Serial Studio release key. The `.deb` signature is the `debsigs` `_gpgorigin` member, which `debsig-verify` can check:

| Field       | Value |
|-------------|-------|
| Owner       | Alex Spataru \<alex@serial-studio.com\> |
| Type        | RSA 4096 |
| Fingerprint | `3303 D913 F9F1 2D5F 2BC3 9852 EBB9 68D7 47D1 1692` |
| Download    | [https://serial-studio.com/gpg-key.asc](https://serial-studio.com/gpg-key.asc) |

To check that the key you downloaded is the right one:

```bash
curl -O https://serial-studio.com/gpg-key.asc
gpg --show-keys --fingerprint gpg-key.asc
```

The output must show the fingerprint above.

## See also

- [Getting Started](Getting-Started.md): first connection walkthrough for all platforms.
- [Troubleshooting](Troubleshooting.md): fixes for common connection and parsing problems.
