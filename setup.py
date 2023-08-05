from setuptools import setup, Extension

setup(
    ext_modules=[
        Extension(
            name='pyevspace.core',
            include_dirs=['src/pyevspace/include'],
            sources=['src/pyevspace/src/evspacemodule.c'],
        ),
    ]
)
