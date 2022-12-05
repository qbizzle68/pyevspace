.. include:: /global.rst

=====================
EVector API Reference
=====================

.. contents:: Table of Contents
	:backlinks: none

A Euclidean vector
------------------

A three-dimensional array object meant to represent a physical vector in 
a Euclidean vector space. The internal data buffer is a C type double, 
which supports :ref:`numeric <numeric-def>` types.

.. _numeric-def:

.. note::

	The underlying data type of the internal array is a C type
	:c:`double`. This obviously supports any type that can be represented
	as a Python :class:`float`, which includes Python :class:`int` types. 
	Therefore numeric types are interpreted in the following order:

		1. :class:`float`\ s are converted to a C :c:`double`
		2. A conversion is attempted with the type's :python:`__float__()` 
			method
		3. A conversion is attempted with the type's :python:`__index__()` 
			method
		4. No suitable conversion can be made and a TypeError is raised

	Therefore in this documentation a numeric type is any type that is
	either a :class:`float` or :class:`int` or can be converted to a
	:class:`float` or :class:`int`.

.. py:class:: EVector([initializer])

	Create an :class:`EVector` object with initial values from 
	`initializer`, or zero if `initializer` is omitted.

	:param iterable initializer: an iterable object of length three with
		:ref:`numeric <numeric-def>` values
	:raises TypeError: if `initializer` is not an iterable
	:raises TypeError: if a value of `initializer` is not a 
		:ref:`numeric <numeric-def>` type
	:raises ValueError: if `initializer` does not have a length of 
		exactly three

Attributes
----------

.. py:attribute:: EVector.e1

	Elementary vector that represents the x-axis of the standard
	basis. **This value should not be edited.**

	:value: :python:`EVector((1, 0, 0))`
	:type: EVector
		
.. py:attribute:: EVector.e2

	Elementary vector that represents the y-axis of the standard
	basis. **This value should not be edited.**

	:value: :python:`EVector((0, 1, 0))`
	:type: EVector

.. py:attribute:: EVector.e3

	Elementary vector that represents the z-axis of the standard
	basis. **This value should not be edited.**

	:value: :python:`EVector((0, 0, 1))`
	:type: EVector

Arithmetic Operators
--------------------

.. py:method:: EVector.__add__(other)

	Standard vector addition.

	:param EVector other: the vector to be added to self
	:return: the vector sum of self and `other`
	:rtype: EVector
	:raises TypeError: if `other` is not an :class:`EVector` type

.. py:method:: EVector.__sub__(other)

	Standard vector subtraction.

	:param EVector other: the vector to be subtracted from self
	:return: the vector subtraction of self and `other`
	:rtype: EVector
	:raises TypeError: if `other` is not an :class:`EVector` type

.. py:method:: EVector.__mul__(scalar)

	Standard scalar multiplication.

	:param scalar: the scalar to multiply each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:return: the scalar product
	:rtype: EVector
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>` type

.. py:method:: EVector.__truediv__(scalar)

	Standard scalar division.

	:param scalar: the scalar to divide each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:return: the scalar quotient
	:rtype: EVector
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>` type

.. py:method:: EVector.__neg__()

	Negates each component of self.

	:return: the negative of self
	:rtype: EVector

.. py:method:: EVector.__iadd__(other)
		
	Inplace standard vector addition.

	:param EVector other: the vector to add to self
	:raises TypeError: if `other` is not an :class:`EVector` type

.. py:method:: EVector.__isub__(other)

	Inplace standard vector subtraction.

	:param EVector other: the vector to subtract from self
	:raises TypeError: if `other` is not an :class:`EVector` type

.. py:method:: EVector.__imul__(scalar)

	Inplace standard scalar multiplication.

	:param scalar: the scalar to multiply each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: EVector.__itruediv__(scalar)

	Inplace standard scalar division.

	:param scalar: the scalar to divide each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: EVector.__str__()

	Returns a string representation of self. The format of the output
	string is similar to a :class:`list`, the components enclosed by
	square brackets. The format of the components are either decimal 
	or scientific notation, whichever requires fewer bytes to store 
	their string representations.

	:return: a string representation of self
	:rtype: str

.. py:method:: EVector.__reduce__()

	Allows support for pickling.

	:return: a tuple used for reconstructing self's state
	:rtype: tuple

Logical Operators
-----------------

.. py:method:: EVector.__eq__(other)

	Compares each element of two vectors for equality.

	:param EVector other: the vector to compare to self
	:return: True if each component of self and `other` are equivalent
	:rtype: bool
	:raises TypeError: if `other` is not an :class:`EVector` type

.. py:method:: EVector.__ne__(other)

	Compares each element of two vectors for an
	inequality.

	:param EVector other: the vector to compare to self
	:return: True if any component of self and `other` are not equivalent
	:rtype: bool
	:raises TypeError: if `other` is not an :class:`EVector` type

All other logic operators are not implemented and will raise a
:py:exc:`TypeError`.

Vector Operators
----------------

.. py:method:: EVector.mag()

	Computes the geometric length of the vector.

	:return: the magnitude of self
	:rtype: float

.. py:method:: EVector.mag2()
		
	Computes the square of the magnitude of the vector. Use this
	instead of squaring the result of :py:meth:`EVector.mag()` to
	avoid rounding errors.

	.. code-block:: python

		>>> vector = EVector((0.619286, 0.581799, 0.913961))
		>>> mag = vector.mag()
		>>> magSquared = vector.mag2()
		>>> magSquared == (mag * mag)
		False

	This yields the same result as :python:`dot(self, self)`. See also
	(py:func:`dot`).

	:return: the square of :py:meth:`self.mag()`
	:rtype: float

.. py:method:: EVector.normalize()
		
	Normalizes the vector by dividing each element by the magnitude
	of the vector. This results in a vector with lengh equal to one.

	:return: None

Sequence Protocol
-----------------

.. py:method:: EVector.__len__()

	Returns the length of the :class:`EVector`. This always returns
	three.

	:return: 3
	:rtype: int

.. py:method:: EVector.__getitem__(index)

	Retrieves the indexed value of the underlying array.

	:param int index: the index of the value to retrieve
	:return: the indexed value of self
	:rtype: float
	:raises TypeError: if `index` is not or cannot be converted to an
		int
	:raises ValueError: if `index` is not in the interval [0, 2]

.. py:method:: EVector.__setitem__(index, value)

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

	>>> vector = EVector((1, 2, 3))
	>>> len(vector)
	3
	>>> vector[0]
	1.0
	>>> vector[1] = 5
	>>> print(vector)
	[1, 5, 3]

Iterator Protocol
-----------------

The iterator protocol for the :class:`EVector` type is not managed
through a separate iterator type but within the class itself.

.. py:method:: EVector.__iter__()

	Returns a reference to self and sets an internal counter to 
	zero.

	:return: a reference to self
	:rtype: :class:`EVector`

.. py:method:: EVector.__next__()

	Returns the index of the internal counter before incrementing 
	the counter.

	:return: the next component of the :class:`EVector`
	:rtype: float
	:raises StopIteration: when the internal counter reaches three

Buffer Protocol
---------------

The :class:`EVector` class supports the buffer protocol and can be
used by other object which also support the buffer interface. For
example it can be used to instantiate a :py:class:`memoryview` object
	
.. code-block:: python
		
	>>> vector = EVector((1, 2, 3))
	>>> view = memoryview(vector)
	>>> view[2] = 3.14
	>>> print(vector)
	[1, 2, 3.14]
