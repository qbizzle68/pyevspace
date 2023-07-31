.. include:: /global.rst

=============
PyEVSpace API
=============

PyEVSpace Documentation
-----------------------

The documentation for the package is split into four sections: Vector,
Matrix, Rotation and ReferenceFrame APIs.

.. _numeric-def:

.. note::

	The underlying data type of the internal array is a C type
	:c:`double`. This obviously supports any type that can be represented
	as a Python :class:`float`, which includes Python :class:`int` types. 
	Therefore numeric types are interpreted in the following order:

		1. :class:`float`\ s are converted to a C :c:`double`
		2. A conversion is attempted with the type's :python:`__float__()` method
		3. A conversion is attempted with the type's :python:`__index__()` method
		4. No suitable conversion can be made and a TypeError is raised

	Therefore in this documentation a numeric type is any type that is
	either a :class:`float` or :class:`int` or can be converted to a
	:class:`float` or :class:`int`.

.. toctree::
	:maxdepth: 3
	
	vector
	matrix
	rotation
	refframe