from setuptools import setup, Extension, find_packages

import sys
sys.path.insert(0, 'src')
from pyevspace.__init__ import __version__, __doc__

from pathlib import Path

currentDirectory = Path(__file__).parent
longDescription = (currentDirectory / "README.md").read_text()

ext_modules = [Extension(
    'pyevspace.core',
    include_dirs=['src/pyevspace/include'],
    sources=['src/pyevspace/src/evspacemodule.c'],
)]

setup(name='pyevspace',
      version=__version__,
      author='Quinton Barnes',
      author_email='devqbizzle68@gmail.com',
      description=__doc__,
      long_description=longDescription,
      long_description_content_type='text/markdown',
      license='MIT',
      url='https://github.com/qbizzle68/pyevspace',
      classifiers=[
          'Development Status :: 4 - Beta',
          'Intended Audience :: Science/Research',
          'Intended Audience :: Developers',
          'License :: OSI Approved :: MIT License',
          'Programming Language :: Python :: 3.8',
          'Programming Language :: Python :: 3.9',
          'Programming Language :: Python :: 3.10',
          'Programming Language :: Python :: 3.11',
          'Programming Language :: Python :: Implementation :: CPython',
          'Topic :: Scientific/Engineering :: Mathematics',
          'Topic :: Scientific/Engineering :: Physics',
          'Topic :: Software Development :: Libraries :: Python Modules',
      ],
      packages=['pyevspace'],
      package_dir={'': 'src'},
      ext_modules=ext_modules,
      )
