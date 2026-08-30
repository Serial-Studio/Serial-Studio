<!-- SPDX-FileCopyrightText: 2020-2026 Alex Spataru <alex@serial-studio.com> -->
<!-- SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial -->

# Local patches to the vendored open62541

`open62541.c` and `open62541.h` are the single-file distribution published with the **v1.5.7**
release. They are upstream files and should stay that way, with the exceptions recorded here.

Re-apply these when bumping to a new release, then update the version in `CMakeLists.txt`.

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

## 2. SAN buffer sizing in the mbedTLS certificate writers (`open62541.c`)

Upstream sizes the scratch buffer for the subjectAltName extension as
`MBEDTLS_SAN_MAX_LEN * sandeep + sandeep` — a fixed 64 bytes per name — in two places:

- `mbedtls_x509write_crt_set_subject_alt_name()` (used by `UA_CreateCertificate`)
- `mbedtls_x509write_csrSetSubjectAltName()` (the CSR counterpart)

A DNS name longer than ~47 characters overflows that budget once the URI SAN is added, the
backward ASN.1 writer returns `MBEDTLS_ERR_ASN1_BUF_TOO_SMALL`, and certificate generation
fails with "Setting subject alternative name failed." GitHub's macOS bare-metal runners have
67-character hostnames (`sjc22-bm210-<uuid>-<mac>.local`), which is how this surfaced: every
secure-channel integration test failed on macOS CI while short-hostname machines passed.

The fix adds the actual name lengths to the allocation in both functions, keeping the
original per-name slack to cover the ASN.1 tag/length overhead:

```c
    buflen = MBEDTLS_SAN_MAX_LEN * sandeep + sandeep;
    for(const mbedtls_write_san_list *it = sanlist; it != NULL; it = it->next)
        buflen += it->node.hostlen;
```

(and the same loop over `it->buf.len` for the `mbedtls_x509_sequence` CSR variant).

`UA_MULTITHREADING` is deliberately left at the value upstream generated (100). Serial Studio
drives the client from one thread and does not need the mutex layer, but the amalgamation's `.c`
was generated against that setting, and lowering it in the header alone would leave the two
inconsistent. The lock layer costs nothing at OPC UA command rates.
