from setuptools import setup, Extension

def initTxt():
    with open('pyevspace/__init__.py', 'r') as f:
        txt = f.readlines()

    description = txt[0][4:-1]
    version = txt[-1].split(' ', 4)[2][1:-1]

    return description, version

    #versionLine = txt[-1]
    #return versionLine.split(' ', 4)[2][1:-1]

def readme():
    with open('README.md', 'r') as f:
        return f.read()

description, __version__ = initTxt()

ext_modules = [Extension(
    '_pyevspace',
    include_dirs=['pyevspace/include'],
    sources=['pyevspace/src/evspacemodule.c'],
)]

setup(name='pyevspace',
      version=__version__,
      author='Quinton Barnes',
      author_email='devqbizzle68@gmail.com',
      description=description,
      long_description=readme(),
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
      packages=['pyevspace', 'pyevspace.examples', 'pyevspace.tests'],
      ext_modules=ext_modules,
      )
