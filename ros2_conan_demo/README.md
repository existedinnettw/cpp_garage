# ROS 2 + Conan demo

A colcon workspace with two ament packages that consume C++ libraries from
**Conan** instead of `apt`/rosdep, using Conan's built-in
[`ROSEnv` generator](https://docs.conan.io/2/integrations/ros.html).
Every ROS package carries its own `conanfile.py`. There is nothing
Conan-specific in any `CMakeLists.txt` or `package.xml`: the packages call
`find_package(fmt)` / `find_package(nlohmann_json)` as they would for any
system library.

```
src/
  str_printer/
    conanfile.py           requires fmt/11.1.3
    CMakeLists.txt         ament_cmake library: find_package(fmt) -> fmt::fmt
    package.xml
  consumer/
    conanfile.py           requires nlohmann_json/3.12.0
    CMakeLists.txt         rclcpp node: find_package(str_printer), find_package(nlohmann_json)
    package.xml
build.sh                   conan install each package into install/conan -> source conanrosenv.sh -> ONE colcon build
```

## How it works

Each package's `conanfile.py` is installed with
`conan install src/<pkg> --output-folder install/conan`. All packages share
that **one** output folder, into which Conan writes:

- `fmt-config.cmake`, `nlohmann_json-config.cmake`, ... (**CMakeDeps**), one
  set per dependency; installs for different packages just accumulate,
- `conan_toolchain.cmake` (**CMakeToolchain**), which prepends its own folder
  to `CMAKE_PREFIX_PATH`,
- `conanrosenv.sh` (**ROSEnv**), which exports `CMAKE_TOOLCHAIN_FILE` and
  `CMAKE_BUILD_TYPE`.

Sourcing `conanrosenv.sh` after the ROS `setup.bash` makes every CMake
configure that colcon runs use the Conan toolchain, so a single, unmodified
`colcon build` finds every Conan library. `--packages-up-to`,
`--packages-select` etc. work as usual.

`str_printer` links `fmt::fmt` and calls `ament_export_dependencies(fmt)`, so
`consumer` gets `find_dependency(fmt)` for free. Because the output folder is
shared, `consumer`'s conanfile only needs to list what it uses directly
(nlohmann_json).

Sharing one folder means all packages must agree on dependency versions, which
they have to anyway to link into one executable. If two conanfiles pinned
different `fmt` versions, the last `conan install` would silently win; Conan's
[workspace feature](https://docs.conan.io/2/incubating.html#workspaces) is the
eventual "proper" answer to resolving a multi-package graph at once, but it is
still incubating so this demo keeps to plain `conan install`.

This machine also has Ubuntu's `libfmt-dev` 9.1.0 installed. The node prints
the fmt and nlohmann_json versions it was compiled against so you can see that
the Conan packages are the ones being used.

## Build

Prerequisites: ROS 2 (tested with Jazzy), `colcon`, Conan 2.x with a default
profile (`conan profile detect`).

```bash
./build.sh                              # everything
./build.sh --packages-up-to consumer    # args are passed to colcon
```

or step by step:

```bash
conan install src/str_printer --build=missing --output-folder install/conan
conan install src/consumer    --build=missing --output-folder install/conan

source /opt/ros/jazzy/setup.bash
source install/conan/conanrosenv.sh
colcon build --symlink-install
```

`conan install` only needs to be re-run when a `conanfile.py` changes; after
that it is just `source ... conanrosenv.sh` + `colcon build` in any new shell.
Running `colcon build` without `conanrosenv.sh` sourced makes
`find_package(fmt)` fall back to whatever the system provides (or fail).

## Run

```bash
source install/setup.bash
ros2 run consumer main                              # publishes /greeting forever
ros2 run consumer main --ros-args -p max_count:=3   # exits after 3 messages
```

Expected output:

```
[INFO] [greeter]: str_printer was built against fmt 11.1.3, consumer uses nlohmann_json 3.12.0
[INFO] [greeter]: publishing: '{"seq":1,"text":"Hello, ROS 2 + Conan! (#1)"}'
...
```

## Notes

- `conan install` does not need the ROS environment; it can run in a clean
  shell or a CI cache stage. Only the `colcon build` step needs both
  `setup.bash` and `conanrosenv.sh`.
- The `conanfile.py` files declare `name` but no `version`/`package()`:
  Conan is used purely as a dependency provider, colcon still owns the build.
- Static libraries (the Conan default) are simplest: no runtime path setup is
  needed for `ros2 run`. If you switch to `-o "*:shared=True"`, also source
  `install/conan/conanrun.sh` before running the node.

## TODO

- Wait for Conan and ROS to improve their integration. Today every package
  needs its own `conan install`, which is annoying to repeat. Ideally colcon
  (or `conan workspace install`, once it leaves incubation) would resolve the
  whole workspace's Conan graph in one step, and each package's
  `conanfile.py` would be picked up automatically the way `package.xml` is.
