#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# Imports the release GPG identity into the runner keychain and arms rpmbuild for it.
# Signing is opt-in on the secrets being present: without them the script exits clean,
# SS_GPG_SIGN is never exported, and the packaging steps ship unsigned artifacts.
#
# Driven by .github/actions/package-linux. CWD-independent.
#
# Environment:
#   GPG_PRIVATE_KEY  armored private key (optional)
#   GPG_PASSPHRASE   passphrase for that key (optional)
#   GPG_KEY_ID       key id the packages are signed with (optional)
#   RUNNER_TEMP      runner scratch directory the passphrase file lives in
#   GITHUB_ENV       step-env file SS_GPG_SIGN=1 is written to
#
set -euo pipefail
if [ -z "${GPG_KEY_ID:-}" ] || [ -z "${GPG_PRIVATE_KEY:-}" ]; then
  echo "GPG signing secrets not set; packages will be unsigned."
  exit 0
fi
mkdir -p ~/.gnupg && chmod 700 ~/.gnupg
echo "allow-preset-passphrase" >> ~/.gnupg/gpg-agent.conf
printf '%s\n' "$GPG_PRIVATE_KEY" | gpg --batch --import
gpg-connect-agent reloadagent /bye
if [ -n "${GPG_PASSPHRASE:-}" ]; then
  for grip in $(gpg --batch --with-colons --with-keygrip \
      --list-secret-keys "$GPG_KEY_ID" | awk -F: '$1=="grp"{print $10}'); do
    printf '%s' "$GPG_PASSPHRASE" | \
      /usr/lib/gnupg/gpg-preset-passphrase --preset "$grip"
  done
fi
PASSFILE="$RUNNER_TEMP/gpg-pass"
touch "$PASSFILE" && chmod 600 "$PASSFILE"
printf '%s' "${GPG_PASSPHRASE:-}" > "$PASSFILE"
printf '%%_signature gpg\n' > ~/.rpmmacros
printf '%%_gpg_name %s\n' "$GPG_KEY_ID" >> ~/.rpmmacros
printf '%%_gpg_sign_cmd_extra_args --batch --pinentry-mode loopback --passphrase-file %s\n' \
  "$PASSFILE" >> ~/.rpmmacros
echo "SS_GPG_SIGN=1" >> "$GITHUB_ENV"
