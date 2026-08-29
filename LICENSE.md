# Serial Studio Licensing

Copyright © 2020–2026 Alex Spataru <alex@serial-studio.com>

Serial Studio is dual-licensed. The license of every file in this repository is stated
in its SPDX header, or in [REUSE.toml](REUSE.toml) for files without one. **The SPDX
declaration is authoritative.** This document only routes you to the right instrument;
it adds no terms of its own.

## Dual-licensed files

Files marked `GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial` are offered under
either license, at your option:

- **GNU General Public License v3.0 or later** — full text in
  [LICENSES/GPL-3.0-or-later.txt](LICENSES/GPL-3.0-or-later.txt). A build that contains
  only these files (the default `BUILD_GPL3=ON` build) is GPLv3 software. You receive
  every right the GPL grants — use for any purpose, including commercial use; private
  and distributed modification; and redistribution in source or binary form — subject
  only to the GPL's own conditions. No additional terms apply to GPL builds.
- **The Serial Studio Commercial License** — full text in
  [LICENSES/LicenseRef-SerialStudio-Commercial.txt](LICENSES/LicenseRef-SerialStudio-Commercial.txt).
  This is the license under which builds that include Pro modules are made and used.

## Proprietary files (Pro modules)

Files marked `LicenseRef-SerialStudio-Commercial` **only** are proprietary. A file is a
Pro module if and only if its SPDX expression is `LicenseRef-SerialStudio-Commercial`
alone. Their source code is published for transparency, audit, and contribution; no GPL
rights attach to it, and compiling, using, or distributing it requires a valid
commercial license. The default build excludes every one of these files.

## Official binaries

The precompiled binaries published on serial-studio.com and in GitHub releases contain
Pro modules and are conveyed solely under the
[Serial Studio End User License Agreement](EULA.md), which includes a 14-day trial per
release. They are not GPL builds.

## Related documents

- Trademarks (name, logo, branding): [TRADEMARKS.md](TRADEMARKS.md)
- Contributions and the Contributor License Agreement: [CONTRIBUTING.md](CONTRIBUTING.md)
- Third-party components: [REUSE.toml](REUSE.toml) and [LICENSES/](LICENSES/)

## Contact

Licensing questions and commercial inquiries: alex@serial-studio.com
