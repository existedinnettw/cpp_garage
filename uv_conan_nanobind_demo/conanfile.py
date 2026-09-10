from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class UvConanNanobindDemoConan(ConanFile):
    """Conan recipe driven by conan-py-build.

    conan-py-build runs `conan build` (requirements -> generate -> build) and then
    `package()`; whatever `cmake.install()` puts under a directory named like the
    Python package ends up inside the wheel.
    """

    name = "uv_conan_nanobind_demo"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        # C++ dependencies of the underlying library and of the bindings.
        self.requires("fmt/12.1.0")
        self.requires("nanobind/2.13.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
