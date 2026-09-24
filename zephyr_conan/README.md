# zephyr_conan: a Zephyr app with Conan dependencies ("Conan outside")

A basic Zephyr application whose third-party C++ libraries come from **Conan**,
built for an STM32 board (`nucleo_f429zi`) and for `native_sim`. Conan owns the
*package graph*; `west build` owns the *firmware image*:

```text
conan build . -pr:h profiles/<board>
  ├── zephyr/4.4.0        pinned west workspace (zephyr + hal_stm32 + cmsis_6 + mcuboot + mbedtls + tf-psa-crypto)   -> ZEPHYR_BASE
  ├── zephyr-sdk/1.0.1    tool_requires: arm-zephyr-eabi-* on PATH                          -> ZEPHYR_SDK_INSTALL_DIR
  ├── spdlog/1.17.0       (native_sim)    } CMakeDeps -> build/<board>/conan/<pkg>-config.cmake  -> CMAKE_PREFIX_PATH
  │     └── fmt/12.1.0    (nucleo_f429zi) }
  ├── generated Zephyr module build/<board>/conan/conan (build-type glue)     -> ZEPHYR_EXTRA_MODULES
  └── build():  west build -b <board> -d build/<board>/zephyr .
```

Zephyr never sees `conan_toolchain.cmake`, and `west build` gets no `-D`
arguments. Zephyr's own toolchain, Kconfig and devicetree logic is untouched;
everything Conan contributes arrives through the build *environment*:
`ZEPHYR_BASE`, `ZEPHYR_SDK_INSTALL_DIR`/`PATH`, `CMAKE_PREFIX_PATH` for the
`find_package()` config files and `ZEPHYR_EXTRA_MODULES` for one tiny
generated module. The `zephyr` and `zephyr-sdk` recipes live in
[conan-recipes](https://github.com/existedinnettw/conan-recipes).

| Path | Role |
| --- | --- |
| `conanfile.py` | Package graph, `board` option, `generate()` = CMakeDeps + env + Zephyr module, `build()` = `west build` |
| `profiles/nucleo_f429zi` | Host profile describing the MCU: `os=baremetal`, SDK gcc, Zephyr's exact `-mcpu/-specs/...` flags |
| `profiles/native_sim` | Host profile for the 64-bit Linux simulator (default profile + board) |
| `CMakeLists.txt`, `prj.conf` | Plain Zephyr app. `find_package(spdlog)` / `find_package(fmt)`, nothing Conan-specific |
| `src/log.hpp` | Facade: spdlog with a `printk` sink where available, otherwise `fmt` + `printk` |
| `src/main.cpp` | Logs a line per second |

## Prerequisites

* Conan 2, CMake >= 3.20, a host gcc/g++ (for `native_sim`).
* `west`, `ninja` and Zephyr's Python requirements on `PATH`, e.g.

  ```bash
  uv venv ~/.venvs/zephyr && uv pip install --python ~/.venvs/zephyr/bin/python \
      "west>=1.2" ninja pyelftools PyYAML pykwalify jsonschema canopen packaging \
      "patool>=2.0.0" psutil pylink-square pyserial requests semver tqdm reuse anytree intelhex
  export PATH=~/.venvs/zephyr/bin:$PATH
  ```

* The `zephyr` and `zephyr-sdk` packages in the Conan cache (or on a remote):

  ```bash
  conan create <conan-recipes>/recipes/zephyr-sdk/all --version=1.0.1 -tf=""
  conan create <conan-recipes>/recipes/zephyr/all --version=4.4.0 -tf="" \
      -o "zephyr/*:projects=hal_stm32,cmsis_6,mcuboot,mbedtls,tf-psa-crypto"
  ```

  The `projects` option must match the one pinned in `conanfile.py`, otherwise
  Conan will happily fetch a *full* multi-GB workspace for you.

## Build

```bash
# native_sim: spdlog + fmt from Conan, host gcc
conan build . -pr:h profiles/native_sim --build=missing
./build/native_sim_native_64/zephyr/zephyr/zephyr.exe

# STM32 Nucleo-F429ZI: fmt from Conan, cross-built by the Conan-provided Zephyr SDK
conan build . -pr:h profiles/nucleo_f429zi --build=missing
west flash -d build/nucleo_f429zi/zephyr      # with the SDK env: source build/nucleo_f429zi/conan/conanbuild.sh
```

Expected output:

```text
*** Booting Zephyr OS build 4.4.0 ***
[app] [info] zephyr_conan on native_sim/native/64 (Zephyr 4.4.0), logging via spdlog 1.17.0
[app] [info] tick    0  uptime        0 ms
[app] [info] tick    1  uptime     1010 ms
```

Anything `west` accepts still works on the generated build directory
(`west build -d build/<board>/zephyr -t menuconfig`, `west flash`, `west debug`),
because Conan only ran `west build` for you.

## How the pieces fit

* **Board = profile + option.** The Conan *host profile* is the ABI contract for
  the target (`os=baremetal`, `arch=armv7`, `compiler.version=14`, the exact
  `-mcpu`/`-mthumb`/`-specs=picolibc.specs`/`-fno-exceptions` flags Zephyr uses
  for the board, `*:fPIC=False` because Zephyr links `-fno-pic`), so
  Conan-built libraries share multilib, libc and ABI with the firmware. The
  `board` option is just the string handed to `west build -b`.
* **`[tool_requires] zephyr-sdk/1.0.1` in the profile** puts the SDK's
  `arm-zephyr-eabi-gcc` on `PATH` for *every* package in the graph, so `fmt`
  is cross-compiled by the same compiler Zephyr uses.
  `tools.build:compiler_executables` names it; Conan maps `os=baremetal` to
  `CMAKE_SYSTEM_NAME=Generic` automatically.
* **`CMakeDeps` + `CMAKE_PREFIX_PATH`.** `generate()` runs `CMakeDeps` and
  prepends the generators folder to the `CMAKE_PREFIX_PATH` environment
  variable (the same channel the `zephyr-sdk` package uses), so the unmodified
  `find_package(spdlog)` in `CMakeLists.txt` resolves.
* **A generated Zephyr module reconciles `CMAKE_BUILD_TYPE`.** Conan's
  CMakeDeps refuses to work without `CMAKE_BUILD_TYPE` (it selects the
  configuration of the prebuilt libraries), whereas Zephyr leaves it unset and
  picks `-Os`/`-O2`/`-Og` via Kconfig. Passing `-DCMAKE_BUILD_TYPE=Release`
  works but silently adds CMake's `-O3 -DNDEBUG` defaults to *every* Zephyr
  translation unit (checked in `compile_commands.json`) and trips Zephyr's
  build-type warning. So `generate()` writes a two-file Zephyr module
  (`zephyr/module.yml` + `CMakeLists.txt`) into the generators folder and
  exports it via `ZEPHYR_EXTRA_MODULES`. Module CMake runs inside
  `find_package(Zephyr)`, i.e. before the app's `find_package(spdlog)`, where
  it sets `CMAKE_BUILD_TYPE` to Conan's build type, blanks
  `CMAKE_<LANG>_FLAGS_<CONFIG>` and sets `NO_BUILD_TYPE_WARNING`. Result: the
  firmware compiles with exactly Zephyr's flags, `CMakeLists.txt` stays pure
  Zephyr, and the same environment works unchanged under sysbuild (which does
  not forward `-DCMAKE_*` to its images).
* **`zephyr/*:projects` is pinned in `default_options`**: the firmware states
  which west modules it needs, and the package id follows.

## Findings / gotchas

* **spdlog does not build for the MCU.** The Zephyr SDK's `libstdc++` is
  built without gthreads (`/* #undef _GLIBCXX_HAS_GTHREADS */`), so
  `std::mutex`, `std::condition_variable` and `std::thread` do not exist, and
  spdlog needs all three (`details/backtracer.h`, `details/periodic_worker.h`).
  Zephyr documents `std::thread`/`std::mutex` as unsupported. Hence
  `conanfile.py` requires `spdlog` only when `os != baremetal` and falls back
  to `fmt`, spdlog's formatting core, which the fmt recipe explicitly supports
  on `baremetal` (`with_os_api=False`). `src/log.hpp` hides the difference.
* **Include order on native_sim.** Zephyr's `toolchain/gcc.h` defines a
  `__unused` macro that collides with a struct member in glibc's
  `<bits/struct_mutex.h>`; include standard/third-party headers before
  `<zephyr/kernel.h>`.
* **`fPIC=False` on bare metal.** Conan's default `fPIC=True` produced GOT
  references (`_GLOBAL_OFFSET_TABLE_ ... defined in discarded section .got.plt`)
  that Zephyr's `-fno-pic` link rejects.
* **`zephyr`/`zephyr-sdk` must not emit CMake config files.** Their recipes
  set `cmake_find_mode=none`; otherwise CMakeDeps' `zephyr-config.cmake` on
  `CMAKE_PREFIX_PATH` shadows Zephyr's real `find_package(Zephyr)`.
* The private Conan remote prompts for a login; pass `-r conancenter` when
  you only need Conan Center packages.

## Relation to the design discussion

This follows the "Conan outside" conclusion of the design discussion that
preceded it: Conan is the package/environment graph, `west build` (and
sysbuild) the untouched firmware image graph, CMake imported targets are the
seam, the SDK is a `tool_requires`, and the pinned west workspace is one
opaque platform package. Two refinements came out of actually building it:

* **The generated Zephyr module is not "wrapping Conan packages in dummy
  modules"** (which the discussion rightly warns against). Libraries stay
  plain `find_package()` targets; the module carries no packages, only the
  build-type reconciliation that Conan's generators force on a build system
  which does not use `CMAKE_BUILD_TYPE`. `CMakeConfigDeps` and its
  `conan_cmakedeps_paths.cmake`, suggested there as the toolchain-free
  alternative, have the same `CMAKE_BUILD_TYPE` requirement.
* **The host profile is still hand-written per board.** The discussion
  identifies deriving the Conan host profile from Zephyr's board/toolchain
  selection as the hard, unsolved piece, and this demo does not solve it:
  `profiles/nucleo_f429zi` was written by reading the flags of a probe build.
  Generating it (board -> arch/CPU/FPU/ABI/SDK version) is the natural next
  step if this grows beyond one MCU family.

See `../zephyr_conan_sysbuild` for the same app built through sysbuild with
MCUboot.
