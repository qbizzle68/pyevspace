.. include:: /global.rst

====================
Vector API Reference
====================

.. currentmodule:: pyevspace

The Vector Type
===============

.. py:class:: Vector()
	Vector(x: SupportsFloat, y: SupportsFloat, z: SupportsFloat)
	Vector(container: Iterable)

	The Vector type represents a vector of a 3-dimensional vector space
	over the real numbers. Vectors are mathematical objects that describe
	a length and direction. The three components of a 3D vector describe
	how many units the vector points in each direction of the x, y, and z
	axis of a reference frame. The distance from the origin to the (x, y, z)
	coordinate is the length of the vector. Vectors can represent positions
	in space, as well as other physical quantities such as velocities,
	accelerations, and forces. For example, a force vector's length represents
	the quantity of the force (in units of force, like Newtons or pounds), and
	the direction of the vector represents the direction the force is applied.

	The vector is the basic unit of PyEVSpace. The Vector type defines most
	arithmetic operators, as well as many methods useful for working with
	vectors, such as dot and cross product, or projecting one vector onto
	another. Vectors can also be rotated, within a reference frame or to or
	from another reference frame. The :class:`Matrix` is used for such cases.
	While matrices can directly be used to apply transformations to vectors,
	the library also provides several functions for handling these rotation
	for you.

	A vector can be initialized by specifying its components directly, or inside
	an iterable container that contains exactly those three elements. By not
	supplying any arguments the vector components will be initialized to zero.
	If initializing by provided components, all components must be specified.

	Direct elements or elements of `container` must be a :class:`float`, or
	compatible with :py:class:`SupportsFloat`, meaning they can be converted to
	a float by defining a :meth:`__float__` method.

	.. versionchanged:: 0.16.0
		Vector is now subclassable, and is no longer an immutable type,
		meaning class members can be directly modified or added.

	:param x: value to initialize the x-component to
	:param y: value to initialize the y-component to
	:param z: value to initialize the z-component to
	:param container: an iterable object of length three with elements compatible
		with :py:class:`SupportsFloat` (this must be the only parameter if present)
	:raises TypeError: if `x`, `y`, or `z` cannot be converted to a :py:class:`float`
	:raises TypeError: if a value of `container` cannot be converted to a :py:class:`float`
	:raises ValueError: if :python:`len(container) != 3`

Other Constructors
------------------

.. py:classmethod:: Vector.__new__(cls: type) -> Self

	Creates a new, uninitialized `cls` object if `cls` is a subclass
	of :py:class:`Vector`. All components of the vector default to zero.

	:param cls: must be a subtype of :class:`Vector`
	:return: an uninitialized :class:`Vector` object
	:raises TypeError: if `cls` is not a subtype of :class:`Vector`
	
	.. versionchanged:: 0.16.0
		`type` can be a subtype of :py:class:`Vector`, and an object of that
		type will be created and returned.

.. py:method:: Vector.__init__()
	Vector.__init__(x: SupportsFloat, y: SupportsFloat, z: SupportsFloat)
	Vector.__init__(container: Iterable)

	Initialize a :class:`Vector` depending on the arguments. This method and arguments
	behave identically to the :class:`Vector` constructor.

	:param x: value to initialize the x-component to
	:param y: value to initialize the y-component to
	:param z: value to initialize the z-component to
	:param container: an iterable object of length three with elements compatible
		with :py:class:`SupportsFloat` (this must be the only parameter if present)
	:raises TypeError: if `x`, `y`, or `z` cannot be converted to a :py:class:`float`
	:raises TypeError: if a value of `container` cannot be converted to a :py:class:`float`
	:raises ValueError: if :python:`len(container) != 3`

Subclassing :class:`Vector`
===========================

As of version `0.16.0` the :class:`Vector` class supports subclassing. As the
:class:`Vector` class is implemented as a C struct, it has a specific memory
layout, and as a consequence of that it is a :deco:`distjoint-base`. This
means :class:`Vector` cannot be combined with any other types whose layout
is defined as a C struct when inheriting, which most Python built-in types
such as :class:`list` or :class:`tuple` are. For example combining :class:`Vector`
and :class:`list` as base classes will raise a :exc:`TypeError`:

.. code-block:: python

	>>> class Foo(Vector, list):
	...     pass
	...
	TypeError: multiple bases have instance lay-out conflict

This limitation does not apply when combining a :deco:`distjoin-base` with a
pure python class, and of course is not an issue if :class:`Vector` is the only
base class for a derived type.

While :class:`Vector` supports inheritance, there is no way for the
constructor to know how to initialize an inherited type. Because of this,
any function (except :meth:`Vector.__new__`) that returns a :class:`Vector`
type will always return a :class:`Vector` instance, even on inherited types.
To change this, you will need to override the function you want to return your
type, and wrap the returned value in the constructor of your type. For example:

.. code-block:: python

	>>> class MyType(Vector):
	...     def __init__(self, container, foo, bar):
	...         super().__init__(self, container)
	...         self.foo = foo
	...         self.bar = bar
	...     def __add__(self, other: MyType) -> MyType:
	...         super().__add__(other)
	...         return MyType(result, self.foo, self.bar)

In this example the result of the :meth:`Vector.__add__` call is used to
create a :class:`MyType` instance. Any method you want to have this behavior must
be implemented yourself, including module level functions that normally
return a :class:`Vector` type.

Instance Methods
================

General Protocols
-----------------

.. py:method:: Vector.__len__() -> int

	Returns the length of the :class:`Vector`. This always returns `3`.

	:return: the method always returns `3`

.. py:method:: Vector.__getitem__(index: int) -> float

	Retrieves the indexed value of the underlying array.

	:param index: the index of the value to retrieve
	:return: the indexed value of self
	:raises TypeError: if `index` is not or cannot be converted to an :py:class:`int`
	:raises ValueError: if `index` is not in the interval [0, 2] (after
		adjusting for negative indexing)

.. py:method:: Vector.__setitem__(index: int, value: SupportsFloat) -> float

	Sets the indexed value of the underlying array.

	:param index: the index of the value to set
	:param value: the value to set the array component to
	:return: `value`
	:raises TypeError: if `index` is not or cannot be converted to an :py:class:`int`
	:raises TypeError: if `value` is not or cannot be converted to a :py:class:`float`
	:raises ValueError: if `index` is not in the interval [0, 2]

.. py:method:: Vector.__repr__() -> str

	Returns a string representation of self, representative of a constructor
	call with the component values of the vector. The format of the components
	follows the same as :py:meth:`Vector.__str__()`.

	:return: a string representation of self

	.. versionchanged:: 0.16.0
		The method now prepends the output with the module name for better
		scope resolution support when using the output with :func:`exec`.

.. py:method:: Vector.__str__() -> str

	Returns a string representation of self. The format of the output
	string is similar to a :class:`list`, the components enclosed by
	square brackets. The format of the components are either decimal
	or scientific notation, whichever requires fewer bytes to store
	their string representations.

	:return: a string representation of self

.. py:method:: Vector.__reduce__()

	Allows support for pickling and deep copying. This function returns a
	2-tuple containing a reference to the type constructor and a tuple of
	arguments to initialize an equivalent instance. For types inheriting from
	:class:`Vector`, this function may need to be overloaded and modified
	to return your own type.

	:return: a tuple used for reconstructing self's state
	:rtype: tuple[type, tuple[float, float, float]]

Iterability
-----------

While the :py:class:`Vector` class does not define an :py:meth:`__iter__` method
directly, the class does support iterability via the sequence protocol. Attempting
to access :py:meth:`Vector.__iter__` will raise an :py:exc:`AttributeError`, however
a :py:class:`Vector` can be used anywhere an iterable is expected.

.. code-block:: python

	>>> v = Vector(1, 2, 3)
	>>> itr = iter(v)
	>>> print(list(itr))
	[1.0, 2.0, 3.0]
	>>> for i in v:
	...     print(i)
	...
	1.0
	2.0
	3.0

Arithmetic Operators
--------------------

.. py:method:: Vector.__add__(other: Vector) -> Vector

	Standard vector addition.

	:param other: the vector to be added
	:return: the vector sum of `self` and `other`
	:raises TypeError: if `other` is not a :class:`Vector` type

.. py:method:: Vector.__sub__(other: Vector) -> Vector

	Standard vector subtraction.

	:param other: the vector to be subtracted
	:return: the vector subtraction of `self` and `other`
	:raises TypeError: if `other` is not a :class:`Vector` type

.. py:method:: Vector.__mul__(scalar: SupportsFloat) -> Vector

	Standard scalar multiplication.

	:param scalar: the scalar to multiply each element of self by
	:return: the scalar product
	:raises TypeError: if `scalar` is not or cannot be converted to a :py:class:`float`

.. py:method:: Vector.__matmul__(matrix: Matrix) -> Vector

	Multiplication with the vector on the left side.

	:param matrix: the matrix to multiply self by
	:return: the transformation of self by matrix
	:raises TypeError: if `matrix` is not a Matrix type

.. py:method:: Vector.__truediv__(scalar: SupportsFloat) -> Vector

	Standard scalar division.

	:param scalar: the scalar to divide each element of self by
	:return: the scalar quotient
	:raises TypeError: if `scalar` is not or cannot be converted to a :py:class:`float`

.. py:method:: Vector.__neg__() -> Vector

	Negates each component of a vector. This results in a vector
	of the same length as `self`, but points in the opposite direction.

	:return: the negative of a vector

.. py:method:: Vector.__iadd__(other: Vector) -> Vector

	Inplace standard vector addition.

	:param other: the vector to add to self
	:raises TypeError: if `other` is not an :class:`Vector` type

.. py:method:: Vector.__isub__(other: Vector) -> Vector

	Inplace standard vector subtraction.

	:param other: the vector to subtract from self
	:raises TypeError: if `other` is not an :class:`Vector` type

.. py:method:: Vector.__imul__(scalar: SupportsFloat) -> Vector

	Inplace standard scalar multiplication.

	:param scalar: the scalar to multiply each element of self by
	:raises TypeError: if `scalar` is not or cannot be converted to a :class:`float`

.. py:method:: Vector.__itruediv__(scalar: SupportsFloat) -> Vector

	Inplace standard scalar division.

	:param scalar: the scalar to divide each element of self by
	:raises TypeError: if `scalar` is not or cannot be converted to a :class:`float`

.. py:method:: Vector.__rmul__(scalar: SupportsFloat) -> Vector:

	Provides support for right-hand side scalar multiplication of
	Vector types, e.g. :python:`1.5 * vector`.

	:param rhs: scalar to multiply `self` by
	:return: `self` multiplied by `scalar`
	:raises TypeError: if `scalar` is not or cannot be converted to a :class:`float`

Logical Operators
-----------------

.. py:method:: Vector.__eq__(other: Vector) -> bool

	Compares each element of two vectors for equality.

	.. seealso::
		Checkout :meth:`Vector.compare_to_tol` and :meth:`Vector.compare_to_ulp`
		for finer control over evaluating equality.

	:param other: the vector to compare to `self`
	:return: True if each component of `self` and `other` are equivalent, False otherwise
	:raises TypeError: if `other` is not a :class:`Vector` type

.. py:method:: Vector.__ne__(other: Vector) -> bool

	Compares each element of two vectors for an inequality.

	:param other: the vector to compare to `self`
	:return: True if any component of `self` and `other` are not equivalent, False otherwise
	:raises TypeError: if `other` is not a :class:`Vector` type

.. note::

	All other logic operators are not implemented and will raise a
	:py:exc:`TypeError`.

Vector Operators
----------------

.. py:method:: Vector.compare_to_tol(other: Vector, rel_tol: float = 1e-9, abs_tol: float = 1e-15) -> bool

	Compares `self` to `other` using tolerance based mechanics. This is the
	same way :meth:`Vector.__eq__` and :meth:`Vector.__ne__` compare vectors.
	This function allows customization of the realtive and absolute tolerance
	values. Using the default tolerance values, this function is equivalent
	to :meth:`Vector.__eq__`. For comparing any two components of `self` and
	`other`, the components are considered equal if, for :python:`diff = abs(a - b)`,
	:python:`diff <= abs_tol + rel_tol * max((abs(a), abs(b)))`.

	:param other: the vector to compare to `self`
	:param rel_tol: the relative tolerance of the comparison
	:param abs_tol: the absolute tolerance of the comparison
	:return: True if all components are equal, False otherwise
	:raises TypeError: if `other` is not a :class:`Vector` type or `rel_tol` or `abs_tol` are
		not float types.

.. py:method:: Vector.compare_to_ulp(other: Vector, max_ulps: int) -> bool

	Compares `self` to `other` using `ULP <https://en.wikipedia.org/wiki/Unit_in_the_last_place>`_
	(Unit in the Last Place) based mechanics. The number of ULPs between two
	floating-point numbers is the number of representable floating-point value
	between the two numbers. This can be a valid way of comparing two
	floating-point numbers in certan domains, however for the types of projects
	PyEVSpace was built to support, it is not sufficient, which is why
	:meth:`Vector.__eq__` uses tolerance based comparisons.

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
	be able to use it. In some cases, a very high level of vector equality may
	be waranted. If tolerance based comparison is still ideal for you
	but you want finer controls over the tolerance values, see :meth:`Vector.compare_to_tol`.

	:param other: the vector to compare to `self`
	:param max_ulps: the maximum amount of ULPs two components can differ by and
		still evaluate as equal
	:return: True if all components are equal, False otherwise
	:raises TypeError: if `other` is not a :class:`Vector` type or `max_ulps` is not an int type

.. py:method:: Vector.magnitude() -> float

	Computes the geometric length of the vector.

	:return: the magnitude of self

.. py:method:: Vector.magnitude_squared() -> float

	Computes the square of the magnitude of the vector. Because
	:meth:`Vector.magnitude` computes the square of a vector's magnitude
	and returns it's square root, calling :meth:`Vector.magnitude` and
	squaring the result will result in rounding errors. Calling this
	method directly preserves accuracy in such cases.

	.. note::
		This is identical to :python:`vector_dot(self, self)`, but is more
		explicit and preserves intent when reading code.

	:return: the square of the magnitude of `self`

.. py:method:: Vector.norm() -> Vector

	Compute a new vector equal to the norm of self. This vector points in
	the same direction as `self`, but has a length of one, also called a
	unit vector.

	:return: `self` as a unit vector

.. py:method:: Vector.normalize() -> None

	Normalizes `self` by dividing each element by the magnitude
	of the vector. This results in a vector with length equal to one.

	:return: None

.. py:method:: Vector.mag()

	.. versionremoved:: 0.15.0
		Use :meth:`Vector.magnitude`

.. py:method:: Vector.mag2()

	.. versionremoved:: 0.15.0
		Use :meth:`Vector.magnitude_squared`

Attributes
==========

.. py:attribute:: Vector.E1

	Elementary vector that represents the x-axis of the standard
	basis. **This value should not be modified.**

	:value: :python:`Vector(1, 0, 0)`
	:type: Vector

.. py:attribute:: Vector.E2

	Elementary vector that represents the y-axis of the standard
	basis. **This value should not be modified.**

	:value: :python:`Vector(0, 1, 0)`
	:type: Vector

.. py:attribute:: Vector.E3

	Elementary vector that represents the z-axis of the standard
	basis. **This value should not be modified.**

	:value: :python:`Vector(0, 0, 1)`
	:type: Vector

.. py:attribute:: Vector.e1

	.. versionremoved:: 0.15.0
		Use :attr:`Vector.E1` instead.

.. py:attribute:: Vector.e2

	.. versionremoved:: 0.15.0
		Use :attr:`Vector.E2` instead.

.. py:attribute:: Vector.e3

	.. versionremoved:: 0.15.0
		Use :attr:`Vector.E3` instead.

Module Methods
==============

.. function:: vector_angle(lhs: Vector, rhs: Vector) -> float

	Computes the shortest angle between vectors `lhs` and `rhs` in radians.
	The order of operands does not matter, and
	:python:`vector_angle(lhs, rhs) == vector_angle(rhs, lhs)` is always True.

	:param lhs: left-hand side of the operation
	:param rhs: right-hand side of the operation
	:return: the shortest angle between `lhs` and `rhs` in radians
	:raises TypeError: if `lhs` or `rhs` is not a Vector type

.. function:: vector_cross(lhs: Vector, rhs: Vector) -> Vector

	Computes the cross product of lhs and rhs. PyEVSpace uses right-handed
	a coordinate system. If a left-handed cross product is needed, negate
	the result or swap the order of arguents.

	.. code-block:: python

		>>> v1 = Vector(1, 2, 3)
		>>> v2 = Vector(4, 5, 6)
		>>> print(vector_cross(v1, v2)) # opposite direction for left-handed system
		[-3, 6, -3]
		>>> # either negate the answer
		>>> print(-vector_cross(v1, v2))
		[3, -6, 3]
		>>> # or swap the order of arguments
		>>> print(vector_cross(v2, v1))
		[3, -6, 2]

	:param lhs: left-hand side of the operation
	:param rhs: right-hand side of the operation
	:return: the right-handed cross product of `lhs` and `rhs`
	:raises TypeError: if `lhs` or `rhs` is not a Vector type

.. function:: vector_dot(lhs: Vector, rhs: Vector) -> float

	Computes the dot product between two vectors. The order of arguments
	is not significant as :python:`vector_dot(v1, v2) == vector_dot(v2, v1)`.

	:param lhs: first vector argument
	:param rhs: second vector argument
	:return: the dot product of lhs and rhs
	:raises TypeError: if `lhs` or `rhs` is not a Vector type

.. function:: vector_exclude(vector: Vector, exclude: Vector) -> Vector

	Computes a vector from `vector` with all direction of `exclude` removed
	from it. This results in a linearly independent vector from `exclude`
	such that :python:`vector_dot(result, exclude)` is `0.0`. In other words,
	the resulting vector and `exclude` are othogonal to one another.

	Another way of viewing this is, if `exclude` is viewed as a normal vector
	to a plane, the resulting vector is the projection of `vector` onto that
	plane. This resulting vector points in all directions of `vector`, except
	any portions in the direction of `exclude`, hence it lies in this plane.

	:param vector: base vector
	:param exclude: vector whose direction will be excluded from `vector`
	:return: `vector` with any direction of `exclude` removed
	:raises TypeError: if `vector` or `exclude` is not a Vector type

.. function:: vector_proj(vector: Vector, onto: Vector) -> Vector

	Projects one vector onto another. This results in a vector pointing
	in the same direction as `onto`, but whose length is
	:python:`vector.magnitude * cos(vector_angle(vector, onto))`.

	:param vector: vector to project
	:param onto: vector being projected onto
	:return: `vector` projected onto `onto`
	:raises TypeError: if `vector` or `onto` is not a Vector type

.. function:: dot(lhs, rhs)

	.. versionremoved:: 0.15.0
		Use :func:`vector_dot`

.. function:: cross(lhs, rhs)

	.. versionremoved:: 0.15.0
		Use :func:`vector_cross`

.. function:: norm(vector)

	.. versionremoved:: 0.15.0
		Use :meth:`Vector.norm`

.. function:: vang(lhs, rhs)

	.. versionremoved:: 0.15.0
		Use :func:`vector_angle`

.. function:: vxcl(vector, exclude)

	.. versionremoved:: 0.15.0
		Use :func:`vector_exclude`

.. function:: proj(vector, onto)

	.. versionremoved:: 0.15.0
		Use :func:`vector_proj`

Buffer Protocol
===============

.. method:: Vector.__buffer__(flags: int) -> memoryview

	Returns a memoryview object with `self` as the reference object.

	:param flags: a combined enumerated value from `inspect.BufferFlags`
		to control the exported :python:`memoryview` object
	:return: the exported :python:`memoryview` object

The :class:`Vector` class supports the buffer protocol and can be
used by other objects which also support the buffer interface. For
example it can be used to instantiate a :py:class:`memoryview` object

.. code-block:: python

	>>> vector = Vector(1, 2, 3)
	>>> view = memoryview(vector)
	>>> view[2] = 3.14
	>>> print(vector)
	[1, 2, 3.14]

This can also be used for interfacing with buffer supporting libraries
like `NumPy <https://numpy.org>`_.

.. code-block:: python

	>>> import numpy as np
	>>> from pyevspace import Vector
	>>>
	>>> vector = Vector(1, 2, 3)
	>>> print(vector)
	[1, 2, 3]
	>>> # give arr access to the underlying data buffer of vector
	>>> arr = np.ndarray((3,), buffer=vector)
	>>> print(arr)
	[1, 2, 3]
	>>> # changes to arr also modify vector as they share the same memory
	>>> arr[0] = 10
	>>> print(arr)
	[10, 2, 3]
	>>> print(vector)
	[10, 2, 3]
