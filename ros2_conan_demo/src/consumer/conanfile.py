from conan import ConanFile


class ConsumerConan(ConanFile):
    """Conan dependencies of the `consumer` ROS package.

    Only direct dependencies are listed. fmt, which str_printer exports as a
    PUBLIC dependency, is found through str_printer's own conanfile.py because
    all packages install into the same Conan output folder.
    """

    name = "consumer"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain", "ROSEnv"

    def requirements(self):
        self.requires("nlohmann_json/3.12.0")
