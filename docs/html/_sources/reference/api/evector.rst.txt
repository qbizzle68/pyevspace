.. include:: /global.rst

=====================
EVector API Reference
=====================

.. contents:: Table of Contents
	:local:

A Euclidean vector
==================

A three-dimensional array object meant to represent a physical vector in 
a Euclidean vector space. The internal data buffer is a C type double, 
which supports Python :class:`float` and :class:`int` types. If the data
type is not a Python :class:`float`, the :class:`EVector` falls back to 
the :func:`__float__` and then :func:`__index__` methods if implemented.

.. py:class:: EVector([initializer])

	Create an :class:`EVector` object with initial values from 
	initializer, or initialize to zero if omitted.

	:param iterable initializer: an iterable object of length three with
		numeric values
	:raises TypeError: if initializer is not an iterable
	:raises TypeError: if a value of initializer is not a numeric type
	:raises ValueError: if initializer is not exactly length three

Attributes:
-----------

.. py:attribute:: EVector.e1

	Elementary vector that represents the x-axis of the standard
	basis. This value should not be edited.

	:value: :python:`EVector((1, 0, 0))`
	:type: EVector
		
.. py:attribute:: EVector.e2

	Elementary vector that represents the y-axis of the standard
	basis. This value should not be edited.

	:value: :python:`EVector((0, 1, 0))`
	:type: EVector

.. py:attribute:: EVector.e3

	Elementary vector that represents the z-axis of the standard
	basis. This value should not be edited.

	:value: :python:`EVector((0, 0, 1))`
	:type: EVector

Methods:
--------

Arithmetic
^^^^^^^^^^

.. py:method:: EVector.__add__(other)

	Standard vector addition of two :class:`EVector`\ s.

	:param EVector other: the vector to be added to self
	:return: the vector sum of self and other
	:rtype: EVector
	:raises TypeError: if other is not an :class:`EVector` type

.. py:method:: EVector.__sub__(other)

	Standard vector subtraction of two :class:`EVector`\ s.

	:param EVector other: the vector to be subtracted from self
	:return: the vector subtraction of self and other
	:rtype: EVector
	:raises TypeError: if other is not an :class:`EVector` type

.. py:method:: EVector.__mul__(scalar)

	Standard scalar multiplication.

	:param scalar: the scalar to multiply each element of self by
	:type scalar: float or int
	:return: the scalar product
	:rtype: EVector
	:raises TypeError: if scalar is not a float or int or cannot be
		converted to a float or int

.. py:method:: EVector.__truediv__(scalar)

	Standard scalar division.

	:param scalar: the scalar to divide each element of self by
	:type scalar: float or int
	:return: the scalar quotient
	:rtype: EVector
	:raises TypeError: if scalar is not a float or int or cannot be
		converted to a float or int

.. py:method:: EVector.__neg__()

	Negates each component of self.

	:return: the negative of self
	:rtype: EVector

.. py:method:: EVector.__iadd__(other)
		
	Inplace standard vector addition.

	:param EVector other: the vector to add to self
	:raises TypeError: if other is not an :class:`EVector` type

.. py:method:: EVector.__isub__(other)

	Inplace standard vector subtraction.

	:param EVector other: the vector to subtract from self
	:raises TypeError: if other is not an :class:`EVector` type

.. py:method:: EVector.__imul__(scalar)

	Inplace standard scalar multiplication.

	:param scalar: the scalar to multiply each element of self by
	:type scalar: float or int
	:raises TypeError: if scalar is not a float or int or cannot be 
		converted to a float or int

.. py:method:: EVector.__itruediv__(scalar)

	Inplace standard scalar division.

	:param scalar: the scalar to divide each element of self by
	:type scalar: float or int
	:raises TypeError: if scalar is not a float or int or cannot be
		converted to a float or int

.. py:method:: EVector.__str__()

	Returns a string representation of self. The format of the
	string is equivalent to f'[{self[0]}, {self[1]}, {self[2]}]'.

	:return: a string representation of self
	:rtype: str

.. py:method:: EVector.__reduce__()

	Allows support for pickling.

	:return: a tuple used for reconstructing self's state
	:rtype: tuple

Logical Operators
^^^^^^^^^^^^^^^^^

.. py:method:: EVector.__eq__(other)

	Compares each element of two vectors for equality.

	:param EVector other: The vector to compare to self
	:return: True if each component of other and self are equivalent
	:rtype: bool
	:raises TypeError: if other is not an :class:`EVector` type

.. py:method:: EVector.__ne__(other)

	Compares each element of two vectors for an
	inequality.

	:return: True if any component of other and self are not
		equivalent
	:rtype: bool
	:raises TypeError: if other is not an :class:`EVector` type

All other logic operators are not implemented and will raise a
:py:exc:`TypeError`.

Vector Operators
^^^^^^^^^^^^^^^^

.. py:method:: EVector.mag()

	Computes the geometric length of the vector.

	:return: the magnitude of self
	:rtype: float

.. py:method:: EVector.mag2()
		
	Computes the square of the magnitude of the vector. Use this
	instead of squaring the result of :py:meth:`EVector.mag()` to
	avoid rounding errors.

	:return: the square of :py:meth:`self.mag()`
	:rtype: float

.. py:method:: EVector.normalize()
		
	Normalizes the vector by dividing each element by the magnitude
	of the vector. This results in a vector with lengh equal to one.

	:return: None

Sequence Protocol
^^^^^^^^^^^^^^^^^

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
	:raises TypeError: if index is not or cannot be converted to an
		int
	:raises ValueError: if index is not in the interval [0, 2]

.. py:method:: EVector.__setitem__(index, value)

	Sets the indexed value of the underlying array.

	:param int index: the index of the value to set
	:param value: the value to set the array component to
	:type value: float or int
	:raises TypeError: if index is not or cannot be converted to an
		int
	:raises TypeError: if value is not a float or int or cannot be
		converted to a float or int
	:raises ValueError: if index is not in the interval [0, 2]

Iterator Protocol
^^^^^^^^^^^^^^^^^

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
^^^^^^^^^^^^^^^

The :class:`EVector` class supports the buffer protocol and can be
used by other object which also support the buffer interface. For
example it can be used to instantiate a :py:class:`memoryview` object
	
.. code-block:: python
		
	>>> vector = EVector((1, 2, 3))
	>>> view = memoryview(vector)
	>>> view[2] = 3.14
	>>> print(vector)
	[1, 2, 3.14]
