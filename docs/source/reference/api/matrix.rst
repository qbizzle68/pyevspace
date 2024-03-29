.. include:: /global.rst

=====================
Matrix API Reference
=====================

.. currentmodule:: pyevspace

.. contents:: Table of Contents
    :backlinks: none

Constructor
-----------

.. py:class:: Matrix([initializer0, initializer1, initializer2])

    Create a :class:`Matrix` object to initialized values.

    Matrix can be initialized with optional initializer list, with each
    initializer representing a row of the matrix values. If using
    initializer list, all three arguments must be present. If no arguments
    are used, the initial values default to zero.

        .. code-block:: python

            >>> row0 = (1, 2, 3)
            >>> row1 = (4, 5, 6)
            >>> row2 = (7, 8, 9)
            >>> matrix = Matrix(row0, row1, row2)

    :param iterable initializer0: an iterable object of length three
        whose values are :ref:`numeric <numeric-def>` types (`initializer1`
        and `initializer2` must also be set)
    :param iterable initializer1: an iterable object of length three
        whose values are :ref:`numeric <numeric-def>` types (`initializer0`
        and `initializer2` must also be set)
    :param iterable initializer2: an iterable object of length three
        whose values are :ref:`numeric <numeric-def>` types (`initializer0`
        and `initializer1` must also be set)
    :raises TypeError: if any initializer is not an iterable
    :raises TypeError: if any value of an initializer is not a
        :ref:`numeric <numeric-def>` type
    :raises ValueError: if `initializer` or any of the sub-iterables of
        `initializer` are not exactly length three

Attributes
----------

.. py:attribute:: Matrix.id

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
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__sub__(other)

    Standard matrix subtraction of two matrices.

    :param Matrix other: the matrix to be subtracted from self
    :return: the matrix difference of self and `other`
    :rtype: Matrix
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__mul__(other)

    Standard multiplication depending on the type of `other`:

        - :ref:`numeric <numeric-def>` - standard scalar multiplication
        - :class:`Vector` - left-hand matrix multiplication of a vector
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
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__isub__(other)

    Inplace standard vector subtraction.

    :param Matrix other: the matrix to subtract from self
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__imul__(scalar)

    Inplace standard scalar multiplication.

    :param scalar: the scalar to multiply each element of self by
    :type scalar: :ref:`numeric <numeric-def>`
    :raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
        type

.. py:method:: Matrix.__itruediv__(scalar)

    Inplace standard scalar division.

    :param scalar: the scalar to divide each component of self by
    :type scalar: :ref:`numeric <numeric-def>`
    :raises TypeError: if `scalar` is not a :ref:`numeric <numeric-def>`
        type

.. py:method:: Matrix..__repr__()

    Returns a string representation of self, representative of a constructor
    call with the component values of the matrix. The format fo the components
    follows the same as :py:meth:`Matrix.__str__()`.

    :return: a string representation of self
    :rtype: str

.. py:method:: Matrix.__str__()

    Returns a string representation of self, formatted similarly to a
    list of lists.

    :return: a string representation of self
    :rtype: str

.. py:method:: Matrix.__reduce__()

    Allows support for pickling.

    :return: a tuple used for reconstructing self's state
    :rtype: tuple

Module Methods
--------------

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

Logical Operators
-----------------

.. py:method:: Matrix.__eq__(other)

    Compares each component of two matrices for equality.

    :param Matrix other: The matrix to compare to self
    :return: True if each component of `other` and self are equivalent
    :rtype: bool
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__ne__(other)

    Compares each component of two matrices for an inequality.

    :return: True if any component of `other` and self are not equivalent
    :rtype: bool
    :raises TypeError: if `other` is not a :class:`Matrix` type

All other logic operators are not implemented and will raise a
:py:exc:`TypeError`.

Mapping Protocol
----------------

The mapping protocol allows for accessing and mutating components of the
matrix. This allows for the square bracket notation, similar to the
sequence protocol of a :py:class:`Vector`. The indices are the row and
column of the component you wish to access. A :py:exc:`TypeError` is
raised if either index is not an integer, and an :py:exc:`IndexError` is
raised if either index is not in the interval [0, 2].

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

A single argument using square bracket notation allows access to a row
of a :py:class:`Matrix`. Because the matrix type is basically a data
array, there is no Python type that represents a row of the matrix type.
The closest thing would be a :py:class:`memoryview` object, which gives
access to the memory buffer of the desired row of the matrix.

.. code-block:: python

    >>> matrix = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    >>> row1 = matrix[1]
    >>> row1List = row1.tolist()
    >>> row1List
    [4.0, 5.0, 6.0]
    >>> row1[1] = 12.3
    >>> print(matrix)
    [[1, 2, 3]
    [4, 12.3, 6]
    [7, 8, 9]]

While this provides a way of quickly accessing an entire row of data,
it's not the most efficient way to change a value of a matrix since a
buffer object has to be created first. The ordinary mapping protocol
should be used in this case:

.. code-block:: python

    >>> matrix = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    >>> # faster
    >>> matrix[1, 1] = 10
    >>> # slower
    >>> matrix[1][1] = 10

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