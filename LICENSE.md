# Serial Studio License Agreement

**Copyright © 2020–2025**  
**Alex Spataru** (<https://github.com/alex-spataru>)  
**All rights reserved where applicable.**

Serial Studio is the result of years of work. It’s free to explore, build, and contribute to, but not free to exploit. To keep things sustainable and fair for everyone, this project uses a **dual-license model** that separates open contributions from commercial use.

You’re welcome to study the source, build it, and even fork it, as long as you stay within the rules. Those rules exist to protect both the community and the author. If you’re unsure about anything, just [get in touch](mailto:alex@serial-studio.com). If you know what you’re doing and bypass the rules anyway, you’re on your own.

## 1. Dual Licensing Overview

Serial Studio is provided under two licensing models:

- The **GNU General Public License v3 (GPLv3)** applies to the open-source portions of the source code, excluding the gated commercial features listed in Section 4.
- The **Serial Studio Commercial License** governs:
  - All **official precompiled binaries**
  - Any builds (by anyone) that include gated Pro features, regardless of where or how they were compiled

> [!IMPORTANT]
> **Source visibility ≠ source permission.**  
> Some parts of the codebase are visible but off-limits. Just because you can see them doesn’t mean you can use, enable, or modify them.

If you build from source and **exclude all Pro features**, you’re in GPLv3 territory.  
If you enable or link anything from Section 4 (intentionally or not) you’ve entered Commercial License territory. No exceptions.

## 2. GPLv3 Licensing Terms (Source Code Use Only)

The GPLv3 terms apply **only if all of the following are true**:

- You compile Serial Studio from source.
- You use only a **GPL-compatible version of Qt**.
- You **do not enable, include, or modify** any of the Pro features listed in Section 4.
- You don’t link to any proprietary Qt modules or third-party commercial dependencies.
- You comply with all standard GPLv3 obligations (license notices, source distribution, etc.).

You can use the `-DBUILD_GPL3=ON` CMake flag to automatically produce a GPL build.
Distributing GPL builds via Linux packaging systems (AUR, Ubuntu, etc.) is allowed if they follow all of the above. Third-party maintainers are responsible for their own compliance.

## 3. Commercial License Terms (Pro Features & Official Binaries)

The [**Serial Studio Commercial License**](LICENSE_COMMERCIAL.md) governs:

- Official precompiled binaries (from [serial-studio.com](https://serial-studio.com/), GitHub, etc.)
- Any build, by anyone, that includes or unlocks any Pro features

### 3.1 What Counts as Commercial Use?

"Commercial use" includes, but isn’t limited to:

- Use in companies or paid projects.
- Inclusion in paid tools, services, or products.
- Internal tools in for-profit entities.
- Any use funded by grants, customers, or commercial research.

If you’re not sure whether your usage counts: assume it does. Or ask.

### 3.2 License Requirements

- A **valid commercial license** is required for all commercial use.
- Activation is mandatory for accessing Pro functionality.
- **Tampering with activation logic** is a direct license violation and will be enforced.

## 4. Gated Pro Features (Commercial Only)

These features are **not GPL-licensed**, and **not allowed** under any free use of the software:

- MQTT support and Qt MQTT integration.
- Qt Serial Bus support.
- XY plotting tools (custom X-axis selection).
- 3D visualization widget.
- Activation, licensing, and related infrastructure.

> [!WARNING]
> If your build includes any of the above, it’s considered **proprietary** and subject to commercial terms.  
> **You are not allowed to enable, replicate, modify, or unlock** any of these Pro features unless you’ve activated a valid commercial license.

Any attempt to bypass or modify the activation system, expose gated features, or distribute modified versions with Pro functionality is treated as a **deliberate license violation**.

## 5. Qt Licensing Compliance

You are fully responsible for complying with the Qt framework’s licensing:

- GPLv3 builds must use Qt versions licensed under **GPL-compatible terms**.
- Commercial Qt modules are only allowed if you have both:.
  - A valid Qt commercial license.
  - A valid Serial Studio commercial license.

Having a Qt commercial license does **not** grant permission to use Pro features in Serial Studio.

## 6. Contributor Licensing Terms

Contributions are welcome (and appreciated) under clear terms:

By submitting a contribution, you agree to let the author license your code under:
- GPLv3, for use in open-source builds
- The Commercial License, for use in proprietary and Pro builds

This grant is perpetual, royalty-free, and non-exclusive. If you’re not okay with that, don’t submit code.

## 7. Branding, Trademarks, and Forking

“Serial Studio” is a trademark of Alex Spataru. If you fork or build your own version:

- You **must fully rebrand** the name, icons, and visual identity.
- You **must not include or enable** Pro features.
- You **must not remove, bypass, or modify** license enforcement logic.
- You **must link clearly** to the original project repository.

You’re welcome to fork for personal use, study, or contribution. You’re not welcome to fork, add features, keep the name, and call it your own.

## 8. Redistribution by Authorized Maintainers

Only approved maintainers (e.g., Flatpak/Snapcraft packagers) may distribute proprietary builds, under strict conditions:

- The source, branding, and activation logic must be unmodified.
- Pro features must remain locked unless a user activates them.
- The package must clearly state that it's proprietary.
- It must link to the [Commercial License Agreement](https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE_COMMERCIAL.md).

Any unauthorized redistribution or modification of Pro-enabled builds is a violation.

## 9. No Tolerance for Bypass or Circumvention

The activation and licensing system exists to keep things fair. It is **not optional**.

If you:
- Modify or disable activation logic
- Fake a license
- Unlock Pro features without permission

...you’ve terminated all your rights under this license, whether GPL or commercial. That’s considered piracy, and will be treated accordingly.

The same goes for any fork or distribution that:
- Circumvents license checks.
- Exposes gated features.
- Replicates Pro modules.

No grey areas. If you know what you’re doing, you know this isn’t allowed.

## 10. Warranty Disclaimer

> This software is provided **“as is”**, without warranty of any kind.  
> The author is not responsible for damages or liability arising from its use.

If you’re a commercial license holder, support is available as agreed in your license terms.

## 11. Summary Table

| Use Case                                        | Allowed? | Conditions                                                                                                                                             |
|------------------------------------------------|----------|---------------------------------------------------------------------------------------------------------------------------------------------------------|
| Build from source (GPLv3)                      | ✅       | Must exclude Pro features, use open Qt, follow GPLv3                                                                                                     |
| Build from source (with Pro features)          | ❌       | Requires a commercial license                                                                                                                           |
| Use official binaries for personal/eval use    | ✅       | Allowed if non-commercial. Pro features remain locked                                                                                                   |
| Use official binaries in commercial setting    | ❌       | Requires a commercial license and activation                                                                                                            |
| Fork for study or contribution (non-distributed)| ✅       | Allowed for private use or bugfix PRs, but no Pro feature enabling or tampering                                                                            |
| Redistribute modified build                    | ❌       | Not allowed unless fully rebranded, GPLv3 licensed, and compliant                                                                                             |
| Redistribute official build (Flatpak, etc.)    | ✅*      | Only by trusted maintainers, no modifications, must link to commercial license                                                                         |

## 12. Contact

For commercial inquiries, license questions, edge cases, or custom arrangements:

- GitHub: <https://github.com/Serial-Studio/Serial-Studio/issues>  
- Email: [alex@serial-studio.com](mailto:alex@serial-studio.com)

---

Thanks for respecting the work that went into this project. This license exists to keep that work sustainable, not to scare you, but to keep things fair.

If you want to use Serial Studio the right way, you're welcome here. If you’re looking to bypass the rules, don’t.
