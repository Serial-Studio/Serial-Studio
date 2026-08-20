# Linux installation

## Overview

Serial Studio ships three package formats for Linux, each available for x86_64 (`x64`) and
ARM64 (`arm64`). All of them come from the [releases page](https://github.com/Serial-Studio/Serial-Studio/releases/latest)
and contain the same application; pick the format that fits your distribution.

| Format   | File name                                  | Best for |
|----------|--------------------------------------------|----------|
| AppImage | `Serial-Studio-Pro-<version>-Linux-<arch>.AppImage` | Any distribution, no root required, portable |
| DEB      | `Serial-Studio-Pro-<version>-Linux-<arch>.deb`      | Debian, Ubuntu, Mint, and derivatives |
| RPM      | `Serial-Studio-Pro-<version>-Linux-<arch>.rpm`      | Fedora, RHEL, openSUSE, and derivatives |

A [Flathub package](https://flathub.org/apps/com.serial_studio.Serial-Studio) is also
available; it follows Flatpak's own signing and update mechanism and is not covered here.

Each format also exists as a `-compat` variant for distributions older than the standard
packages support; see the Legacy distributions section below.

## Signing key

> **Warning:** package signing was introduced after version 4.0.3, so the 4.0.3 packages are
> unsigned and fail the verification steps below. Signed packages are currently available
> from the [continuous build](https://github.com/Serial-Studio/Serial-Studio/releases/tag/continuous)
> only; version 4.0.3 will be the first signed release.

Release packages are signed with the Serial Studio release key:

| Field       | Value |
|-------------|-------|
| Owner       | Alex Spataru \<alex@serial-studio.com\> |
| Type        | RSA 4096 |
| Fingerprint | `3303 D913 F9F1 2D5F 2BC3 9852 EBB9 68D7 47D1 1692` |
| Download    | [https://serial-studio.com/gpg-key.asc](https://serial-studio.com/gpg-key.asc) |

The RPM carries the signature in its package header, the DEB carries a `debsigs` origin
signature, and the AppImage embeds a signature in its ELF payload. What your system checks
automatically differs per format; the sections below spell it out.

## RPM (Fedora, RHEL, openSUSE)

`rpm`, `dnf`, and `zypper` verify the header signature of standalone `.rpm` files. Without
the release key in the RPM database, installation warns about an unknown or missing key (for
example `Signature verification failed` or a prompt to trust an unknown key). Import the key
once:

```bash
sudo rpm --import https://serial-studio.com/gpg-key.asc
```

Verify the download:

```bash
rpm -K Serial-Studio-Pro-*-Linux-x64.rpm
```

Expected output:

```text
Serial-Studio-Pro-4.0.3-Linux-x64.rpm: digests signatures OK
```

Install with your package manager so dependencies resolve automatically:

```bash
sudo dnf install ./Serial-Studio-Pro-*-Linux-x64.rpm     # Fedora, RHEL
sudo zypper install ./Serial-Studio-Pro-*-Linux-x64.rpm  # openSUSE
```

The package installs the `serial-studio-pro` binary to `/usr/bin` and adds a **Serial Studio
Pro** entry to the application menu. Uninstall with `sudo dnf remove serial-studio-pro` (or
the `zypper` equivalent).

## DEB (Debian, Ubuntu, Mint)

```bash
sudo apt install ./Serial-Studio-Pro-*-Linux-x64.deb
```

The leading `./` matters: it tells `apt` to install a local file instead of searching the
configured repositories. Uninstall with `sudo apt remove serial-studio-pro`.

The package embeds a `debsigs` origin signature (the `_gpgorigin` member inside the `.deb`).
Note that `apt` and `dpkg` do not verify signatures of individual package files; on
Debian-based systems, trust normally flows through signed repository metadata, which does not
apply to a direct download. The embedded signature exists for auditing tools such as
`debsig-verify`; the download itself is protected by HTTPS.

## AppImage

The AppImage runs on any distribution without installation:

```bash
chmod +x Serial-Studio-Pro-*-Linux-x64.AppImage
./Serial-Studio-Pro-*-Linux-x64.AppImage
```

If launching fails with a FUSE error, install `libfuse2` (recent Ubuntu releases ship only
FUSE 3 by default):

```bash
sudo apt update && sudo apt install libfuse2
```

Without FUSE, the AppImage still works via self-extraction:

```bash
./Serial-Studio-Pro-*-Linux-x64.AppImage --appimage-extract
./squashfs-root/AppRun
```

The AppImage embeds a GPG signature from the release key. Display it with:

```bash
./Serial-Studio-Pro-*-Linux-x64.AppImage --appimage-signature
```

## Legacy distributions (glibc 2.28)

The standard packages require glibc 2.34 or newer because they ship the official Qt
libraries. Older distributions from the RHEL 8 / Debian 10 generation refuse to start them
with an error such as `version 'GLIBC_2.34' not found`.

For those systems, every release also includes `-compat` variants of all three formats:

| Format   | File name |
|----------|-----------|
| AppImage | `Serial-Studio-Pro-<version>-Linux-<arch>-compat.AppImage` |
| DEB      | `Serial-Studio-Pro-<version>-Linux-<arch>-compat.deb`      |
| RPM      | `Serial-Studio-Pro-<version>-Linux-<arch>-compat.rpm`      |

The compat packages bundle their own C runtime next to the application and launch it through
that runtime, so the host's glibc version no longer matters. External programs started by
Serial Studio (for example through the Process driver) still run against the host's own
libraries. The tested baseline is RHEL 8 / Rocky Linux 8 and Debian 10 (glibc 2.28), on both
x86_64 and ARM64.

Installation and signature verification work exactly as described for the standard formats;
the compat packages are signed with the same release key. On distributions with glibc 2.34
or newer, prefer the standard packages; the compat variants run there too but are larger.

## Serial port permissions

On most distributions, regular users cannot open serial devices until they join the
`dialout` group (`uucp` on Arch-based systems):

```bash
sudo usermod -a -G dialout $USER
```

Log out and back in for the group change to take effect. More first-connection fixes are in
[Getting Started](Getting-Started.md) and [Troubleshooting](Troubleshooting.md).

## See also

- [Getting Started](Getting-Started.md): first connection walkthrough for all platforms.
- [Windows Installation](Windows-Installation.md): MSI, portable ZIP, and Microsoft Store
  packages on Windows.
- [Troubleshooting](Troubleshooting.md): fixes for common connection and parsing problems.
- [Command Line Interface](Command-Line-Interface.md): flags for running Serial Studio from
  scripts and headless environments.
