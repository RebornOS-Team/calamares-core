set -o xtrace

INITIAL_BRANCH="$(git branch --show-current)"

(
    cd "$PROJECT_DIRECTORY" \
        && { 
            git remote add upstream "https://github.com/calamares/calamares" \
            || true
        } \
        && git fetch --all \
        && git checkout -B _calamares --track upstream/calamares \
        && git reset --hard upstream/calamares
)

git checkout "$INITIAL_BRANCH"

set +o xtrace