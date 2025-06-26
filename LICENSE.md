# Serial Studio License Agreement

**Copyright © 2020–2025**  
**Alex Spataru** (<https://github.com/alex-spataru>)  
**All rights reserved where applicable.**

Serial Studio is the result of years of work. It’s free to explore, build, and contribute to, but not free to exploit. To sustain development and protect the integrity of the software, this project uses a **dual-license model** with a **strict 14-day trial**:

- You may use the official **Pro binary** for **free** during a **single-use, 14-day evaluation period**.
- After the trial, a **paid commercial license** is required for continued use of the official binary.
- You may compile the **GPLv3-licensed source code** yourself, provided you fully comply with strict open-source limitations.

The following terms define your rights and obligations when using, modifying, building, or distributing Serial Studio.

## 1. Dual Licensing Overview

Serial Studio is offered under two distinct licenses:

- The **GNU General Public License v3 (GPLv3)** applies **only** to the open-source components explicitly marked as such and **excludes** any proprietary or commercial features listed in Section 4.
- The **Serial Studio Commercial License** applies to:
  - All **official precompiled binaries**.
  - All builds (official or third-party) that include any **gated commercial features**.
  - **Post-trial usage** of the official binary.

### 1.1 Trial Period

You may use the official Pro version of Serial Studio for **free** during a **one-time, 14-day trial**. During this period:

- All features are fully unlocked.
- Use is limited to **evaluation purposes only**.
- Trial use is limited to **one machine and one user**. You may **not reset, extend, or circumvent** the trial using VMs, OS reinstalls, system time changes, or any other means.

After 14 days, you must either:

- Purchase a valid **commercial license**, or
- **Compile** the GPLv3 version yourself (see Section 2).

The official binary will **automatically disable** after the trial ends unless a valid license is activated.

For full commercial terms, see [LicenseRef-SerialStudio-Commercial](LICENSES/LicenseRef-SerialStudio-Commercial.txt).  
For open-source GPL terms, see [GPL-3.0-only](LICENSES/GPL-3.0-only.txt).

## 2. GPLv3 Licensing Terms (Source Code Only)

You may use, modify, and redistribute the source code under the [GNU GPLv3](LICENSES/GPL-3.0-only.txt), **only if** you meet all the following conditions:

### 2.1 GPL Compliance Requirements

- You **must compile** Serial Studio from source.
- You **must use a GPL-compliant version of Qt** (e.g. [Qt open-source edition](https://www.qt.io/download-open-source)).
- You **must not** enable, link, replicate, simulate, or include **any** Pro modules listed in Section 4.
- You **must not link against** proprietary Qt modules or commercial-only third-party dependencies.
- You **must preserve all license notices** and comply with all GPLv3 obligations, including source availability.

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

> [!WARNING]
> Compiling any version that includes Pro features results in a **non-GPL-covered build** and requires a commercial license.

### 2.3 Official Binary Distribution vs. GPL Builds

Distributions via third-party package managers (e.g., Homebrew, Winget, Chocolatey) that download **official precompiled binaries** from GitHub or serial-studio.com **are not covered by the GPLv3**, even if hosted in repositories labeled as open source.

These binaries are licensed exclusively under the **Serial Studio Commercial License**, and:

- Are provided **solely** for personal evaluation use during the 14-day trial.
- **May not be used commercially** without a valid license and activation.
- **May not be redistributed, modified, or repackaged** under the GPL or any other license.
- Must not be presented or implied as “free” or “open-source” software.

Maintainers of package manager formulas, manifests, or recipes **must not misrepresent** the license of the official binaries or suggest that they are GPL-licensed.

> If your manifest or formula downloads the official binary, you are distributing **proprietary trial software**, not open-source software.

However, third-party maintainers **are permitted to distribute GPLv3-compliant builds** of Serial Studio **only if**:

- The build is compiled **entirely from source**, with **no Pro modules enabled** or linked.
- The build complies strictly with Section 2.1 of this license.
- The Qt version used is **fully GPL-compatible**.
- All original license notices are preserved.
- The package is clearly labeled as a **community-built GPL version** with no affiliation to or support from the official project.

## 3. Commercial License Terms

The [Serial Studio Commercial License](LICENSES/LicenseRef-SerialStudio-Commercial.txt) governs:

- All **official binaries** published on serial-studio.com or GitHub.
- Any build that includes or links to **Pro modules**.
- All usage after the 14-day trial ends.

### 3.1 Key Terms

- Any use of Pro builds in **commercial, institutional, or enterprise** contexts **requires** a valid commercial license.
- Builds containing Pro modules are **unauthorized** without license activation.
- Tampering with or bypassing activation, license checks, or trial logic is **strictly prohibited**.

### 3.2 “Commercial Use” Defined

“Commercial use” includes, but is not limited to:

- Use by for-profit entities or contractors.
- Use in revenue-generating workflows or systems.
- Use in production, client-facing, or enterprise environments.
- Use in research funded by grants or private capital.

If you’re unsure, assume you need a commercial license.

## 4. Proprietary Features (Pro Modules)

The following features are **excluded from the GPL license** and available **only under the Serial Studio Commercial License**:

- Qt MQTT and Qt SerialBus modules
- MQTT support (broker/client)
- XY plotting
- 3D visualization
- Activation/licensing systems

These modules are defined as **separate works** and not covered by GPLv3. Their source code, even if visible, does not confer any right to use, modify, compile, or distribute them outside a valid commercial license.

> [!WARNING]
> Under no circumstances may Pro features be enabled, or redistributed outside official builds.

## 5. Qt Licensing Compliance

### 5.1 GPL Builds

You may build the GPL version **only if**:

- No commercial-only Qt modules are linked.
- GPLv3 terms and Qt’s open-source license are fully followed.
- No Pro modules are enabled, stubbed, or compiled.

### 5.2 Pro Builds

All builds containing or depending on Pro features:

- Require a **valid Qt commercial license** (separate from the Serial Studio license).
- May only be created and used with both **Serial Studio** and **Qt** commercial licenses.

You are solely responsible for Qt license compliance.

## 6. Contributor License Agreement (CLA)

Contributors must accept the CLA to contribute. By submitting code or content, you:

- **Irrevocably grant** the project owner the right to license your contributions under both the **GPLv3** and the **Serial Studio Commercial License**.
- Affirm you **own or have rights** to all contributed content.
- Understand and agree that your contribution may be used in both open-source and commercial versions of the software.

No pull request may be merged without a signed CLA.

## 7. Trademark & Forking Policy

"Serial Studio" is a protected trademark. Use of the name, logo, or branding is **not permitted** in:

- Forks or derivative works.
- Modified or unofficial builds.
- Public screenshots or marketing of altered versions.

### 7.1 Forking Guidelines

You may fork for study or contribution **only if**:

- You rebrand the application entirely.
- You **exclude** all Pro features.
- You **do not** disable, modify, or bypass activation systems.
- You **link visibly** to the official repo.

## 8. Summary Table

| Use Case                              | Allowed? | Conditions                                                                                  |
|---------------------------------------|----------|---------------------------------------------------------------------------------------------|
| GPL build from source                 | ✅       | GPL Qt only, no Pro features, full GPLv3 compliance.                                       |
| GPL build with Pro modules            | ❌       | Requires full commercial license. Not permitted under GPLv3.                               |
| Use official binary (14-day trial)    | ✅       | Evaluation only, one-time per user/machine. No commercial use.                             |
| Use official binary after trial       | ❌       | Requires license activation.                                                               |
| Commercial use of any kind            | ❌       | Requires commercial license regardless of trial state.                                     |
| Fork for study (non-distributed)      | ✅       | Must not alter license logic. No Pro features. Must be rebranded.                          |
| Distribute modified GPL version       | ✅*      | Must be fully rebranded, exclude Pro modules, and comply fully with GPLv3.                 |

> *Unauthorized Pro feature inclusion, even by accident, voids GPL eligibility. The build system is designed to prevent accidents.*

## 9. Activation, Tampering & Enforcement

### 9.1 Trial System Protection

You may not:

- Reset or reinitiate trial periods.
- Modify license or activation code.
- Falsify activation tokens or server responses.

### 9.2 Unauthorized Builds

- You may not compile or distribute any version that enables or simulates Pro functionality without a valid license.
- Even for personal use, bypassing activation constitutes **unauthorized use**.

### 9.3 Enforcement

Violations may result in:

- Termination of all rights under the commercial license.
- DMCA takedown notices.
- Legal action under applicable copyright law.
- Permanent blacklisting of license keys and domains.

## 10. Build-Time Compliance Enforcement

- Pro modules only compile if a valid commercial license and instance ID are provided.
- Compiling or running unauthorized Pro builds immediately **terminates license rights**.
- No “educational” or “non-commercial” exception applies to Pro functionality.

**Visibility of source code does not grant usage rights. Possession ≠ permission.**

## 11. Governing Law

This license is governed by the laws of **Mexico**, and any legal action must be filed in the courts of **Mexico City**. If any provision is found unenforceable, the rest remains in effect.

## 12. DMCA Agent

To report license violations or respond to a takedown:

- **Agent**: Alex Spataru  
- **Email**: alex@serial-studio.com

This project complies with 17 U.S.C. § 512 and issues takedowns for:

- Unauthorized builds or forks with Pro modules
- Trademark-infringing distributions
- Bypass of activation systems

False claims will be prosecuted under penalty of perjury.

## 13. Contact

For licensing questions or commercial inquiries:

- GitHub: <https://github.com/Serial-Studio/Serial-Studio/issues>  
- Email: [alex@serial-studio.com](mailto:alex@serial-studio.com)
