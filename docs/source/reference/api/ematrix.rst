.. include:: /global.rst

=====================
EMatrix API Reference
=====================

.. contents:: Table of Contents
	:local:

A Euclidean matrix
===============================================

A three by three-dimensional array object mean to represent a linear
transformation of physical vectors in a Euclidean vector space. The
internal data buffer is a C type :c:`double`, which supports numeric types.

.. _numeric-def:

.. note::

	The underlying data type of the internal array is a C type
	:c:`double`. This obviously supports any type that can be
	represented as a Python :class:`float`, which also includes Python
	:class:`int` types. The way this works is as follows:

		1. :class:`float`\ s are converted to a C :c:`double`
		2. A conversion is attempted with the type's :python:`__float__()` 
			method
		3. A conversion is attempted with the type's :python:`__index__()` 
			method
		4. No suitable conversion can be made and a TypeError is raised

	Therefore in this documentation a numeric type is any type that is
	either a :class:`float` or :class:`int` or can be converted to a
	:class:`float` or :class:`int`.

.. py:class:: EMatrix([initializer])

	Create an :class:`EMatrix` object with initial values from
	initializer, or initialize to zero if omitted.

	:param iterable initializer: an iterable object of length three
		whose values are iterable objects of length three with numeric
		values
	:raises TypeError: if initializer or any value of initializer is
		not an iterable
	:raises TypeError: if any value of a subiterable is not a numeric
		type
	:raises ValueError: if initializer or any of the subiterables of
		initializer are not exactly length three

Attributes:
-----------

.. py:attribute:: EMatrix.I

	Identity matrix. This value should not be edited.

	:value: :python:`EMatrix((1, 0, 0), (0, 1, 0), (0, 0, 1))`
	:type: EMatrix

Methods:
--------

Arithmetic
^^^^^^^^^^

.. py:method:: EMatrix.__add__(other)

	Standard matrix addition of two :class:`EMatrix`\ s.

	:param EMatrix other: the matrix to be added to self
	:return: the matrix sum of self and other
	:rtype: EMatrix
	:raises TypeError: if other is not an :class:`EMatrix` type

.. py:method:: EMatrix.__sub__(other)

	Standard matrix subtraction of two :class:`EMatrix`\ s.

	:param EMatrix other: the matrix to be subtracted from self
	:return: the matrix difference of self and other
	:rtype: EMatrix
	:raises TypeError: if other is not an :class:`EMatrix` type

.. py:method:: EMatrix.__mul__(other)

	Standard multiplication depending on the type of `other`:

		- :ref:`numeric <numeric-def>`: standard scalar multiplication
		- :class:`EVector`: vector multiplication
		- :class:`EMatrix`: matrix multiplication

	:param other: the object to multiply self by
	:type other: :ref:`numeric <numeric-def>` or EVector or EMatrix
	:return: the product of self and other
	:rtype: EVector or EMatrix
	:raises TypeError: if other is not a :ref:`numeric <numeric-def>`, 
		:class:`EVector` or :class:`EMatrix` type

.. py:method:: EMatrix.__truediv__(scalar)

	Standard scalar division.

	:param scalar: the scalar to divide each
		component of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:return: the scalar quotient
	:rtype: EMatrix
	:raises TypeError: if scalar is not a :ref:`numeric <numeric-def>` 
		type

.. py:method:: EMatrix.__neg__()

	Negates each component of self.

	:return: the negative of self
	:rtype: EMatrix

.. py:method:: EMatrix.__iadd__(other)

	Inplace standard vector addition.

	:param EMatrix other: the matrix to add to self
	:raises TypeError: if other is not an :class:`EMatrix` type

.. py:method:: EMatrix.__isub__(other)

	Inplace standard vector subtraction.

	:param EMatrix other: the matrix to subtract from self
	:raises TypeError: if other is not an :class:`EMatrix` type

.. py:method:: EMatrix.__imul__(scalar)

	Inplace standard scalar multiplication.

	:param scalar: the scalar to multilpy each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:raises TypeError: if scalar is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: EMatrix.__itruediv__(scalar)

	Inplace standard scalar division.

	:param scalar: the scalar to divide each component of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:raises TypeError: if scalar is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: EMatrix.__str__()

	Returns a string representation of self, formatted similarly to a
	list of lists.

	:return: a string representation of self
	:rtype: str

.. py:method:: EMatrix.__reduce__()

	Allows support for pickling.

	:return: a tuple used for reconstructing self's state
	:rtype: tuple

Logical Operators
^^^^^^^^^^^^^^^^^

.. py:method:: EMatrix.__eq__(other)

	Compares each component of two matrices for equality.

	:param EMatrix other: The matrix to compare to self
	:return: True if each component of other and self are equivalent
	:rtype: bool
	:raises TypeError: if other is not an :class:`EMatrix` type

.. py:method:: EMatrix.__ne__(other)

	Compares each component of two matrices for an inequality.

	:return: True if any component of other and self are not equivalent
	:rtype: bool
	:raises TypeError: if other is not an :class:`EMatrix` type

All other logic operators are not implemented and will raise a 
:py:exc:`TypeError`.

Mapping Protocol
^^^^^^^^^^^^^^^^

The mapping protocol allows for accessing and mutating components of the
matrix. This allows for the square bracket notation, similar to the
sequence protocol. The indeces are the row and column of the component
you wish to access. A py:exc:`TypeError` is raised if either index is
not an integer, and a py:exc:`IndexError` is raised if either index is
not in the interval [0, 2].

.. code-block:: python
	
	>>> matrix = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> matrix[1, 1]
	5.0
	>>> matrix[0, 2] = 11
	>>> print(matrix)
	[[1, 2, 11]
	[4, 5, 6]
	[7, 8, 9]]
	>>> matrix[2, 3]
	Traceback (most recent call last):
	  File "<stdin>", line 1, in <module>
	IndexError: col index (3) must be in [0-2]
	>>> matrix[1, 1.0]
	Traceback (most recent call last):
	  File "<stdin>", line 1, in <module>
	TypeError: 'float' object cannot be interpreted as an integer

Buffer Protocol
^^^^^^^^^^^^^^^

The :class:`EMatrix` class supports the buffer protocol and can be used
by other objects which also support the buffer interface. For example
it can be used to instantiate a :py:class:`memoryview` object

.. code-block:: python

	>>> matrix = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> view = memoryview(matrix)
	>>> view[1, 1] = 1.69
	>>> print(matrix)
	[[1, 2, 3]
	[4, 1.69, 6]
	[7, 8, 9]]
