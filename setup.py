from setuptools import setup, find_packages, Extension

setup(
    name = 'pyevspace',
    version = '0.0.1',
    package = find_packages(),
    license = 'MIT',
    classifiers = [
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: Implementation :: CPython',
        ],
    ext_modules=[
        Extension(
            'pyevspace',
            include_dirs = ['pyevspace/include'],
            sources = ['pyevspace/src/evspacemodule.c'],
        ),
    ],
)
