.. include:: /global.rst

=======================
PyEVSpace Documentation
=======================

.. module:: pyevspace

The pyevspace module defines a vector and a matrix type to define a Euclidean 
vector space within Python projects. The Vector allows you to represent physical 
quantites like positions, velocities, accelerations and forces. The Matrix type 
allows us to represent idealized reference frames and to create linear 
transformations, most usefully pure rotations to move vectors between reference 
frames. A handful of module level functions for manipulating vectors and matrices 
are also available.

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

Available Types
---------------

.. class:: Vector
	:noindex:

	A three-dimensional array object meant to represent a physical vector in
	a Euclidean vector space. The internal data buffer is a C type double, 
	which supports :ref:`numeric <numeric-def>` types.

.. class:: Matrix
	:noindex:

	A three by three-dimensional array object mean to represent a linear
	transformation of physical vectors in a Euclidean vector space. The
	internal data buffer is a C type :c:`double`, which supports 
	:ref:`numeric <numeric-def>` types.

Module Functions
----------------

Vector
^^^^^^



Matrix
^^^^^^

.. function:: det(matrix)

	Computes the determinate of a matrix. Useful for determining if a matrix is
	invertible i.e. the determinate is non-zero.

	:param Matrix matrix: matrix to compute the determinate of
	:return: determinate of `matrix`
	:rtype: Matrix

.. function:: transpose(matrix)

	Returns the transpose of `matrix`. The transpose of a matrix is simply a
	flipped matrix by switching its rows and columns. Therefore if T is the
	transpose of M, :python:`M[i, j] == T[j, i]`.

	:param Matrix matrix: the matrix to transpose
	:return: the transpose of `matrix`
	:rtype: Matrix