.. include:: /global.rst

=====================
Vector API Reference
=====================

.. currentmodule:: pyevspace

.. contents:: Table of Contents
	:backlinks: none

Constructor
-----------

.. py:class:: Vector([{x, y, z | initializer}])

	Create a :class:`Vector` object and initialize components.

	A vector can be constructed directly by setting the x, y and z components,
	from an iterable object of length three, or to zero if all arguments are omitted.
	If initializing the components directly all components must be set. If
	initializing from an iterable the iterable must be the only parameter.
	All components will be set to zero if all arguments are omitted.

	:param numeric x: value to initialize the x-component to (the `y` and `z`
		parameters must also be filled when initializing directly)
	:param numeric y: value to initialize the y-component to (the `x` and `z`
		parameters must also be filled when initializing directly)
	:param numeric z: value to initialize the z-component to (the `x` and `y`
		parameters must also be filled when initializing directly)
	:param iterable initializer: an iterable object of length three with
		:ref:`numeric <numeric-def>` values (this must be the only parameter when
		initializing from an iterable)
	:raises TypeError: if `x`, `y`, `z` or `initializer` is not an iterable
	:raises TypeError: if a value of `initializer` is not a
		:ref:`numeric <numeric-def>` type
	:raises TypeError: if exactly three direct initializers aren't used or iterable
		is not the only argument
	:raises ValueError: if `initializer` does not have a length of
		exactly three

Attributes
----------

.. py:attribute:: Vector.e1

	Elementary vector that represents the x-axis of the standard
	basis. **This value should not be edited.**

	:value: :python:`Vector(1, 0, 0)`
	:type: Vector

.. py:attribute:: Vector.e2

	Elementary vector that represents the y-axis of the standard
	basis. **This value should not be edited.**

	:value: :python:`Vector(0, 1, 0)`
	:type: Vector

.. py:attribute:: Vector.e3

	Elementary vector that represents the z-axis of the standard
	basis. **This value should not be edited.**

	:value: :python:`Vector(0, 0, 1)`
	:type: Vector

Arithmetic Operators
--------------------

.. py:method:: Vector.__add__(other)

	Standard vector addition.

	:param Vector other: the vector to be added to self
	:return: the vector sum of self and `other`
	:rtype: Vector
	:raises TypeError: if `other` is not an :class:`Vector` type

.. py:method:: Vector.__sub__(other)

	Standard vector subtraction.

	:param Vector other: the vector to be subtracted from self
	:return: the vector subtraction of self and `other`
	:rtype: Vector
	:raises TypeError: if `other` is not an :class:`Vector` type

.. py:method:: Vector.__mul__(scalar)

	Standard scalar multiplication.

	:param scalar: the scalar to multiply each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:return: the scalar product
	:rtype: Vector
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>` type

.. py:method:: Vector.__matmul__(matrix)

	Multiplication with the vector on the left side.

	:param Matrix matrix: the matrix to multiply self by
	:return: the transformation of self by matrix
	:rtype: Vector
	:raises TypeError: if `matrix` is not a Matrix type

.. py:method:: Vector.__truediv__(scalar)

	Standard scalar division.

	:param scalar: the scalar to divide each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:return: the scalar quotient
	:rtype: Vector
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>` type

.. py:method:: Vector.__neg__()

	Negates each component of a vector.

	:return: the negative of a vector
	:rtype: Vector

.. py:method:: Vector.__iadd__(other)

	Inplace standard vector addition.

	:param Vector other: the vector to add to self
	:raises TypeError: if `other` is not an :class:`Vector` type

.. py:method:: Vector.__isub__(other)

	Inplace standard vector subtraction.

	:param Vector other: the vector to subtract from self
	:raises TypeError: if `other` is not an :class:`Vector` type

.. py:method:: Vector.__imul__(scalar)

	Inplace standard scalar multiplication.

	:param scalar: the scalar to multiply each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: Vector.__itruediv__(scalar)

	Inplace standard scalar division.

	:param scalar: the scalar to divide each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: Vector.__repr__()

	Returns a string representation of self, representative of a constructor
	call with the component values of the vector. The format fo the components
	follows the same as :py:meth:`Vector.__str__()`.

	:return: a string representation of self
	:rtype: str

.. py:method:: Vector.__str__()

	Returns a string representation of self. The format of the output
	string is similar to a :class:`list`, the components enclosed by
	square brackets. The format of the components are either decimal
	or scientific notation, whichever requires fewer bytes to store
	their string representations.

	:return: a string representation of self
	:rtype: str

.. py:method:: Vector.__reduce__()

	Allows support for pickling.

	:return: a tuple used for reconstructing self's state
	:rtype: tuple

Module Methods
--------------

.. function:: dot(lhs, rhs)

	Computes the dot product between two vectors. The order of arguments
	is not significant as :python:`dot(v1, v2) == dot(v2, v1)`.

	:param Vector lhs: left-hand side of the operation
	:param Vector rhs: right-hand side of the operation
	:return: the dot product lhs and rhs
	:rtype: float
	:raises TypeError: if `lhs` or `rhs` is not Vector type

.. function:: cross(lhs, rhs)

	Computes the cross product between two vectors. A left-handed
	coordinate frame is used. If a right-handed cross product is needed,
	simply negate the answer.

	.. code-block:: python

		>>> left_cross = cross(Vector.e1, Vector.e2)
		>>> print(left_cross)
		[0, 0, 1]
		>>> right_cross = -cross(Vector.e1, Vector.e2)
		>>> print(right_cross)
		[0, 0, -1]

	:param Vector lhs: left-hand side of the operation
	:param Vector rhs: right-hand side of the operation
	:return: the cross product of lhs and rhs
	:rtype: Vector
	:raises TypeError: if `lhs` or `rhs` is not Vector type

.. function:: norm(vector)

	Returns a normalized version of `vector`. This creates a new vector, as
	opposed to the 'inplace' normalize method provided by the Vector class.
	See (:py:meth:`Vector.normalize`).

	.. code-block:: python

		>>> vector1 = Vector((1, 2, 3))
		>>> vector2 = norm(vector1)
		>>> vector1.normalize()
		>>> print(vector1)
		[0.267261, 0.534522, 0.801784]
		>>> print(vector2)
		[0.267261, 0.534522, 0.801784]

	:param Vector vector: the vector to find the normal of
	:return: the normalized version of `vector`
	:rtype: Vector
	:raises TypeError: if `vector` is not Vector type

.. function:: vang(lhs, rhs)

	Computes the shortest angle between two vectors. This relies on the
	definition of the vector dot product to compute. Since
	v1 * v2 = \|v1\|*\|v2\|*cos(theta), the dot product of v1 and v2,
	and the magnitude of v1 and v2 is all we need to compute the angle
	between the two vectors.

	.. code-block:: python

		>>> lhs = Vector((1, 2, 3))
		>>> rhs = Vector((4, 2, -3))
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

	:param Vector lhs: left-hand side of the operation
	:param Vector rhs: right-hand side of the operation
	:return: the angle between `lhs` and `rhs` in radians
	:rtype: float

.. function:: vxcl(vector, exclude)

	Excludes all portions of one vector from another. This results in a linearly
	independent vector to `exclude`.

	.. code-block:: python

		>>> ones = Vector((1, 1, 1))
		>>> excl = vxcl(ones, Vector.e1)
		>>> print(excl)
		[0, 1, 1]
		>>> dot(excl, Vector.e1)
		0.0
		>>> vector = Vector((3, 2, 1))
		>>> vector_excl = Vector((1.5, 0.75, 0))
		>>> excl2 = vxcl(vector, Vector.e2)
		>>> print(excl2)
		[-0.2, 0.4, 1]
		>>> dot(excl2, vector_excl)
		0.0

	This effectively produces the projection of `vector` onto the plane whose
	normal vector is `exclude`.

	:param Vector vector: vector to exclude from
	:param Vector exclude: vector to exclude from `vector`
	:return: `vector` with all portions of `exclude` removed from its direction
	:rtype: Vector

.. function:: proj(vector, onto)

	Projects one vector onto another.

	:param Vector vector: vector to project
	:param Vector onto: vector to project `vector` onto
	:return: `vector` projected onto `onto`
	:rtype: Vector

Logical Operators
-----------------

.. py:method:: Vector.__eq__(other)

	Compares each element of two vectors for equality.

	:param Vector other: the vector to compare to self
	:return: True if each component of self and `other` are equivalent
	:rtype: bool
	:raises TypeError: if `other` is not an :class:`Vector` type

.. py:method:: Vector.__ne__(other)

	Compares each element of two vectors for an
	inequality.

	:param Vector other: the vector to compare to self
	:return: True if any component of self and `other` are not equivalent
	:rtype: bool
	:raises TypeError: if `other` is not an :class:`Vector` type

All other logic operators are not implemented and will raise a
:py:exc:`TypeError`.

Vector Operators
----------------

.. py:method:: Vector.mag()

	Computes the geometric length of the vector.

	:return: the magnitude of self
	:rtype: float

.. py:method:: Vector.mag2()

	Computes the square of the magnitude of the vector. Use this
	instead of squaring the result of :py:meth:`Vector.mag()` to
	avoid rounding errors.

	.. code-block:: python

		>>> vector = Vector((0.619286, 0.581799, 0.913961))
		>>> mag = vector.mag()
		>>> magSquared = vector.mag2()
		>>> magSquared == (mag * mag)
		False

	This yields the same result as :python:`dot(self, self)`. See also
	(py:func:`dot`).

	:return: the square of :py:meth:`self.mag()`
	:rtype: float

.. py:method:: Vector.normalize()

	Normalizes the vector by dividing each element by the magnitude
	of the vector. This results in a vector with length equal to one.

	:return: None

Sequence Protocol
-----------------

.. py:method:: Vector.__len__()

	Returns the length of the :class:`Vector`. This always returns
	three.

	:return: 3
	:rtype: int

.. py:method:: Vector.__getitem__(index)

	Retrieves the indexed value of the underlying array.

	:param int index: the index of the value to retrieve
	:return: the indexed value of self
	:rtype: float
	:raises TypeError: if `index` is not or cannot be converted to an
		int
	:raises ValueError: if `index` is not in the interval [0, 2]

.. py:method:: Vector.__setitem__(index, value)

	Sets the indexed value of the underlying array.

	:param int index: the index of the value to set
	:param value: the value to set the array component to
	:type value: float or int
	:raises TypeError: if `index` is not or cannot be converted to an
		int
	:raises TypeError: if `value` is not a :ref:`numeric <numeric-def>`
		type
	:raises ValueError: if `index` is not in the interval [0, 2]

.. code-block:: python

	>>> vector = Vector((1, 2, 3))
	>>> len(vector)
	3
	>>> vector[0]
	1.0
	>>> vector[1] = 5
	>>> print(vector)
	[1, 5, 3]

Iterator Protocol
-----------------

.. py:method:: Vector.__iter__()

	Returns an iterator object for the vector.

	:return: an iterator for a vector
	:rtype: :class:`Vector`

Buffer Protocol
---------------

The :class:`Vector` class supports the buffer protocol and can be
used by other object which also support the buffer interface. For
example it can be used to instantiate a :py:class:`memoryview` object

.. code-block:: python

	>>> vector = Vector((1, 2, 3))
	>>> view = memoryview(vector)
	>>> view[2] = 3.14
	>>> print(vector)
	[1, 2, 3.14]
