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
todo: find out what needs to be done to use VS out of the box
