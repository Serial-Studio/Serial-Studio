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
> - **Do not enable or distribute** gated Pro modules without a valid license.  
> - **Transparency of source code does not imply usage rights**. Pro modules remain proprietary, regardless of visibility.

All source files include SPDX identifiers to clearly distinguish GPL-covered components from proprietary ones. This ensures transparency for users and compliance automation tools.

If your build includes any gated features, it cannot be considered GPL-compliant and must instead be used under the terms of the Commercial License.

## 2. GPLv3 Licensing Terms (Source Code Use Only)

You may use, modify, and redistribute the source code under the [GNU GPLv3](https://www.gnu.org/licenses/gpl-3.0.html), but **only** if **all** the following conditions are strictly met:

### 2.1 Requirements

- You **must compile** Serial Studio from source.
- You **must use a fully GPL-compliant version of Qt**, such as the [open-source Qt distribution](https://www.qt.io/download-open-source).
- You **must not enable, compile, or link** any of the gated commercial features listed in Section 4.
- You **must not** link to any proprietary or commercial-only Qt modules or third-party dependencies.
- You **must preserve** all original license notices and comply with all GPLv3 obligations, including source code availability.

> [!NOTE]
> **Serial Studio is intentionally designed to protect contributors from accidental license violations**.
> 
> By default, it builds a fully **GPLv3-compliant** version with all proprietary features disabled.
> 
> Compiling a version with **Pro modules** requires a **valid commercial license**, **explicit configuration**,  
> and successful **network-based license validation**.
> 
> If a build includes gated features **without a license**, the build system was modified in a way that
> bypasses license enforcement, which constitutes a violation of this license's terms regarding
> proprietary components.

### 2.2 Third-Party Package Distributions

- Builds distributed via package managers (e.g., AUR, Ubuntu, Fedora) **must comply** with all of the above conditions.
- The project maintainer **does not verify or certify** GPL compliance of third-party builds. Users assume all risk when using such builds.

## 3. Commercial License Terms (Pro Modules & Official Binaries)

The [**Serial Studio Commercial License**](LICENSE_COMMERCIAL.md) applies to:

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

## 4. Proprietary Features (Pro Modules)

The following features are **explicitly excluded** from GPLv3 and are **governed exclusively** by the Commercial License:

- Linking against or including the **Qt MQTT** module.
- Linking against or including the **Qt Serial Bus** module.
- MQTT integration (broker/client support).
- XY plotting capabilities.
- 3D visualization tools.
- Activation or license management systems.

> [!NOTE]
> Compiling or linking any of the above features, even in a self-built version, results in a build that is not covered by the
> GPLv3 and must instead be licensed under the Commercial License.
> 
> **Under no circumstances may the listed Pro modules be enabled, replicated, or unlocked outside of the activation system.**  
> **Any such attempt terminates your rights under the Serial Studio Commercial License. GPLv3-covered components remain governed by the GPL.**

## 5. Qt Licensing Compliance

Serial Studio is designed to comply with the licensing requirements of the Qt framework. Use of any version of Qt (open-source or commercial) is allowed only if all other licensing conditions in this document are fully respected.

### 5.1 GPLv3 Builds

You may compile and use Serial Studio under GPLv3 using any version of Qt, provided that:

- You do **not** link against any Qt modules that are only available under commercial terms.
- You comply with all obligations of the Qt open-source license and the GNU GPLv3.
- You **do not include, enable, or access** any of the proprietary Pro modules listed in Section 4.

### 5.2 Commercial/Pro Builds

Any build of Serial Studio that includes gated Pro modules (even if the features are not activated by default) **requires a valid Qt commercial license**.

- This requirement is separate from and in addition to the Serial Studio Commercial License.
- Gated Pro modules may depend on Qt libraries that are only available under Qt’s commercial terms.

**Clarification**: Compiling a build that includes Pro functionality (regardless of activation state) is only legal if you hold both a valid Qt commercial license **and** a Serial Studio commercial license.

You are solely responsible for ensuring your Qt license covers your intended use of Serial Studio.

## 6. Contributor Licensing Terms

All contributors must agree to the Contributor License Agreement (CLA) before their code can be merged.

By signing the CLA and submitting a contribution (code, documentation, translations, etc.) to this project, you:

- **Explicitly and irrevocably grant** the author (Alex Spataru) the right to license your contributions under:
  - The GNU General Public License v3 (GPLv3), for use in open-source distributions, **and**
  - The Serial Studio Commercial License, for use in proprietary binaries and gated feature builds

This dual-license grant is **perpetual, royalty-free, irrevocable, and non-exclusive**.

This license **does not transfer ownership** of your contributions. It only grants the author the right to license them under both the GPLv3 and the Commercial License.

> You will be prompted to sign the CLA automatically when opening a pull request.  
> If you do not agree to the terms of the CLA, do not submit contributions.

## 7. Trademarks, Branding, and Forking Restrictions

“Serial Studio” is a trademark of Alex Spataru. Whether registered or unregistered, its use is protected under applicable trademark law.

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

The use of “Serial Studio” or derivative branding in a way that suggests endorsement, affiliation, or origin is strictly prohibited without express written permission.

### Forking for Study & Contributions:

- Forking the project for **personal learning, code exploration, or contribution via pull request** is welcome and encouraged.
- You may reference Pro-related code **only for identifying bugs or stability issues**. Contributions that help improve robustness or fix issues are accepted, as long as:
  - **You do not alter, remove, bypass, or disable** any activation or license enforcement logic.
  - **You do not enable or expose** Pro modules to users who have not activated a valid license.

- **Do not distribute** modified versions under any name (even for free) unless you comply fully with all branding, licensing, and rebranding conditions in this license.

Violations of these terms may result in takedown requests, legal action, or immediate termination of license rights.

## 8. Warranty Disclaimer

To the maximum extent permitted by applicable law:

> THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.  
> IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES, OR LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM THE USE OF THIS SOFTWARE OR VIOLATION OF THIS LICENSE.

Commercial licensees may receive support as specified in their license agreement, but **no guarantees are implied unless contractually stated**.

## 9. Summary Table

| Use Case                                        | Allowed? | Conditions                                                                                                                                             |
|------------------------------------------------|----------|---------------------------------------------------------------------------------------------------------------------------------------------------------|
| Build from source (GPLv3)                      | ✅       | Must use GPL Qt, exclude gated features, comply with GPLv3                                                                                              | 
| Build from source (with Pro modules)           | ❌       | Requires **commercial license** and valid Qt commercial license if applicable                                                                          |
| Use official binaries for personal/eval use    | ✅       | Non-commercial only, gated features locked unless activated                                                                                             |
| Use official binaries in commercial setting    | ❌       | Requires commercial license and activation                                                                                                              |
| Fork for study or contribution (non-distributed)| ✅       | Allowed for personal study and bugfix contributions. Pro code may be referenced for fixes, but **activation, licensing, or feature unlocks must not be altered** |                                                      |
| Redistribute modified build | ✅ (*) | Only allowed if fully rebranded, excludes all Pro modules, complies with GPLv3 terms, and includes attribution with link to the original project

## 10. Tampering, Circumvention & Enforcement

### 10.1 Activation System Integrity

The activation and license management system is included **only** for purposes of transparency, security auditing, and community trust. Any attempt to modify, remove, or bypass activation mechanisms within proprietary modules is strictly prohibited, regardless of intent or distribution.

- This includes any change to:
  - Activation checks
  - Feature unlock conditions
  - Hardcoded license responses
  - Signature or token validation logic
  - UI or CLI code related to activation status

**Attempting to enable Pro modules through source code changes is a direct violation of this license.**

### 10.2 Proprietary Module Restrictions

The activation and license management system is part of the proprietary components explicitly excluded from GPLv3 coverage. It is **not compiled or included** in standard GPLv3 builds.

You may not modify, remove, or bypass these systems in any build that includes proprietary modules. Doing so violates the terms of the Serial Studio Commercial License.

This restriction also applies to all gated commercial modules listed in Section 4 (e.g., 3D visualization, XY plotting, MQTT integration):

- You **may improve or patch** the source code of these proprietary modules (e.g., for bug fixes), but:
  - You **must not disable, bypass, or remove** the activation checks that restrict access to these modules.
  - You **must not expose or unlock** Pro modules to unlicensed users under any circumstances.

Modifying the source in a way that enables unrestricted access to commercial features without a valid license constitutes **unauthorized use** and may result in termination of your license rights, DMCA action, or legal enforcement. Only users with an active, valid commercial license may compile and use these proprietary modules. Redistribution is not permitted unless explicitly authorized by the author.

### 10.3 Enforcement and Legal Action

Bypassing license enforcement or enabling commercial features without valid activation:
- **Terminates all rights** under the Serial Studio Commercial License. GPLv3 rights remain governed by the GPL.
- May violate applicable copyright laws.
- May result in **DMCA takedowns**, cease-and-desist letters, and/or legal action

Disabling or removing license enforcement in order to access or retain Pro modules without a valid commercial license is a violation of the Commercial License and does not place those modules under GPLv3 or any open-source license. Visibility of source code does not grant usage rights.

The author **actively monitors** forks, builds, and public repositories. Unauthorized copies may be removed or prosecuted without prior notice.

This license is governed by and construed in accordance with the laws of Mexico, without regard to its conflict of law provisions.
If any provision of this license is found unenforceable, the remaining terms shall remain in full effect.

### 10.4 Build-Time Enforcement

Serial Studio’s build system is intentionally designed to prevent accidental license violations:

- By default, all builds are GPLv3-compliant and exclude all proprietary code.
- Pro modules are only compiled if:
  - A valid commercial license key and instance ID are provided via environment variables, and  
  - The build system explicitly detects and authorizes those credentials at configuration time.

Compiling any build that includes Pro modules **without an active, valid commercial license** is explicitly forbidden. Doing so constitutes deliberate circumvention of license enforcement and a direct violation of this license.

If a user successfully compiles a Pro-enabled build without proper authorization, **all rights under the Serial Studio Commercial License are immediately terminated**. GPLv3-covered components remain governed by the GPL.

This safeguard exists to:
- Prevent accidental non-compliance.
- Clearly distinguish between authorized users and those bypassing protections.

**Unauthorized Pro builds may not be compiled, used, distributed, or retained in any form. Possession does not confer any rights. No part of the proprietary codebase may be compiled, linked, or activated without a valid commercial license.**

## 11. Contact

For license questions, edge cases, commercial inquiries, or to negotiate custom builds:
- GitHub: <https://github.com/Serial-Studio/Serial-Studio/issues>  
- Email: [alex@serial-studio.com](mailto:alex@serial-studio.com)
