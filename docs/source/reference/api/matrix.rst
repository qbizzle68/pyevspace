.. include:: /global.rst

=====================
Matrix API Reference
=====================

.. currentmodule:: pyevspace

.. contents:: Table of Contents
	:backlinks: none

Constructor
-----------

.. py:class:: Matrix([initializer])

	Create an :class:`Matrix` object with initial values from
	`initializer`, or zero if `initializer` is omitted.

	:param iterable initializer: an iterable object of length three
		whose values are iterable objects of length three with
		:ref:`numeric <numeric-def>` values

		.. code-block:: python

			>>> initializer = ((1, 2, 3), (4, 5, 6), (7, 8, 9))
			>>> matrix = Matrix(initializer)

	:raises TypeError: if `initializer` or any value of `initializer` is
		not an iterable
	:raises TypeError: if any value of a subiterable is not a
		:ref:`numeric <numeric-def>` type
	:raises ValueError: if `initializer` or any of the subiterables of
		`initializer` are not exactly length three

Attributes
----------

.. py:attribute:: Matrix.I

	Identity matrix. **This value should not be edited.**

	:value: :python:`Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1))`
	:type: Matrix

Arithmetic Operators
--------------------

.. py:method:: Matrix.__add__(other)

	Standard matrix addition of two matrices.

	:param Matrix other: the matrix to be added to self
	:return: the matrix sum of self and `other`
	:rtype: Matrix
	:raises TypeError: if `other` is not an :class:`Matrix` type

.. py:method:: Matrix.__sub__(other)

	Standard matrix subtraction of two matrices.

	:param Matrix other: the matrix to be subtracted from self
	:return: the matrix difference of self and `other`
	:rtype: Matrix
	:raises TypeError: if `other` is not an :class:`Matrix` type

.. py:method:: Matrix.__mul__(other)

	Standard multiplication depending on the type of `other`:

		- :ref:`numeric <numeric-def>` - standard scalar multiplication
		- :class:`EVector` - left-hand matrix multiplication of a vector
		- :class:`Matrix` - matrix multiplication of a matrix

	:param other: the object to multiply self by
	:type other: :ref:`numeric <numeric-def>` | EVector | Matrix
	:return: the product of self and `other`
	:rtype: EVector or Matrix
	:raises TypeError: if other is not a :ref:`numeric <numeric-def>`,
		:class:`EVector` or :class:`Matrix` type

.. py:method:: Matrix.__truediv__(scalar)

	Standard scalar division.

	:param scalar: the scalar to divide each component of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:return: the scalar quotient
	:rtype: Matrix
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: Matrix.__neg__()

	Negates each component of self.

	:return: the negative of self
	:rtype: Matrix

.. py:method:: Matrix.__iadd__(other)

	Inplace standard vector addition.

	:param Matrix other: the matrix to add to self
	:raises TypeError: if `other` is not an :class:`Matrix` type

.. py:method:: Matrix.__isub__(other)

	Inplace standard vector subtraction.

	:param Matrix other: the matrix to subtract from self
	:raises TypeError: if `other` is not an :class:`Matrix` type

.. py:method:: Matrix.__imul__(scalar)

	Inplace standard scalar multiplication.

	:param scalar: the scalar to multilpy each element of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: Matrix.__itruediv__(scalar)

	Inplace standard scalar division.

	:param scalar: the scalar to divide each component of self by
	:type scalar: :ref:`numeric <numeric-def>`
	:raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
		type

.. py:method:: Matrix.__str__()

	Returns a string representation of self, formatted similarly to a
	list of lists.

	:return: a string representation of self
	:rtype: str

.. py:method:: Matrix.__reduce__()

	Allows support for pickling.

	:return: a tuple used for reconstructing self's state
	:rtype: tuple

Logical Operators
-----------------

.. py:method:: Matrix.__eq__(other)

	Compares each component of two matrices for equality.

	:param Matrix other: The matrix to compare to self
	:return: True if each component of `other` and self are equivalent
	:rtype: bool
	:raises TypeError: if `other` is not an :class:`Matrix` type

.. py:method:: Matrix.__ne__(other)

	Compares each component of two matrices for an inequality.

	:return: True if any component of `other` and self are not equivalent
	:rtype: bool
	:raises TypeError: if `other` is not an :class:`Matrix` type

All other logic operators are not implemented and will raise a
:py:exc:`TypeError`.

Mapping Protocol
----------------

The mapping protocol allows for accessing and mutating components of the
matrix. This allows for the square bracket notation, similar to the
sequence protocol. The indeces are the row and column of the component
you wish to access. A :py:exc:`TypeError` is raised if either index is
not an integer, and a :py:exc:`IndexError` is raised if either index is
not in the interval [0, 2].

.. code-block:: python

	>>> matrix = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
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
---------------

The :class:`Matrix` class supports the buffer protocol and can be used
by other objects which also support the buffer interface. For example
it can be used to instantiate a :py:class:`memoryview` object

.. code-block:: python

	>>> matrix = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> view = memoryview(matrix)
	>>> view[1, 1] = 1.69
	>>> print(matrix)
	[[1, 2, 3]
	[4, 1.69, 6]
	[7, 8, 9]]

as well as other third-party numeric libraries that support buffers like
NumPy.

.. code-block:: python

	>>> matrix = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> array = np.ndarray((3, 3), buffer=matrix)
	>>> array[0, 1] = 3.14
	>>> print(matrix)
	[[1, 3.14, 3]
	[4, 5, 6]
	[7, 8, 9]]