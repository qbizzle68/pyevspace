from pathlib import Path
import sys

from setuptools import Extension
from setuptools.dist import Distribution
import setuptools.command.build_ext


def build_test_extension():
    ext_cpp = Extension(
        "capsule_consumer_cpp",
        sources=["tests/_test_capsule/capsule_consumer_module.cpp"],
        include_dirs=["include", "cpp/evspace/include",
                      "cpp/evspace/external"],
        extra_compile_args=["-std=c++17"],
    )
    ext_c = Extension(
        "capsule_consumer_c",
        sources=["tests/_test_capsule/capsule_consumer_module.c"],
        include_dirs=["include"],
    )
    dist = Distribution(attrs={"ext_modules": [ext_cpp, ext_c]})
    cmd = setuptools.command.build_ext.build_ext(dist)
    cmd.build_lib = "tests/_test_capsule"
    cmd.ensure_finalized()
    cmd.run()


def pytest_addoption(parser):
    # Allow skipping of building capsule extension modules for tests
    parser.addoption("--skip-capsule", action="store_true", default=False,
                     help="Skip building C/C++ API capsule modules")


def pytest_ignore_collect(collection_path, config):
    """Don't collect capsule extension module dependent test
    modules to avoid import errors.
    """

    if config.getoption("--skip-capsule"):
        if collection_path.name == "capsule_test.py":
            return True


def pytest_configure(config):
    """Only build capsule extension modules if --skip-capsule
    is not specified.
    """

    if config.getoption("--skip-capsule") is False:
        build_test_extension()
        sys.path.insert(0, str(Path(__file__).parent / "_test_capsule"))
