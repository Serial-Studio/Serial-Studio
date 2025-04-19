# Serial Studio License

Copyright &copy; 2020–2025 Alex Spataru <https://github.com/alex-spataru>  

All rights reserved where applicable.

Serial Studio is distributed under a **dual-license model**:

- The **GNU General Public License v3 (GPLv3)** applies to the *source code*.
- The **Commercial License** applies to all *official precompiled binaries*.

The source code will always remain licensed under GPLv3. However, **official binaries are not GPLv3-licensed** and require a commercial license for commercial or proprietary use.

## 1. GNU General Public License v3 (GPLv3)

You may use, modify, and redistribute the **source code** of Serial Studio under the terms of the [GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).

### Conditions:
- You **must** build Serial Studio yourself from source, using a GPL-compliant version of Qt (e.g. from the [open-source Qt edition](https://www.qt.io/download-open-source)) **or**
- Obtain a GPLv3-compliant build from a trusted third party (e.g., official Linux distribution or package manager) that uses GPL-compatible dependencies.
- You **must** preserve the GPLv3 license and all copyright notices.
- You **must not** use any proprietary Qt modules or commercially licensed dependencies in your GPLv3 build.

Package manager builds (e.g., from Ubuntu, Arch AUR, etc.) are permitted only if they are built using a fully GPLv3-compliant toolchain (including Qt). The author does not guarantee or certify the GPL-compliance of third-party builds, use them at your own risk.

### Permitted Under GPLv3:
- Forking or modifying the source code for personal, academic, or open-source projects.
- Using your self-built version in a commercial setting, **only if** you fully comply with GPLv3 (including code disclosure, license propagation, etc.).
- Embedding GPLv3-compiled builds into other open-source software (GPL-compatible).

### Not Permitted Under GPLv3:
- Using official precompiled binaries (from serial-studio.com, GitHub, etc.) under GPLv3 — they're not GPL-licensed.
- Using non-GPL-compliant Qt builds (e.g., closed-source or proprietary modules).
- Reusing the name “Serial Studio”, logos, or trademarks — these are **not** covered by the GPL.

**Note:** GPL-licensed builds **exclude** paid features such as MQTT, CANBus, and 3D visualization.

## 2. Commercial License Agreement

All **official precompiled binaries** (e.g., from [serial-studio.com](https://serial-studio.com), GitHub Releases, or any channel managed by Alex Spataru) are licensed under the terms of the **Serial Studio Commercial License Agreement**.

### Key Terms:
- You **may not** use official binaries in commercial, enterprise, or proprietary environments without purchasing a valid license.
- You **may only use the binaries for personal or evaluation purposes** unless a license is purchased.
- Activation is required to unlock Pro features **and** obtain commercial usage rights.

If you download an official binary, you are agreeing to be bound by the Commercial License terms.

Full Commercial License: [https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE_COMMERCIAL.md](https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE_COMMERCIAL.md)

### Commercial License Benefits (with valid license):
- Pro features:
  - MQTT integration
  - CANBus / Modbus decoding
  - XY plotting and 3D visualization
- Use in commercial or enterprise environments
- Priority support (email, GitHub, optional video support)

### Restrictions (Even with a license):
- You **may not** redistribute, reverse engineer, or rebrand the official binaries.
- You **may not** create competing or derivative commercial products based on Serial Studio without written consent.

## 3. Licensing Compliance Notes

### Qt Licensing:
Serial Studio relies on the Qt framework. You are responsible for ensuring your Qt build is GPL-compatible. If you use a commercially licensed Qt build, your resulting build is **not GPL-compliant** and **may not be used without appropriate Qt licensing**.

When in doubt, use [open-source Qt](https://www.qt.io/download-open-source) and **avoid any closed-source Qt modules**.

### Contributor Terms:
By contributing code, documentation, or any intellectual property to Serial Studio, you **grant the maintainer (Alex Spataru) a perpetual, irrevocable right** to license your contributions under:
- GPLv3 (for open-source distribution), **and**
- A commercial license (for use in official precompiled binaries).

This is required to maintain the dual-licensing model. If you do not agree, do not submit contributions.

## 4. Trademark & Branding

The name **“Serial Studio”**, associated logos, icons, and branding elements are **trademarks** of Alex Spataru. These **may not be reused** in derivative projects, forks, or redistributed versions without **explicit written permission**.

Forks must choose a different name and remove all Serial Studio branding to avoid confusion.

## 5. No Warranty

To the fullest extent permitted by law:

> THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Commercial licensees may receive limited support, but **no guarantee is offered** unless explicitly stated in a paid support contract.

## Summary

| Usage                                | Requirements                                          |
|--------------------------------------|-------------------------------------------------------|
| Build from source (GPLv3)            | Free, must use open-source Qt, follow GPL terms       |
| Use official binary (non-commercial) | Free for personal use, no Pro features                |
| Use official binary (commercial)     | Requires valid license, activation, no redistribution |

Still got questions? Think you're a legal edge case?  
Reach out via [GitHub Issues](https://github.com/Serial-Studio/Serial-Studio/issues) or [email](mailto:alex@serial-studio.com).
