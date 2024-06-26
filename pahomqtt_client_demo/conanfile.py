from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.build import check_min_cppstd

class pahomqtt_pingpongRecipe(ConanFile):
    name = "pahomqtt_pingpong"
    version = "0.1"
    package_type = "application"

    # Optional metadata
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of pahomqtt_pingpong package here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def requirements(self):
        self.requires("paho-mqtt-cpp/1.2.0")
        # self.requires("tomlplusplus/[~3]") #cpp17
        self.requires("toml11/[~3]")
        # self.requires("ghc-filesystem/[~1.5]")

    def validate(self):
        check_min_cppstd(self, 11) #paho

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        # checkout conan recipe, may check paho-c version too
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    

    
