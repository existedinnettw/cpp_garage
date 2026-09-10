# uv + Conan + nanobind demo

A `uv`-managed Python project whose compiled extension is built **directly** on
top of a Conan-managed C++ library. `uv sync` (or `uv build`) is the only
command you run; underneath it, Conan resolves the C++ dependencies, CMake
builds the C++ library and the nanobind module, and the result is packed into a
wheel.

```
uv sync / uv build
  └─ conan-py-build            (PEP 517 build backend, from PyPI)
       └─ conan build          (conanfile.py: requires fmt + nanobind from Conan Center)
            └─ CMake + Ninja
                 ├─ cpp/        libgeom.a     — plain C++ library, links fmt
                 └─ bindings/   _core.so      — nanobind module, links libgeom
       └─ conan export-pkg     (package() -> cmake --install -> into the wheel)
```

This follows the approach recommended in the Conan docs
([Conan + Python integration](https://docs.conan.io/2/integrations/python.html))
using [conan-py-build](https://conan-py-build.conan.io) and
[nanobind](https://github.com/wjakob/nanobind).

## Layout

| Path | Role |
| --- | --- |
| `pyproject.toml` | uv project; `build-backend = "conan_py_build.build"` |
| `conanfile.py` | Conan recipe: C++ deps (`fmt`, `nanobind`), CMake build, `package()` |
| `cpp17.profile` | Extra Conan profile layered on your default (nanobind needs C++17) |
| `CMakeLists.txt` | Builds `cpp/` then the nanobind module, installs it into the package dir |
| `cpp/` | The underlying C++ library `geom` (depends on `fmt`): `Point` struct, `Polygon` class, `Stats<T>` / `clamp_all<T>` templates |
| `bindings/module.cpp` | nanobind glue only, no logic; binds the class, template instantiations and overloads |
| `src/uv_conan_nanobind_demo/` | Python package; `__init__.py` re-exports `_core` and adds a pure-Python helper |
| `tests/` | pytest suite exercising the extension |

## Prerequisites

- `uv`, a C++17 compiler, `cmake` and `ninja` on `PATH`
- A Conan 2 default profile (`conan profile detect`). Conan itself is installed
  into uv's isolated build environment automatically via the backend's
  dependencies, so no global `pip install conan` is required.

## Usage

```bash
cd uv_conan_nanobind_demo

uv sync                 # builds C++ deps + libgeom + _core.so, installs into .venv
uv run pytest           # run the tests
uv run python -c 'import uv_conan_nanobind_demo as g; print(g.distance(g.Point(0,0), g.Point(3,4)))'

uv build                # produce sdist + wheel under dist/
```

Any change under `cpp/`, `bindings/` or the build files triggers a rebuild on
the next `uv sync` (see `[tool.uv].cache-keys`).

## What the bindings demonstrate

| C++ (`cpp/include/geom/`) | Python | nanobind feature |
| --- | --- | --- |
| `struct Point {x, y}` | `Point(x, y)`, `.x`, `==`, `repr` | `def_rw`, dunder methods |
| `class Polygon` with methods, `at()` throwing `std::out_of_range` | `Polygon([...])`, `.area()`, `len()`, `poly[i]`, `for p in poly` | `class_`, `def_prop_ro`, automatic `std::exception` -> Python exception translation (`IndexError`, `RuntimeError`, `ValueError`) |
| `template <typename T> class Stats` | `StatsF64`, `StatsI64` | a `bind_stats<T>()` helper binds each instantiation; `stats_of()` in `__init__.py` picks one from the Python element types |
| `template <typename T> clamp_all(...)` | `clamp_all(values, lo, hi)` | two instantiations registered under one name become Python overloads; int lists hit `clamp_all<long>`, anything with a float falls through to `clamp_all<double>` |

```python
>>> import uv_conan_nanobind_demo as g
>>> sq = g.Polygon([g.Point(0,0), g.Point(1,0), g.Point(1,1), g.Point(0,1)])
>>> sq
Polygon(n=4, area=1.000, perimeter=4.000)
>>> g.stats_of([1, 2, 3])
Stats<int>(n=3, min=1, max=3, mean=2.000)
>>> g.clamp_all([-5, 3, 99], 0, 10)
[0, 3, 10]
```

## Notes

- `[tool.conan-py-build.sdist].include` lists `cpp`, `bindings`, `tests` as plain
  paths (not globs). Without it the sdist only contains the default files and a
  wheel cannot be built from it.

- `[tool.conan-py-build].extra-arguments` is appended to **both** `conan build`
  and `conan export-pkg`, so only flags valid for both go there. The backend
  already passes `--build=missing`; do not add it again or `export-pkg` fails.
- `-r conancenter` is set so the build never touches other remotes you may have
  configured (a private remote that prompts for a login would break the
  non-interactive build with `EOF when reading a line`).
- Conan settings come from your `default` profile with `cpp17.profile`
  composed on top. Use `uv sync -C host-profile=<name>` to select a different
  host profile (conan-py-build config setting).
- The backend installs the newest `conan` in the isolated build env. If your
  system Conan is older you will see a harmless `Downgrading cache` warning.
