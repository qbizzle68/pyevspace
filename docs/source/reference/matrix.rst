.. include:: /global.rst

====================
Matrix API Reference
====================

.. currentmodule:: pyevspace

The Matrix Type
===============

.. py:class:: Matrix()
    Matrix(row1: Iterable, row2: Iterable, row3: Iterable)
    Matrix(container: Iterable)

    The Matrix type represents a matrix of a 3-dimensional vector space
    over the real numbers. Matrices represent linear transformations
    between vector spaces. In PyEVSpace, each reference frame can be thought
    of as a unique vector space, and matrices transform vectors between them.
    For users comfortable working with matrices, the Matrix type overloads
    most of the arithmetic operators and, provides several methods useful
    for working with matrices such as computing determinates, inverses, and
    transposing. For those not familiar with matrices, PyEVSpace has several
    functions for handling rotations for you, and it's possible to leverage
    full use of the library without even handling matrices directly!

    A matrix can be initialized by specifying each row directly, or wrapped
    in a container. All containers must have a length of exactly `3`, where
    the row containers have components that are compatible with
    :class:`SupportsFloat`, meaning they can be converted to a :class:`float` by
    defining a :meth:`__float__` method.

    If no arguments are provided the matrix components are initialized
    to zero.

    .. code-block:: python

        >>> row0 = (1, 2, 3)
        >>> row1 = (4, 5, 6)
        >>> row2 = (7, 8, 9)
        >>> matrix = Matrix(row0, row1, row2)
        >>> # or via a top-level container
        >>> container = (row0, row1, row2)
        >>> matrix = Matrix(container)

    .. versionchanged:: 0.16.0
        Matrix is now subclassable, and is no longer an immutable type,
        meaning class members can be directly modified or added.

    :param row0: an iterable object of length `3` whose values are or can be
        converted to a :class:`float`
    :param row1: an iterable object of length `3` whose values are or can be
        converted to a :class:`float`
    :param row2: an iterable object of length `3` whose values are or can be
        converted to a :class:`float`
    :param container: a container of length `3` whose values would
        otherwise be identical to `row0`, `row1`, and `row2`. If this
        parameter is provided it must be the only one.
    :raises TypeError: if any initializer is not an iterable
    :raises TypeError: if any value of an initializer is not or cannot be
        converted to a :class:`float`
    :raises ValueError: if `container` or any of the sub-iterables of
        `container` are not exactly length `3`

Other Constructors
------------------

.. py:classmethod:: Matrix.__new__(cls: type) -> Self

    Create a new, uninitialized `cls` object if `cls` is a subclass of
    :class:`Matrix`. All components of the matrix default to zero.

    :param cls: must be a subtype of :class:`Matrix`
    :return: an uninitialized :class:`Matrix` object
    :raises TypeError: if `cls` is not a subtype of :class:`Matrix`

    .. versionchanged:: 0.16.0
        `type` can be a subtype of :class:`Matrix`, and an object of that
        type will be created and returned.

.. py:method:: Matrix.__init__()
    Matrix.__init__(row0: Iterable, row1: Iterable, row2: Iterable)
    Matrix.__init__(container: Iterable)

    Initialize a :class:`Matrix` depending on the parameters. This method and
    parameters behave identrically to the :class:`Matrix` constructor.

    :param row0: an iterable object of length `3` whose values are or can be
        converted to a :class:`float`
    :param row1: an iterable object of length `3` whose values are or can be
        converted to a :class:`float`
    :param row2: an iterable object of length `3` whose values are or can be
        converted to a :class:`float`
    :param container: a container of length `3` whose values would
        otherwise be identical to `row0`, `row1`, and `row2`. If this
        parameter is provided it must be the only one.
    :raises TypeError: if any initializer is not an iterable
    :raises TypeError: if any value of an initializer is not or cannot be
        converted to a :class:`float`
    :raises ValueError: if `container` or any of the sub-iterables of
        `container` are not exactly length `3`

Subclassing :class:`Matrix`
===========================
As of version `0.16.0` the :class:`Matrix` class supports subclassing. As the
:class:`Matrix` class is implemented as a C struct, it has a specific memory
layout, and as a consequence of that it is a :deco:`distjoint-base`. This
means :class:`Matrix` cannot be combined with any other types whose layout
is defined as a C struct when inheriting, which most Python built-in types
such as :class:`list` or :class:`tuple` are. For example combining :class:`Matrix`
and :class:`list` as base classes will raise a :exc:`TypeError`:

.. code-block:: python

	>>> class Foo(Matrix, list):
	...     pass
	...
	TypeError: multiple bases have instance lay-out conflict

This limitation does not apply when combining a :deco:`distjoin-base` with a
pure python class, and of course is not an issue if :class:`Matrix` is the only
base class for a derived type.

While :class:`Matrix` supports inheritance, there is no way for the
constructor to know how to initialize an inherited type. Because of this,
any function (except :meth:`Matrix.__new__`) that returns a :class:`Matrix`
type will always return a :class:`Matrix` instance, even on inherited types.
To change this, you will need to override the function you want to return your
type, and wrap the returned value in the constructor of your type. For example:

.. code-block:: python

	>>> class MyType(Matrix):
	...     def __init__(self, container, foo, bar):
	...         super().__init__(self, container)
	...         self.foo = foo
	...         self.bar = bar
	...     def __add__(self, other: MyType) -> MyType:
	...         super().__add__(other)
	...         return MyType(result, self.foo, self.bar)

In this example the result of the :meth:`Matrix.__add__` call is used to
create a :class:`MyType` instance. Any method you want to have this behavior must
be implemented yourself, including module level functions that normally
return a :class:`Matrix` type.

Instance Methods
================

General Protocols
-----------------

.. py:method:: Matrix.__len__() -> int

    Returns the length of the matrix object. This always returns `3`.

    :return: the method always returns `3`

.. py:method:: Matrix.__getitem__(row_index: int | slice, col_index: int | slice) -> float | memoryview

    Retrieves a portion of the matrix using the mapping protocol. If `row_index` and
    `col_index` are of type :class:`int` (or provide an :meth:`__index__` method)
    the value at that index is returned. If either (or both) indices are a
    :class:`slice` type, a :class:`memoryview` object is returned with attributes
    that map to the specified indices.

    :param row_index: the row(s) of the matrix to retrieve
    :param col_index: the column(s) of the matrix to retrieve
    :return: either a component of the matrix or a view of the indexed memory
    :raises TypeError: if `row_index` or `col_index` are not :class:`int` or
        :class:`slice` types
    :raises IndexError: if `row_index` or `col_index` are integers outside the
        range [0-2] (after adjusting for negative indexing)

.. py:method:: Matrix.__setitem__(row_index: int, col_index: int, value: SupportsFloat) -> float

    Sets a components of the matrix to `value`. This version of PyEVSpace currently
    only supports map assignment of single components, and therefore :class:`slice`
    arguments will raise a :exc:`TypeError`.

    :param row_index: the row of the component to set
    :param col_index: the col of the component to set
    :param value: the value to assign the component at (row, col)
    :return: `value`
    :raise TypeError: if `row_index` or `col_index` are not :class:`int` types
        and do not provide :meth:`__index__`
    :raise TypeError: if `value` is not or cannot be converted to a :class:`float`
    :raise IndexError: if `row_index` or `col_index` are not in the range [0-2]
        (after adjusting for negative indexing)

.. py:method:: Matrix..__repr__() -> str

    Returns a string representation of `self`, representative of a constructor
    call with the component values of the matrix. The format of the components
    follows the same as :meth:`Matrix.__str__()`.

    :return: a string representation of `self`

    .. versionchanged:: 0.16.0
        The method now prepends the output with the module name for better
        scope resolution support when using the output with :func:`exec`.

.. py:method:: Matrix.__str__() -> str

    Returns a string representation of self, formatted similarly to a
    :class:`list` of :class:`list` s. The format of the components are
    either decimal of scientific notation, whichever requires fewer
    bytes to store their string representation.

    :return: a string representation of `self`

    .. versionchanged:: 0.16.0
        The last two rows are padded for left alignment.

.. py:method:: Matrix.__reduce__()

    Allows support for pickling and deep copying. This function returns a
    2-tuple containing a reference to the type constructor and a tuple of
    arguments to initialize an equivalent instance. For types inheriting from
    :class:`Matrix`, this function may beed to be overloaded and modified
    to return your own type.

    :return: a :class:`tuple` used for reconstructing self's state
    :rtype: tuple[type, tuple[tulpe[float, ...], ...]]

Iterability
-----------

.. versionadded:: 0.16.0
    :class:`Matrix` now supports iterability.

While the :class:`Matrix` class does not define an :meth:`__iter__` method
directly, the class does support iterability via the sequence protocol. Attempting
to access :meth:`Matrix.__iter__` will raise an :exc:`AttributeError`, however
a :class:`Matrix` can be used anywhere an iterable is expected. Note that when
iterating on a :class:`Matrix`, a :class:`memoryview` object is returned, which
itself is iterable. When trying to convert a :class:`Matrix` to a different container
type, each row of the matrix will also need converting. The :func:`map` function is
useful for this:

.. code-block:: python

    >>> m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    >>> container = list(map(lambda o: list(o), m))
    [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [7.0, 8.0, 9.0]]

Arithmetic Operators
--------------------

.. py:method:: Matrix.__add__(other: Matrix) -> Matrix

    Standard matrix addition of two matrices.

    :param other: the matrix to be added to `self`
    :return: the matrix sum of `self` and `other`
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__sub__(other: Matrix) -> Matrix

    Standard matrix subtraction of two matrices.

    :param other: the matrix to be subtracted from `self`
    :return: the matrix difference of `self` and `other`
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__mul__(scalar: SupportsFloat) -> Matrix

    Scalar matrix multiplication.

    :param scalar: the scalar to multiply the components of `self` by
    :return: the scalar product of `self` and `scalar`
    :raises TypeError: if `scalar` is not or cannot be converted to a :class:`float`

.. py:method:: Matrix.__matmul__(other: Matrix) -> Matrix
    Matrix.__matmul__(vector: Vector) -> Vector

    Matrix-matrix or matrix-vector multiplcation.

    :param other: the matrix to multiply `self` by
    :param vector: the vector to multiply `self` by
    :return: the result of the multiplication, the return type is the
        same type as the right-hand operand

.. py:method:: Matrix.__truediv__(scalar: SupportsFloat) -> Matrix

    Standard scalar division.

    :param scalar: the scalar to divide each component of `self` by
    :return: the scalar quotient
    :raises TypeError: if `scalar` is not or cannot be converted to a :class:`float`

.. py:method:: Matrix.__neg__() -> Matrix

    Negates each component of `self`.

    :return: the negative of `self`

.. py:method:: Matrix.__iadd__(other: Matrix) -> Matrix

    Inplace standard vector addition.

    :param other: the matrix to add to `self`
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__isub__(other: Matrix) -> Matrix

    Inplace standard vector subtraction.

    :param other: the matrix to subtract from `self`
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__imul__(scalar: SupportsFloat) -> Matrix

    Inplace standard scalar multiplication.

    :param scalar: the scalar to multiply each element of `self` by
    :raises TypeError: if `scalar` is not or cannot be converted to a :class:`float`

.. py:method:: Matrix.__imatmul__(other: Matrix) -> Matrix

    Inplace matrix multiplication.

    :param other: the other matrix to multiply `self` inplace by
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__itruediv__(scalar: SupportsFloat) -> Matrix

    Inplace standard scalar division.

    :param scalar: the scalar to divide each component of `self` by
    :raises TypeError: if `scalar` is not or cannot be converted to a :class:`float`

Logical Operators
-----------------

.. py:method:: Matrix.__eq__(other: Matrix) -> bool

    Compares each component of two matrices for equality.

    .. seealso::

        Checkout :meth:`Matrix.compare_to_tol` and :meth:`Matrix.compare_to_ulp`
        for finer control over evaluating equality.

    :param other: The matrix to compare to `self`
    :return: True if each component of `other` and `self` are equivalent, False otherwise
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. py:method:: Matrix.__ne__(other: Matrix) -> bool

    Compares each component of two matrices for an inequality.

    :return: True if any component of `other` and self are not equivalent, False otherwise
    :raises TypeError: if `other` is not a :class:`Matrix` type

.. note::

    All other logic operators are not implemented and will raise a
    :py:exc:`TypeError`.

Matrix Operators
----------------

.. py:method:: Matrix.compare_to_tol(other: Vector, rel_tol: float = 1e-9, abs_tol: float = 1e-15) -> bool

    Compares `self` to `other` using tolerance based mechanics. This is the
    same way :meth:`Matrix.__eq__` and :meth:`Matrix.__ne__` compare vectors.
    This function allows customization of the relative and absolute tolerance
    values. using the default tolerance values, this function is equivalent
    to :meth:`Matrix.__eq__`. For comparing any two components of `self` and
    `other`, the components are considered equal if, for :python:`diff = abs(a - b)`,
    :python:`diff <= abs_tol + rel_tol * max((abs(a), abs(b)))`.

    :param other: the matrix to compare to `self`
    :param rel_tol: the relative tolerance of the comparison
    :param abs_tol: the absolute tolerance of the comparison
    :return: True if all components are equal, False otherwise
    :raises TypeError: if `other` is not a :class:`Matrix` type or `rel_tol` or `abs_tol`
        are not :class:`float` types.

.. py:method:: Matrix.compare_to_ulp(other: Matrix, max_ulps: int) -> bool

    Compares `self` to `other` using `ULP <https://en.wikipedia.org/wiki/Unit_in_the_last_place>`_
    (Unit in the Last Place) based mechanics. The number of ULPs between two
    floating-point numbers is the number of representable floating-point value
    between the two numbers. This can be a valid way of comparing two
    floating-point numbers in certan domains, however for the types of projects
    PyEVSpace was built to support, it is not sufficient, which is why
    :meth:`Matrix.__eq__` uses tolerance based comparisons.

    The most obvious example of why ULP based comparison does not suffice for
    our uses is comparing two values that would be very common to encounter. The
    values :python:`cos(math.pi / 4)` and :python:`sin(math.pi / 4)` should be
    identical, however because of the binary representation of floating-point
    values, the number of ULPs between these values is quite large. In fact these
    values evaluate as False using the built-in Python equality operator:

    .. code-block:: python

        >>> print(cos(pi / 4))
        0.7071067811865476
        >>> print(sin(pi / 4))
        0.7071067811865475
        >>> #            ^ notice the rounding differences
        >>> print(cos(pi / 4) == sin(pi / 4))
        False

    Using a default number of ULPs that accommodates this equality would also
    allow other, less equivalent values to evaluate as equal which should not
    be accepted.

    This method still allows users who would like an ULP based comparison to
    be able to use it. In some cases, a very high level of matrix equality may
    be waranted. If tolerance based comparison is still ideal for you
    but you want finer controls over the tolerance values, see :meth:`Matrix.compare_to_tol`.

    :param other: the matrix to compare to `self`
    :param max_ulps: the maximum amount of ULPs two components can differ by and
        still evaluate as equal
    :return: True if all components are equal, False otherwise
    :raises TypeError: if `other` is not a :class:`Matrix` type or `max_ulps` is not
        an :class:`int` type

.. py:method:: Matrix.determinate() -> float

    Computes the determinate of a matrix.

    :return: the detminate of `self`

.. py:method:: Matrix.inverse() -> Matrix

    Computes the inverse of a matrix. A matrix whose determinate is equal to `0` is
    said to be *singular*. A singular matrix is not invertible, therefore be aware this
    method may not succeed. A purely rotational matrix, by definition, has a determinate
    of `1`, so if you are calling this method on a matrix returned by :func:`compute_rotation_matrix`
    you won't have any problems with invertibility. While much more can be said about
    invertibile matrices, in the scope of this package the inverse of a matrix can be
    used to apply the opposite rotation a matrix represents.

    .. code-block:: python

        >>> v = Vector(1, 2, 3)
        >>> m = compute_rotation_matrix(1.5, X_AXIS) # rotation 1.5 radians around the x-axis
        >>> rotated_vector = v @ m
        >>> print(rotated_vector)
        [1, 3.13396, -1.78278]
        >>> unrotated_vector = rotated_vector @ m.inverse() # 'reverse' the rotation
        >>> print(unrotated_vector)
        [1, 2, 3]
        >>> print(unrotated_vector == v)
        True

    :return: the inverse of `self` if `self` is a non-singular matrix
    :raises ValueError: if `self` is a singular matrix, i.e. :python:`self.determinate() == 0.0`

.. py:method:: Matrix.transpose() -> Matrix

    Creates a new matrix equal to the transpose of `self`. The *ith* row and *jth* column
    component of the resulting matrix will be the *jth* row and *ith* column component of
    `self`.

    :return: the transpose of `self`

.. py:method:: Matrix.transpose_inplace() -> None

    Same as :meth:`Matrix.transpose` except the calling matrix is modified in place.
    This would be equivalent to :python:`m = m.transpose()`.

    :return: None

Attributes
==========

.. py:attribute:: Matrix.IDENTITY

    Identity matrix with ones on the diagonal and zeros everwhere else.
    **This value should not be edited.**

    :value: :python:`Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1))`
    :type: Matrix

.. py:attribute:: Matrix.id

    .. versionremoved:: 0.15.0
        Use :attr:`Matrix.IDENTITY`

Module Methods
==============

.. function:: det(matrix)

    .. versionremoved:: 0.15.0
        Use :meth:`Matrix.determinate`

.. function:: transpose(matrix)

    .. versionremoved:: 0.15.0
        Use :meth:`Matrix.transpose`

Buffer Protocol
===============

.. method:: Matrix.__buffer__(flags: int) -> memoryview

    Returns a :class:`memoryview` object with `self` as the reference object.

    :param flags: a combined enumerated value from :class:`inspect.BufferFlags`
        to control the exported :class:`memoryview` object
    :return: the exported :class:`memoryview` object

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

This can also be used for interfacing with buffer supporting libraries
like `NumPy <https://numpy.org>`_

.. code-block:: python

    >>> import numpy as np
    >>> from pyevspace import Matrix
    >>>
    >>> matrix = Matrix()
    >>> print(matrix)
    [[0, 0, 0]
    [0, 0, 0]
    [0, 0, 0]]
    >>> # give arr access to the underlying data buffer of matrix
    >>> arr = np.ndarray((3, 3), buffer=matrix)
    [[0. 0. 0.]
     [0. 0. 0.]
     [0. 0. 0.]]
    >>> # changes to arr also modify matrix as they share the same memory
    >>> arr[0, 0] = 10
    >>> print(arr)
    [[10.  0.  0.]
     [ 0.  0.  0.]
     [ 0.  0.  0.]]
    >>> print(matrix)
    [[10, 0, 0]
    [0, 0, 0]
    [0, 0, 0]]

MatrixView Type
===============

.. class:: _MatrixView

    .. note::
        This type is not part of the public API and should not be instantiated directly.
        It is returned internally by :meth:`Matrix.__getitem__` when a :class:`slice` is
        used and exposes a :class:`memoryview` of the underlying matrix data.
