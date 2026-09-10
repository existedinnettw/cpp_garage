#!/usr/bin/env bash
# Build the whole workspace with a single colcon invocation.
#
# Every ROS package keeps its own conanfile.py, but all of them are installed
# into ONE Conan output folder. The generated CMakeDeps config files simply
# accumulate there (fmt-config.cmake, nlohmann_json-config.cmake, ...), and the
# single conan_toolchain.cmake / conanrosenv.sh puts that folder on
# CMAKE_PREFIX_PATH, so one plain `colcon build` sees every dependency.
set -eo pipefail  # no -u: ROS setup.bash is not set -u clean
cd "$(dirname "$0")"

ROS_DISTRO="${ROS_DISTRO:-jazzy}"
CONAN_OUT="install/conan"

# 1. conan install for every package that has a conanfile.py
for conanfile in src/*/conanfile.py; do
  echo "=== conan install $(dirname "${conanfile}")"
  conan install "$(dirname "${conanfile}")" --build=missing -r conancenter \
    --output-folder "${CONAN_OUT}"
done

# 2. ROS env + the (single) Conan ROS env
# shellcheck disable=SC1090
source "/opt/ros/${ROS_DISTRO}/setup.bash"
# shellcheck disable=SC1090
source "${CONAN_OUT}/conanrosenv.sh"

# 3. one colcon build; extra args are passed through, e.g.
#    ./build.sh --packages-up-to consumer
colcon build --symlink-install "$@"
