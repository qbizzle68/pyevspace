from setuptools import setup, Extension

import pyevspace
description, long_description = pyevspace.__doc__.split('\n', 1)

ext_modules = [Extension(
    '_pyevspace',
    include_dirs=['pyevspace/include'],
    sources=['pyevspace/src/evspacemodule.c'],
)]

setup(name='pyevspace',
      version='0.0.11.1',
      author='Quinton Barnes',
      author_email='devqbizzle68@gmail.com',
      description='A Euclidean vector space module.',
      long_description=long_description,
      long_description_content_type='text/markdown',
      license='MIT',
      url='https://github.com/qbizzle68/pyevspace',
      classifiers=[
          'Development Status :: 3 - Alpha',
          'Intended Audience :: Science/Research',
          'Intended Audience :: Developers',
          'License :: OSI Approved :: MIT License',
          'Programming Language :: C',
          'Programming Language :: Python :: 3',
          'Programming Language :: Python :: Implementation :: CPython',
          'Topic :: Scientific/Engineering :: Mathematics',
          'Topic :: Scientific/Engineering :: Physics',
          'Topic :: Software Development :: Libraries :: Python Modules',
      ],
      packages=['pyevspace', 'pyevspace.examples', 'pyevspace.tests'],
      ext_modules=ext_modules,
      )
