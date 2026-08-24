<!-- SPDX-FileCopyrightText: 2020-2026 Alex Spataru <alex@serial-studio.com> -->
<!-- SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial -->

# Local patches to the vendored open62541

`open62541.c` and `open62541.h` are the single-file distribution published with the **v1.5.7**
release. They are upstream files and should stay that way, with the one exception recorded here.

Re-apply this when bumping to a new release, then update the version in `CMakeLists.txt`.

## 1. Architecture detection (`open62541.h`, near line 49)

Upstream generates the published amalgamation on Linux, so the header ships with

```c
/* #undef UA_ARCHITECTURE_WIN32 */
#define UA_ARCHITECTURE_POSIX
```

hardcoded. The auto-detection block immediately below it only runs when *no* architecture is
already defined, so on Windows the POSIX branch stays selected and the build dies at

```
open62541.h(489): fatal error C1083: Cannot open include file: 'pthread.h'
```

The fix leaves the macro undefined so the existing detection picks `UA_ARCHITECTURE_WIN32` on
Windows and `UA_ARCHITECTURE_POSIX` everywhere else:

```c
/* #undef UA_ARCHITECTURE_WIN32 */
/* #undef UA_ARCHITECTURE_POSIX */
```

Nothing else is touched. Every other POSIX-only include in the amalgamation is already guarded by
`UA_ARCHITECTURE_POSIX`, `__linux__` or `UA_HAVE_EPOLL`, so this single change is enough to make
the distribution portable.

`UA_MULTITHREADING` is deliberately left at the value upstream generated (100). Serial Studio
drives the client from one thread and does not need the mutex layer, but the amalgamation's `.c`
was generated against that setting, and lowering it in the header alone would leave the two
inconsistent. The lock layer costs nothing at OPC UA command rates.
