#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# gRPC arrives as a prebuilt tarball from GRPC_REPO; building it from source would cost more
# than the rest of the job put together.
#
# Retried: the release-asset endpoint answers HTTP 500 often enough to redden a run on its own
# (2026-09-03, Linux arm64), and a single failed GET here costs the whole job. --clobber so a
# partial download from the previous attempt does not make the retry fail on an existing file.
#
# Driven by .github/actions/download-grpc. CWD-independent.
#
# Environment:
#   SS_WORKSPACE   directory the asset is downloaded into
#   GRPC_VERSION   release tag (without the leading 'v')
#   GRPC_REPO      repository the release lives in
#   GRPC_ASSET     asset file name for this platform
#   GRPC_PREFIX    directory the tarball is extracted into
#   GH_TOKEN       token the gh CLI authenticates with
#
set -euo pipefail
cd "${SS_WORKSPACE}"
for attempt in 1 2 3 4 5; do
  if gh release download "v${GRPC_VERSION}" \
       -R "${GRPC_REPO}" \
       -p "${GRPC_ASSET}" \
       --clobber; then
    break
  fi
  if [ "$attempt" -eq 5 ]; then
    echo "::error::gh release download failed after $attempt attempts"
    exit 1
  fi
  sleep $((attempt * 15))
done
mkdir -p "${GRPC_PREFIX}"
tar -xzf "${GRPC_ASSET}" -C "${GRPC_PREFIX}"
