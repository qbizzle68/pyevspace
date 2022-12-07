.. include:: /global.rst

=======================
PyEVSpace Documentation
=======================

.. module:: pyevspace

The pyevspace module defines a vector and a matrix type to define a Euclidean 
vector space within Python projects. The EVector allows you to represent physical 
quantites like positions, velocities, accelerations and forces. The EMatrix type 
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

.. class:: EVector
	:noindex:

	A three-dimensional array object meant to represent a physical vector in
	a Euclidean vector space. The internal data buffer is a C type double, 
	which supports :ref:`numeric <numeric-def>` types.

.. class:: EMatrix
	:noindex:

	A three by three-dimensional array object mean to represent a linear
	transformation of physical vectors in a Euclidean vector space. The
	internal data buffer is a C type :c:`double`, which supports 
	:ref:`numeric <numeric-def>` types.

Module Functions
----------------

Vector
^^^^^^

.. function:: dot(lhs, rhs)

	Computes the dot product between two vectors. The order of arguments
	is not significant as :python:`dot(v1, v2) == dot(v2, v1)`.

	:param EVector lhs: left-hand side of the operation
	:param EVector rhs: right-hand side of the operation
	:return: the dot product lhs and rhs
	:rtype: float
	:raises TypeError: if `lhs` or `rhs` is not EVector type

.. function:: cross(lhs, rhs)

	Computes the cross product between two vectors. A left-handed
	coordiante frame is used. If a right-handed cross product is needed,
	simply negate the answer.

	.. code-block:: python

		>>> left_cross = cross(EVector.e1, EVector.e2)
		>>> print(left_cross)
		[0, 0, 1]
		>>> right_cross = -cross(EVector.e1, EVector.e2)
		>>> print(right_cross)
		[0, 0, -1]

	:param EVector lhs: left-hand side of the operation
	:param EVector rhs: right-hand side of the operation
	:return: the cross product of lhs and rhs
	:rtype: EVector
	:raises TypeError: if `lhs` or `rhs` is not EVector type

.. function:: norm(vector)

	Returns a normalized version of `vector`. This creates a new vector, as
	opposed to the 'inplace' normalize method provided by the EVector class.
	See (:py:meth:`EVector.normalize`).

	.. code-block:: python

		>>> vector1 = EVector((1, 2, 3))
		>>> vector2 = norm(vector1)
		>>> vector1.normalize()
		>>> print(vector1)
		[0.267261, 0.534522, 0.801784]
		>>> print(vector2)
		[0.267261, 0.534522, 0.801784]

	:param EVector vector: the vector to find the normal of
	:return: the normalized version of `vector`
	:rtype: EVector
	:raises TypeError: if `vector` is not EVector type

.. function:: vang(lhs, rhs)

	Computes the shortest angle between two vectors. This relies on the
	definition of the vector dot product to compute. Since
	v1 * v2 = \|v1\|*\|v2\|*cos(theta), the dot product of v1 and v2,
	and the magnitude of v1 and v2 is all we need to compute the angle
	between the two vectors.

	.. code-block:: python

		>>> lhs = EVector((1, 2, 3))
		>>> rhs = EVector((4, 2, -3))
		>>> acos(dot(lhs, rhs) / (lhs.mag() * rhs.mag()))
		1.6204458893289022
		>>> vang(lhs, rhs)
		1.6204458893289022

	In the plane of the two vectors, two angles can be computed between them.
	Only enough information is able to determine these angles, not which one
	is necessarily desired. Therefore, the smallest of these two angles is
	always the one that is returned.

	The order of arguments is not significant, so :python:`vang(lhs, rhs)
	== vang(rhs, lhs)`.

	:param EVector lhs: left-hand side of the operation
	:param EVector rhs: right-hand side of the operation
	:return: the angle between `lhs` and `rhs` in radians
	:rtype: float

.. function:: vxcl(vector, exclude)

	Excludes all portions of one vector from another. This results in a linearly
	independent vector to `exclude`.

	.. code-block:: python
		
		>>> ones = EVector((1, 1, 1))
		>>> excl = vxcl(ones, EVector.e1)
		>>> print(excl)
		[0, 1, 1]
		>>> dot(excl, EVector.e1)
		0.0
		>>> vector = EVector((3, 2, 1))
		>>> vector_excl = EVector((1.5, 0.75, 0))
		>>> excl2 = vxcl(vector, EVector.e2)
		>>> print(excl2)
		[-0.2, 0.4, 1]
		>>> dot(excl2, vector_excl)
		0.0

	This effectively produces the projection of `vector` onto the plane whose
	normal vector is `exculde`.

	:param EVector vector: vector to exclude from
	:param EVector exclude: vector to exclude from `vector`
	:return: `vector` with all portions of `exclude` removed from its direction
	:rtype: EVector

Matrix
^^^^^^

.. function:: det(matrix)

	Computes the determinate of a matrix. Useful for determining if a matrix is
	invertible i.e. the determinate is non-zero.

	:param EMatrix matrix: matrix to compute the determinate of
	:return: determinate of `matrix`
	:rtype: EMatrix

.. function:: transpose(matrix)

	Returns the transpose of `matrix`. The transpose of a matrix is simply a
	flipped matrix by switching its rows and columns. Therefore if T is the
	transpose of M, :python:`M[i, j] == T[j, i]`.

	:param EMatrix matrix: the matrix to transpose
	:return: the transpose of `matrix`
	:rtype: EMatrix