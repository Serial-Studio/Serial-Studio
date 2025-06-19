# Serial Studio License Agreement

**Copyright © 2020–2025**  
**Alex Spataru** (<https://github.com/alex-spataru>)  
**All rights reserved where applicable.**

Serial Studio is the result of years of work. It’s free to explore, build, and contribute to, but not free to exploit. To keep things sustainable and fair for everyone, this project uses a **dual-license model** that separates open contributions from commercial use. The following terms define your rights and obligations when using, modifying, building, or distributing Serial Studio.

## 1. Dual Licensing Overview

Serial Studio is provided under two distinct licensing models:

- The **GNU General Public License v3 (GPLv3)** applies to the open-source portions of the source code, **excluding any gated commercial features** listed in Section 4.
- The **Serial Studio Commercial License** governs:
  - All **official precompiled binaries**.
  - Any **builds (user or third-party) that include gated commercial features**, regardless of how they were compiled or distributed.

> [!NOTE]
> This project contains both open-source and proprietary components.  
> The **source code is transparent**, but **not all parts are open source**.  
> You are free to **use, study, and contribute to** the GPL-licensed portions, but:  
> - **Do not unlock, disable, or remove** activation or license enforcement mechanisms.  
> - **Do not enable or distribute** gated Pro features without a valid license.  
> - **Transparency is not permission** — Pro features remain proprietary, even if visible in the code.

Any inclusion of gated features **automatically revokes GPLv3 coverage** and places the entire build under the Serial Studio Commercial License.

## 2. GPLv3 Licensing Terms (Source Code Use Only)

You may use, modify, and redistribute the source code under the [GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.html), but **only** if **all** the following conditions are strictly met:

### 2.1 Requirements

- You **must compile** Serial Studio from source.
- You **must use a fully GPL-compliant version of Qt**, such as the [open-source Qt distribution](https://www.qt.io/download-open-source).
- You **must not enable, compile, or link** any of the gated commercial features listed in Section 4.
- You **must not** link to any proprietary or commercial-only Qt modules or third-party dependencies.
- You **must preserve** all original license notices and comply with all GPLv3 obligations, including source code availability.

### 2.2 Third-Party Package Distributions

- Builds distributed via package managers (e.g., AUR, Ubuntu, Fedora) **must comply** with all of the above conditions.
- The project maintainer **does not verify or certify** GPL compliance of third-party builds. Users assume all risk when using such builds.

> [!NOTE]
> You can use the `-DBUILD_GPL3=ON` CMake flag to automatically produce a GPL-compatible build.

## 3. Commercial License Terms (Pro Features & Official Binaries)

The **Serial Studio Commercial License** applies to:

- All **official precompiled binaries** distributed via [serial-studio.com](https://serial-studio.com/), GitHub, or any other official channel.
- Any builds, regardless of origin, that include **any gated commercial features** (Section 4).

### 3.1 Key Terms

- Any use of the commercial build or gated features in a **commercial**, **enterprise**, or **non-personal** setting **requires a valid commercial license** from the author.
- Activation is mandatory to unlock gated functionality and obtain legal usage rights.
- Tampering with the activation system is strictly prohibited and constitutes a license violation.
- Downloading or building a version that includes gated features signifies agreement to the [Commercial License Agreement](https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE_COMMERCIAL.md).

### 3.2 Definition: “Commercial Use”

"Commercial use" is broadly defined and includes, but is not limited to:

- Internal use in for-profit entities or by contractors.
- Use in revenue-generating products, services, or systems.
- Use in customer-facing, production, or enterprise environments.
- Inclusion in hardware, SaaS platforms, or research with commercial funding.

**If you’re not sure, you need a commercial license.**

## 4. Gated Commercial Features (Pro Modules)

The following features are **explicitly excluded** from GPLv3 and are **governed exclusively** by the Commercial License:

- Linking against or including the **Qt MQTT** module.
- Linking against or including the **Qt Serial Bus** module.
- MQTT integration (broker/client support).
- XY plotting capabilities.
- 3D visualization tools.
- Activation or license management systems.

> [!NOTE]
> Compiling or linking any of the above features, even in a self-built version, **automatically reclassifies** your build as proprietary and subject to the commercial terms. No exceptions.
> 
> **Under no circumstances may the listed Pro features be enabled, replicated, or unlocked outside of the activation system.**
> **Any such attempt voids all rights under this license.**

## 5. Qt Licensing Compliance

You are solely responsible for compliance with the Qt framework's licensing.

### 5.1 Qt Usage Rules

- **GPLv3 builds of Serial Studio may only use the open-source Qt version** licensed under GPL-compatible terms.
- **Use of commercial Qt**, or linking against proprietary Qt modules, is only allowed if:
  - You hold a valid Qt commercial license from The Qt Company, **and**
  - You also comply with the **Serial Studio Commercial License**.

- Having a commercial Qt license **does not waive** or replace the requirement for a Serial Studio commercial license.

### 5.2 Exception for Authorized Maintainers

**Trusted maintainers** (e.g., Flatpak, Snapcraft, selected Linux distributions) are granted permission to build and distribute **commercial-feature-enabled binaries**, provided that:

- **No modifications** are made to source code, branding, or activation logic.
- Gated features are **not unlocked or bypassed**.
- The build is clearly marked as **proprietary**.
- The package includes a prominent link to the [Commercial License Agreement](https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE_COMMERCIAL.md).
- End users must still obtain a valid license and perform activation to access Pro functionality.

Unauthorized modification, feature unlocking, or removal of licensing mechanisms is a breach of this license and may result in legal action.

## 6. Contributor Licensing Terms

By submitting any contribution (code, documentation, translations, etc.) to this project, you:

- **Explicitly and irrevocably grant** the author (Alex Spataru) the right to license your contributions under:
  - GPLv3, for use in open-source distributions, **and**
  - The Serial Studio Commercial License, for use in proprietary binaries and gated feature builds

This dual-license grant is **perpetual, royalty-free, and non-exclusive**. If you do not accept these terms, **do not submit contributions** to this project.

## 7. Trademarks, Branding, and Forking Restrictions

"Serial Studio", along with its logo, name, iconography, and visual identity, are **trademarks of Alex Spataru**.

### Trademark Restrictions:

- You **may not** use the name "Serial Studio", or any part of its branding, in:
  - Forks or derivative products.
  - Rebranded or unofficial distributions.
  - Marketing, screenshots, or public descriptions of modified versions.

- All forks, clones, or third-party builds:
  - **Must be fully rebranded**, including the app name, icon, and all visual assets.
  - **Must not include any Pro or commercial features**, as listed in Section 4.
  - **Must not include, modify, or disable** the activation or license management systems.
  - **Must include a visible and functional link** to the original project repository:  
    [https://github.com/Serial-Studio/Serial-Studio](https://github.com/Serial-Studio/Serial-Studio)

- You **may not imply endorsement, affiliation, or official status** without prior written permission from the author.

### Forking for Study & Contributions:

- Forking the project for **personal learning, code exploration, or contribution via pull request** is welcome and encouraged.
- You may reference Pro-related code **only for identifying bugs or stability issues**. Contributions that help improve robustness or fix issues are accepted, as long as:
  - **You do not alter, remove, bypass, or disable** any activation or license enforcement logic.
  - **You do not enable or expose** Pro features to users who have not activated a valid license.

- **Do not distribute** modified versions under any name (even for free) unless you comply fully with all branding, licensing, and rebranding conditions in this license.

Violations of these terms may result in takedown requests, legal action, or immediate termination of license rights.

## 8. Authorized Redistribution by Trusted Maintainers

Trusted redistributors (e.g., Flatpak, Snapcraft) are permitted to package and distribute the proprietary build **only if** the following strict conditions are met:

- The build is **unmodified** (source, features, and branding).
- All license checks, activation logic, and enforcement code must remain fully intact and unmodified.
- Pro features remain gated and disabled unless activated by the end user.
- Redistribution is accompanied by:
  - A clear indication that the software is proprietary.
  - A direct link to the [Commercial License Agreement](https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE_COMMERCIAL.md).

Any deviation from these conditions constitutes unauthorized redistribution.

## 9. Warranty Disclaimer

To the maximum extent permitted by applicable law:

> THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.  
> IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES, OR LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM THE USE OF THIS SOFTWARE OR VIOLATION OF THIS LICENSE.

Commercial licensees may receive support as specified in their license agreement, but **no guarantees are implied unless contractually stated**.

## 10. Summary Table

| Use Case                                        | Allowed? | Conditions                                                                                                                                             |
|------------------------------------------------|----------|---------------------------------------------------------------------------------------------------------------------------------------------------------|
| Build from source (GPLv3)                      | ✅       | Must use GPL Qt, exclude gated features, comply with GPLv3                                                                                              |
| Build from source (with Pro features)          | ❌       | Requires **commercial license** and valid Qt commercial license if applicable                                                                          |
| Use official binaries for personal/eval use    | ✅       | Non-commercial only, gated features locked unless activated                                                                                             |
| Use official binaries in commercial setting    | ❌       | Requires commercial license and activation                                                                                                              |
| Fork for study or contribution (non-distributed)| ✅       | Allowed for personal study and bugfix contributions. Pro code may be referenced for fixes, but **activation, licensing, or feature unlocks must not be altered** |                                                      |
| Redistribute modified build                    | ❌       | Not allowed — forks must not be distributed under any name unless fully rebranded, stripped of Pro code, and comply with license + attribution rules   |
| Redistribute official build (e.g., Flatpak)    | ✅\*     | Must be unmodified, clearly proprietary, include license link — **only by authorized maintainers with explicit written permission**                    |

## 11. Tampering, Circumvention & Enforcement

### 11.1 Activation System Integrity

The activation and license management system is included **only** for purposes of transparency, security auditing, and community trust. **Any modification, removal, bypass, or circumvention of this system is strictly prohibited**, regardless of whether the resulting build is used personally, privately, or non-commercially.

- This includes any change to:
  - Activation checks
  - Feature unlock conditions
  - Hardcoded license responses
  - Signature or token validation logic
  - UI or CLI code related to activation status

**Attempting to enable Pro features through source code changes is a direct violation of this license.**

### 11.2 No GPL Exception

The activation and license management system is **explicitly excluded** from GPLv3 rights. You are **not permitted** to modify, remove, or bypass it under open-source terms.

This restriction also applies to all **gated commercial modules** listed in Section 4, such as 3D visualization, XY plotting, and MQTT integration:

- You **may modify or improve** the source code of Pro modules (e.g., fixing bugs, improving performance), but:
  - You **must not disable, bypass, or remove** the activation checks that control access to these modules.
  - You **must not expose** or unlock any Pro features to unlicensed users.

Any modification that results in unrestricted access to Pro functionality, reclassifies your usage as **unauthorized** and voids all rights under this license. Such actions may result in legal consequences, including takedowns or civil claims.

### 11.3 Enforcement and Legal Action

Bypassing license enforcement or enabling commercial features without valid activation:
- **Terminates all rights** under this license (both GPL and Commercial)
- May be considered **software piracy**
- May result in **DMCA takedowns**, cease-and-desist letters, and/or legal action

The author **actively monitors** forks, builds, and public repositories. Unauthorized copies may be removed or prosecuted without prior notice.

## 12. Contact

For license questions, edge cases, commercial inquiries, or to negotiate custom builds:
- GitHub: <https://github.com/Serial-Studio/Serial-Studio/issues>  
- Email: [alex@serial-studio.com](mailto:alex@serial-studio.com)
