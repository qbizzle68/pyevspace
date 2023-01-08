PyEVSpace User Guide
====================

.. role:: python(code)
	:language: python

Usage
-----
To use the module simply import pyevspace into your project

.. code-block:: python

	import pyevspace as evs

As of this writing (|version|) the module is pretty small, so importing
all (:python:`*`) doesn't add much to the importing namespace, however
it is still best practice to only import exactly what you need.

Creating an :class:`Vector`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
A vector is created directly by passing the constructor the x, y and z
components.

.. code-block:: python

	>>> vector = Vector(1, 2, 3)

An iterable can also be used, even another :class:`Vector`.

.. code-block:: python

	>>> vector = Vector([2.1, 6.5, 101.3])
	>>> vector = Vector(Vector((4, 5, 6)))

An empty vector, or a vector of zeros can be created without any
parameters to the constructor

.. code-block:: python

	>>> vector = Vector()
	>>> print(vector)
	[0, 0, 0]

Creating an :class:`Matrix`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
An :class:`Matrix` is created similarly to an :class:`Vector`, but
instead of taking a single iterable with exactly three values, it
takes three of them. Each of the iterables cooresponds to the row
of the matrix it represents.

.. code-block:: python

	>>> matrix = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> print(matrix)
	[[1, 2, 3]
	[4, 5, 6]
	[7, 8, 9]]

Again, any iterable will work, including lists and :class:`Vectors`

.. code-block:: python

	>>> tp = (1, 2, 3)
	>>> ls = [2, 4, 6]
	>>> vec = Vector((3, 6, 9))
	>>> matrix = Matrix(tp, ls, vec)
	>>> print(matrix)
	[[1, 2, 3]
	[2, 4, 6]
	[3, 6, 9]]

An empty :class:`Matrix` can also be created without passing any
parameters into the constructor

.. code-block:: python
	
	>>> matrix = Matrix()
	>>> print(matrix)
	[[0, 0, 0]
	[0, 0, 0]
	[0, 0, 0]]

Arithmetic Operations
^^^^^^^^^^^^^^^^^^^^^

The :class:`Vector` supports vector addition and subtraction as well
as scalar multiplication and division.

.. code-block:: python

	>>> # vector addition
	>>> print(Vector((1, 2, 3)) + Vector((5, 6, 7)))
	[6, 8, 10]
	>>> # vector subtraction
	>>> print(Vector((7, 8, 5)) - Vector((10, 2, 7)))
	[-3, 6, -2]
	>>> # scalar multiplication
	>>> print(Vector((3, 6, 9)) * 1.5)
	[4.5, 9, 13.5]
	>>> # scalar division
	>>> print(Vector((7, 2, 8)) / 0.5)
	[14, 4, 16]

The :class:`Matrix` supports matrix addition and subtraction as well
as scalar multiplication and division.

.. code-block:: python

	>>> matrix1 = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> matrix2 = Matrix((9, 8, 7), (6, 5, 4), (3, 2, 1))
	>>> # matrix addition
	>>> print(matrix1 + matrix2)
	[[10, 10, 10]
	[10, 10, 10]
	[10, 10, 10]]
	>>> # matrix subtraction
	>>> print(matrix1 - matrix2)
	[[-8, -6, -4]
	[-2, 0, 2]
	[4, 6, 8]]
	>>> # scalar multiplication
	>>> print(matrix1 * 2)
	[[2, 4, 6]
	[8, 10, 12]
	[14, 16, 18]]
	>>> # scalar division
	>>> print(matrix2 / 3)
	[[3, 2.66667, 2.33333]
	[2, 1.66667, 2.33333]
	[1, 0.666667, 0.333333]]

The :class:`Matrix` also supports left-hand matrix multiplication of
:class:`Vectors`. The :class:`Matrix` acts as a linear transformation
of the :class:`Vector`.

	>>> matrix = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> vector = Vector((1, 2, 3))
	>>> # matrix multiplication
	>>> result = matrix * vector
	>>> print(result)
	[14, 32, 50]
	>>> type(result)
	<class 'pyevspace.Vector'>

Multiple linear transforms can also be compounded via matrix
multiplication.

	>>> # 90 degree rotation around x-axis
	>>> matrix1 = Matrix((1, 0, 0), (0, 0, 1), (0, -1, 0))
	>>> # 90 degree rotation around y-axis
	>>> matrix2 = Matrix((0, 0, -1), (0, 1, 0), (1, 0, 0))
	>>> # equivalent transormation equal to a 90 degree rotation around
	>>> # the x-axis, followed by a 90 degree rotation around the y-axis
	>>> compound = matrix2 * matrix1
	>>> print(compound)
	[[0, 1, 0]
	[0, 0, 1]
	[1, 0, 0]]

Other :class:`Vector` Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are other vector like operators for the :class:`Vector` class,
include magnitude, norm, and dot and cross products.

.. code-block:: python

	>>> vector1 = Vector((1, 2, 3))
	>>> vector2 = Vector((4, 5, 6))
	>>> # magnitude of a vector
	>>> vector1.mag()
	3.7416573867739413
	>>> # vector norm
	>>> print(norm(vector1))
	[0.267261, 0.534522, 0.801784]
	>>> # vector dot product
	>>> dot(vector1, vector2)
	32
	>>> # vector cross product
	>>> print(cross(vector1, vector2))
	[-3, 6, -3]

Other :class:`Matrix` Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are also two other operators for the :class:`Matrix` : transpose
and determinate. 

.. code-block:: python

	>>> matrix = Matrix((7, 2, 5), (5, 2, 9), (6, 3, 3))
	>>> # matrix transpose
	>>> print(transpose(matrix))
	[[7, 5, 6]
	[2, 2, 3]
	[5, 9, 3]]
	>>> # determinate
	>>> det(matrix)
	-54.0

Check out some more complex :doc:`examples <examples>` and the 
:doc:`api reference </reference/api/index>` for more details about the
types and methods of the package.
