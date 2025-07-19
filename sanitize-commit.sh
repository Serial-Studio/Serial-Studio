#!/bin/bash

#
# sanitize-commit.sh — Clean up and prepare Git commits
#
# This script:
# - Normalizes file permissions on tracked files
# - Runs clang-format on source files
# - Shows a list of changed files
# - Enforces Conventional Commit messages
# - Optionally commits and pushes changes
#
# Intended to be run from the root of the repository.
#
# License: GNU General Public License v3.0
# https://www.gnu.org/licenses/gpl-3.0.html
#
# Author: Alex Spataru <https://github.com/alex-spataru>
#
# Usage:
#   ./sanitize-commit.sh

# Fail fast on errors, undefined vars, or broken pipelines
set -euo pipefail

# Go to the root of the git repo
REPO_ROOT=$(git rev-parse --show-toplevel)
cd "$REPO_ROOT" || exit 1

# Create a temp file with the list of tracked files
tempfile=$(mktemp)
git ls-files -z > "$tempfile"

# Get the name of this script so we can exclude it from chmod operation
SCRIPT_NAME="$(basename "$0")"

# Set standard file permissions for everything except this script
echo "Sanitizing file permissions..."
while IFS= read -r -d '' file; do
    if [[ "$(basename "$file")" == "$(basename "$0")" ]]; then
        continue
    fi

    [[ -f "$file" ]] && chmod 644 "$file"
    [[ -d "$file" ]] && chmod 755 "$file"
done < "$tempfile"

# Clean up the temporary file after use
rm -f "$tempfile"

# Format C/C++ files under known directories
echo "Running clang-format..."
for dir in app doc examples; do
    if [[ -d "$dir" ]]; then
        find "$dir" -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.c' \) \
            ! -name 'miniaudio.h' \
            -print0 | xargs -0 clang-format -i || echo "clang-format failed in $dir"
    fi
done

# Get a list of changed files (unstaged + staged)
echo "Checking for changes..."
CHANGED=$(git status --short)

# If nothing changed, we're done
if [[ -z "$CHANGED" ]]; then
    echo "No changes detected."
    exit 0
fi

# Show the user what changed
echo
echo "Changed files:"
git status --short
echo

# Count how many files changed (staged if any, otherwise unstaged)
COUNT=$(git diff --cached --name-only | wc -l)
[[ "$COUNT" -eq 0 ]] && COUNT=$(git diff --name-only | wc -l)
echo "$COUNT file(s) changed."

# Ask user whether to proceed with committing and pushing
read -rp "Do you want to commit and push these changes? [y/N] " ANSWER
ANSWER_LOWER=$(echo "$ANSWER" | tr '[:upper:]' '[:lower:]')
if [[ "$ANSWER_LOWER" != "y" ]]; then
    echo "Aborting."
    exit 0
fi

# Ask for a Conventional Commit message, and validate it
while true; do
    echo
    echo "Enter a Conventional Commit message (e.g., 'fix: correct permission issue'):"
    read -rp "> " COMMIT_MSG

    # Validate message against the conventional commit pattern
    if [[ "$COMMIT_MSG" =~ ^(feat|fix|chore|docs|style|refactor|perf|test)(\(.+\))?:\ .+ ]]; then
        break
    else
        echo "Invalid commit message format. Use Conventional Commits (e.g., 'feat: add new thing')."
    fi
done

# Stage and commit all changes
git add .
git commit -m "$COMMIT_MSG"

# Ask user if they want to push
BRANCH=$(git rev-parse --abbrev-ref HEAD)
read -rp "Push to origin/$BRANCH? [y/N] " PUSH_CONFIRM
PUSH_CONFIRM_LOWER=$(echo "$PUSH_CONFIRM" | tr '[:upper:]' '[:lower:]')
if [[ "$PUSH_CONFIRM_LOWER" == "y" ]]; then
    git push origin "$BRANCH"
    echo "Changes pushed."
else
    echo "Changes committed but not pushed."
fi
