Installing PyEVSpace
====================

.. role:: console(code)
    :language: console

.. role:: python(code)
    :language: python

Pip
---
The best way to install PyEVSpace is to download it from `PyPi 
<https://pypi.org/project/pyevspace/>`_ into your
`virtual environment <https://docs.python.org/3/library/venv.html?highlight=venv#module-venv>`_
using :console:`pip`

.. code-block:: console

    pip install pyevspace

From source
-----------

.. _git_clone_label:

Installing From source
^^^^^^^^^^^^^^^^^^^^^^
To install the package from the current `source <https://github.com/qbizzle68/pyevspace>`_
code, first clone the repo to your machine

.. code-block:: console

    git clone https://github.com/qbizzle68/pyevspace.git

Then to install run

.. code-block:: console

    pip install <path-to-cloned-repo>

.. note::

    The :console:`-e` or :console:`--editable` flag can be used to install the
    package as editable, however this only affects pure python files. Since any
    changes made to the source code need to be re-compiled, this only affects
    the :file:`__init__.py` file which may or may not be useful for you.

Building In Place
^^^^^^^^^^^^^^^^^
While not recommended, it is possible to build the package inplace with:

.. code-block:: console

    python setup.py build_ext --inplace --build-lib <path-to-build-pyevspace>

the package will be built at <path-to-build-pyevspace>. 

.. note::
    The extension module is named :file:`_pyevspace.pyd`, and since building
    in place does not include the :file:`__init__.py` file, import mechanics
    will be different. For example, if normally importing :class:`Vector`
    using :python:`from pyevspace import Vector`, you now need
    :python:`from pyevspace._pyevspace import Vector`.

Testing
-------
To test the package, first :ref:`clone the source repo<git_clone_label>`.
Then run 

.. code-block:: console

    pytest

which will output the test results to the console. The test source code includes
two smaller extension modules to test PyEVSpace's C API capsule for C and C++
compatibility for others wanting to include and access PyEVSpace from their
own extension modules. To skip building and running these tests, pass the
:console:`--skip-capsule` flag to pytest when invoking the tests.

Tox
^^^
PyEVSpace currently supports all actively maintained versions of Python, which
includes 3.10 to 3.14. The package is configured to test all versions (if installed
on your machine) using Tox. Running Tox with

.. code-block:: console

    tox

will test all available supported versions on your machine, as well as linting
project python files using flake8. To test only a specific version of python
(or the linter) specify the version using the :console:`-e` flag when invoking
Tox. For example, to only test PyEVSpace built for python 3.13, use

.. code-block:: console

    tox -e 3.13

The available environments that can be run using Tox are "3.10", "3.11", "3.12",
"3.13", "3.14", "lint", and "stubtest".
