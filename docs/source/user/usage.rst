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

Creating an :class:`EVector`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
An EVector is created from any iterable with exactly three values. The
simplest way is to use a tuple of values

.. code-block:: python
	
	>>> vector = EVector((1, 2, 3))
	>>> # don't forget the second pair of parenthesis

Of course any iterable will work, including lists and even an 
:class:`EVector`

.. code-block:: python

	>>> vector = EVector([2.1, 6.5, 101.3])
	>>> vector = EVector(EVector((4, 5, 6)))

An empty vector, or a vector of zeros can be created without any
parameters to the constructor

.. code-block:: python

	>>> vector = EVector()
	>>> print(vector)
	[0, 0, 0]

Creating an :class:`EMatrix`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
An :class:`EMatrix` is created similarly to an :class:`EVector`, but 
instead of taking an iterable with exactly three values, it takes three 
of them. Each of the iterables cooresponds to the row of the matrix it 
represents.

.. code-block:: python

	>>> matrix = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> print(matrix)
	[[1, 2, 3]
	[4, 5, 6]
	[7, 8, 9]]

Again, any iterable will work, including lists and :class:`EVectors`

.. code-block:: python

	>>> tp = (1, 2, 3)
	>>> ls = [2, 4, 6]
	>>> vec = EVector((3, 6, 9))
	>>> matrix = EMatrix(tp, ls, vec)
	>>> print(matrix)
	[[1, 2, 3]
	[2, 4, 6]
	[3, 6, 9]]

An empty :class:`EMatrix` can also be created without passing any
parameters into the constructor

.. code-block:: python
	
	>>> matrix = EMatrix()
	>>> print(matrix)
	[[0, 0, 0]
	[0, 0, 0]
	[0, 0, 0]]

Arithmetic Operations
^^^^^^^^^^^^^^^^^^^^^

The :class:`EVector` supports vector addition and subtraction as well
as scalar multiplication and division.

.. code-block:: python

	>>> # vector addition
	>>> print(EVector((1, 2, 3)) + EVector((5, 6, 7)))
	[6, 8, 10]
	>>> # vector subtraction
	>>> print(EVector((7, 8, 5)) - EVector((10, 2, 7)))
	[-3, 6, -2]
	>>> # scalar multiplication
	>>> print(EVector((3, 6, 9)) * 1.5)
	[4.5, 9, 13.5]
	>>> # scalar division
	>>> print(EVector((7, 2, 8)) / 0.5)
	[14, 4, 16]

The :class:`EMatrix` supports matrix addition and subtraction as well
as scalar multiplication and division.

.. code-block:: python

	>>> matrix1 = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> matrix2 = EMatrix((9, 8, 7), (6, 5, 4), (3, 2, 1))
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

The :class:`EMatrix` also supports left-hand matrix multiplication of
:class:`EVectors`. The :class:`EMatrix` acts as a linear transformation
of the :class:`EVector`.

	>>> matrix = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
	>>> vector = EVector((1, 2, 3))
	>>> # matrix multiplication
	>>> result = matrix * vector
	>>> print(result)
	[14, 32, 50]
	>>> type(result)
	<class 'pyevspace.EVector'>

Multiple linear transforms can also be compounded via matrix
multiplication.

	>>> # 90 degree rotation around x-axis
	>>> matrix1 = EMatrix((1, 0, 0), (0, 0, 1), (0, -1, 0))
	>>> # 90 degree rotation around y-axis
	>>> matrix2 = EMatrix((0, 0, -1), (0, 1, 0), (1, 0, 0))
	>>> # equivalent transormation equal to a 90 degree rotation around
	>>> # the x-axis, followed by a 90 degree rotation around the y-axis
	>>> compound = matrix2 * matrix1
	>>> print(compound)
	[[0, 1, 0]
	[0, 0, 1]
	[1, 0, 0]]

Other :class:`EVector` Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are other vector like operators for the :class:`EVector` class,
include magnitude, norm, and dot and cross products.

.. code-block:: python

	>>> vector1 = EVector((1, 2, 3))
	>>> vector2 = EVector((4, 5, 6))
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

Other :class:`EMatrix` Operators
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are also two other operators for the :class:`EMatrix` : transpose
and determinate. 

.. code-block:: python

	>>> matrix = EMatrix((7, 2, 5), (5, 2, 9), (6, 3, 3))
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
