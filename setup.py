from setuptools import setup
from setuptools.command.build_ext import build_ext


class BuildExt(build_ext):
    def build_extensions(self):
        if self.compiler.compiler_type == 'msvc':
            flag = '/std:c++17'
        else:
            flag = '-std=c++17'
        for ext in self.extensions:
            ext.extra_compile_args.append(flag)
        super().build_extensions()


setup(
    cmdclass={'build_ext': BuildExt}
)
