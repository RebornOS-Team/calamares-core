#! /usr/bin/env sh

SCRIPT_DIRECTORY="$(dirname -- "$(readlink -f -- "$0")")"
PROJECT_DIRECTORY="$(dirname -- "$SCRIPT_DIRECTORY")"
BUILD_DIRECTORY="$PROJECT_DIRECTORY"/build

set -o xtrace

INITIAL_BRANCH="$(git branch --show-current)"

(
    cd "$PROJECT_DIRECTORY" \
        && { 
            git remote add upstream "https://codeberg.org/Calamares/calamares" \
            || true
        } \
        && git fetch --all \
        && git checkout -B _calamares --track upstream/calamares \
        && git reset --hard upstream/calamares
)

git checkout "$INITIAL_BRANCH"

set +o xtrace