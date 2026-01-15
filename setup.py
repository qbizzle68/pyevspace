from setuptools import setup, Extension

setup(
    ext_modules=[
        Extension(
            name='pyevspace._pyevspace',
            include_dirs=[
                'include',
                'cpp/evspace/include',
            ],
            sources=['src/pyevspace/pyevspacemodule.cpp'],
        ),
    ]
)
