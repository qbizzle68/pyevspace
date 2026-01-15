from setuptools import setup, Extension

setup(
    ext_modules=[
        Extension(
            name='pyevspace._pyevspace',
            include_dirs=[
                'include',
                'cpp/evspace/include',
                'cpp/evspace/external',
            ],
            sources=['src/pyevspace/pyevspacemodule.cpp'],
        ),
    ]
)
