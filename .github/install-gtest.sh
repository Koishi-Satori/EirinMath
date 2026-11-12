#!/usr/bin/env bash
#
# Best-effort installation of a system GoogleTest for the Linux CI jobs.
#
# The runner images point apt at Ubuntu mirrors that occasionally stop
# resolving.  Try the configured mirrors first, then fall back to
# mirrors.edge.kernel.org; if even that fails, tell CMake to use the
# FetchContent copy instead (the build itself always works without a
# system GoogleTest).
set -u

readonly APT_OUTPUT="${GITHUB_OUTPUT:-/dev/null}"

switch_apt_mirror() {
    echo "::warning::Ubuntu mirrors are unreachable; switching apt to mirrors.edge.kernel.org"
    if [ -f /etc/apt/apt-mirrors.txt ]; then
        echo "https://mirrors.edge.kernel.org/ubuntu/" | sudo tee /etc/apt/apt-mirrors.txt > /dev/null
    fi
    for file in /etc/apt/sources.list /etc/apt/sources.list.d/ubuntu.sources; do
        [ -f "$file" ] || continue
        sudo sed -i -E 's#https?://[A-Za-z0-9._-]*ubuntu\.com/ubuntu/?#https://mirrors.edge.kernel.org/ubuntu/#g' "$file"
    done
}

try_install() {
    timeout 180 sudo apt-get update -o Acquire::Retries=2 &&
        timeout 300 sudo apt-get install -y --no-install-recommends libgtest-dev
}

switched=0
if ! getent hosts azure.archive.ubuntu.com > /dev/null 2>&1; then
    switch_apt_mirror
    switched=1
fi

if try_install; then
    echo "system_gtest=true" >> "$APT_OUTPUT"
    exit 0
fi

if [ "$switched" = 0 ]; then
    switch_apt_mirror
    if try_install; then
        echo "system_gtest=true" >> "$APT_OUTPUT"
        exit 0
    fi
fi

echo "::warning::libgtest-dev is unavailable; CMake will use the FetchContent GoogleTest"
echo "system_gtest=false" >> "$APT_OUTPUT"
exit 0
