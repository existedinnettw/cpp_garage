from conan import ConanFile


class StrPrinterConan(ConanFile):
    """Conan dependencies of the `str_printer` ROS package.

    Only the `[requires]` and the generators are declared: the package is still
    built by colcon/ament, Conan just provides third-party libraries.
    """

    name = "str_printer"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain", "ROSEnv"

    def requirements(self):
        self.requires("fmt/11.1.3")
