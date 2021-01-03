import os
import platform
import shutil
import subprocess
import sys
from distutils.dir_util import copy_tree
from distutils.file_util import copy_file
from distutils.version import LooseVersion

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(["cmake", "--version"])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: "
                + ", ".join(e.name for e in self.extensions)
            )
        for ext in self.extensions:
            self.build_extension(ext)
        if self.inplace:
            self.copy_extensions_to_source()

    def build_extension(self, ext):
        print("CMake Library Output Directory = {}".format(self.build_lib))
        cmake_args = [
            "-DPYTHON_EXECUTABLE=" + sys.executable,
            "-DBUILD_EXAMPLES=OFF",
            "-DBUILD_PYTHON_BINDINGS=ON",
            "-DENABLE_TESTING=OFF",
            "-DENABLE_COVERAGE=OFF",
            "-DENABLE_STATIC_ANALYSIS=OFF",
            "-DINSTALL_PACKAGE=OFF",
        ]

        cfg = "Debug" if self.debug else "Release"
        build_args = ["--config", cfg]

        if self.parallel:
            if platform.system() == "Windows":
                if sys.maxsize > 2 ** 32:
                    cmake_args += ["-A", "x64"]
                build_args += ["--", "/MP{}".format(self.parallel)]
            else:
                build_args += ["--", "-j{}".format]

        if not os.path.exists(self.build_lib):
            os.makedirs(self.build_lib)
        subprocess.check_call(["cmake", ext.sourcedir] + cmake_args, cwd=self.build_lib)
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=self.build_lib
        )

    def copy_extensions_to_source(self):
        build_py = self.get_finalized_command("build_py")
        for ext in self.extensions:
            fullname = self.get_ext_fullname(ext.name)
            print("Package fullname = {}".format(fullname))
            filename = self.get_ext_filename(fullname)
            print("Package filename = {}".format(filename))
            modpath = fullname.split(".")
            package = ".".join(modpath[:-1])
            package_dir = build_py.get_package_dir(package)
            print("Package directory = {}".format(package_dir))
            dest_dir = os.path.join(package_dir)
            src_dir = os.path.join(self.build_lib, "lib")

            # Always copy, even if source is older than destination, to ensure
            # that the right extensions for the current Python/platform are
            # used.
            copy_tree(src_dir, dest_dir, verbose=self.verbose, dry_run=self.dry_run)
            if ext._needs_stub:
                self.write_stub(package_dir or os.curdir, ext, True)


setup(
    ext_modules=[CMakeExtension("signal_tl.c_extension")],
    cmdclass=dict(build_ext=CMakeBuild),
)
