Installing PyEVSpace
====================

.. role:: console(code)
    :language: console

PIP
---
The best way to install PyEVSpace is to download it from `PyPi 
<https://pypi.org/project/pyevspace/>`_ using :console:`pip`.

.. code-block:: console

    pip install pyevspace

It is best practice to use a virtual environment while installing
packages. Check out the Python docs for more information on `virtual 
envirnoments 
<https://docs.python.org/3/library/venv.html?highlight=venv#module-venv>`_.

From source
-----------
Building The Extension
^^^^^^^^^^^^^^^^^^^^^^
The package can also be built directly from `source 
<https://github.com/qbizzle68/pyevspace>`_ if you choose. The extension
can be built inplace using setuptools with 

.. code-block:: console
    
    python setup.py build_ext --inplace

.. note::

    The extension module is named :file:`_pyevspace.pyd` and is imported
    in the :file:`__init__.py` file of the pyevspace package. If you need
    to access the extension directly from here remember to import it using
    the correct name.

Building The Package
^^^^^^^^^^^^^^^^^^^^
To instead install the package (don't forget to activate your virtual 
envirnoment) use

.. code-block:: console

    pip install .

.. note::

    Installing in editable mode (:console:`-e`) most likely won't do 
    anything differently for you because the package is mostly written in
    C, requiring the extension to be re-compiled everytime it is edited.

Using Visual Studio
^^^^^^^^^^^^^^^^^^^
PyEVSpace was built using Visual Studio, so you can work on it straight
away using the solution in the repository. Clone the repo onto your machine
using git

.. code-block:: console

    git clone https://github.com/qbizzle68/pyevspace.git 

and open the `pyevspace.sln` file to develop using Visual Studio. You can
also create a new project and use the 'Clone a repository' option, using
https://github.com/qbizzle68/pyevspace as the URL to clone from. You will
need to create a virtual envirnoment to support the other tools used in
development like sphinx and tox. Activate your virtual envirnoment and
run:

.. code-block:: console

    pip install -r requirements.txt

Optionally you can manually install only the packages you will need.

The Visual Studio solution is configured to build the library to 
'pyevspace/bin/$(Platform)', where $(Platform) is something like x86 or x64. To build
the package using setup.py check the section above.

To use a difference IDE check how to develop Python extension modules
on your specific development environment. You will need to list 
`pyevspace/pyevspace/include` in your include path.

Testing
-------
The test module is included in the distributed package and is built using
unittest, Python's default testing suite. If you have PyEVSpace in your
environment, simply run:

.. code-block:: console

    python -m unittest pyevspace.tests.pyevspace_tests

If for some reason you want a more verbose output, use the -v flag as well.

Tox
^^^
Since PyEVSpace supports multiple Python versions, we use tox to test across
those versions. Tox is already configured to test Python versions 3.8 through
3.11. To ensure any changes work on all supported versions, run:

.. code-block:: console

    tox

It may take a minute to run the entire test suites for every version.