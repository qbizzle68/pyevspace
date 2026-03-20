.. role:: python(code)
    :language: python

.. currentmodule:: pyevspace

===========
Basic Usage
===========
This page is a basic quickstart for diving into the library. While
not exhaustive, at least one example of much of the library can be
found here. This page should be used for getting a basic idea of
the capabilities of PyEVSpace, and can help point you towards which
part of the :doc:`API </reference/index>` you should checkout next
to improve your understanding of the package.

PyEVSpace is easy to use and doesn't have many importable symbols to
memorize. Nonetheless it's still smart to use an alias to avoid
poluting the namespace. Import the module using something like

.. code-block:: python

    >>> import pyevspace as evs

Vector Types
############
The basic ingredient of PyEVSpace is the :py:class:`Vector` type. A
vector is an object that represent both magnitude and direction. A
vector can be instantiated with its x, y, and z coordinates

.. code-block:: python

    >>> vector = evs.Vector(1, 2, 3)

or from an iterable or sequence

.. code-block:: python

    >>> ls = [1, 2, 3]
    >>> vector = evs.Vector(ls)
    >>> print(vector)
    [1, 2, 3]
    >>> d = {'a': 4, 'b': 5, 'c': 6}
    >>> vector = evs.Vector(d.values())
    >>> print(vector)
    [4, 5, 6]

Note that an iterable or sequence used must have a length of exactly
three, and each element must be a :python:`Real` type. This includes
custom data types that implement :python:`__float__` or
:python:`__index__` methods.

A trivial example of this is:

.. code-block:: python

    >>> class DummyFloat:
    ...     def __init__(self, value: float | int):
    ...         self.value = value
    ...     def __float__(self) -> float:
    ...         return float(self.value)
    ...
    >>> x = DummyFloat(1.1)
    >>> y = DummyFloat(2.2)
    >>> z = DummyFloat(3.3)
    >>> vector = evs.Vector(x, y, z)
    >>> print(vector)
    [1.1, 2.2, 3.3]

Using Vectors
*************
Vectors can be added or subtracted together using basic operators

.. code-block:: python

    >>> lhs = evs.Vector(1, 2, 3)
    >>> rhs = evs.Vector(4, 5, 6)
    >>> result = lhs + rhs
    >>> print(result)
    [5, 7, 9]
    >>> result = lhs - rhs
    >>> print(result)
    [-3, -3, -3]

or multiplied/divided by scalars

.. code-block:: python

    >>> vector = evs.Vector(1, 2, 3)
    >>> result = vector * 1.5
    >>> print(result)
    [1.5, 3, 4.5]
    >>> result = vector / 2
    >>> print(result)
    [0.5, 1, 1.5]

Vector Functions
****************
There are several instance methods and module functions that can be
used to compute useful data for vectors. For example the length of a
vector can be computed using the :py:meth:`Vector.magintude` method:

.. code-block:: python

    >>> vector = evs.Vector(0, 3, 4)
    >>> print(vector.magnitude())
    5.0

A vector can be normalized by dividing by the magnitude which results
in a vector pointing in the same direction as the original, but has a
length of exactly one, sometimes called a unit vector.

.. code-block:: python

    >>> vector = evs.Vector(0, 3, 4)
    >>> vector_norm1 = vector / vector.magnitude()
    >>> print(vector_norm1)
    [0, 0.6, 0.8]
    >>> vector_norm2 = vector.norm()    # returns a new normalized vector
    >>> print(vector_norm2)
    [0, 0.6, 0.8]
    >>> print(vector)       # vector remains unchanged
    [0, 3, 4]
    >>> vector.normalize()   # normalize vector by modifying in place
    print(vector)
    [0, 0.6, 0.8]
    >>> print(vector_norm1 == vector_norm2 == vector)
    True

Other module level functions compute values or new vectors from two
vector arguments:

* :py:func:`vector_angle`
* :py:func:`vector_cross`
* :py:func:`vector_dot`
* :py:func:`vector_exclude`
* :py:func:`vector_proj`

Check the :doc:`API reference </reference/index>` for more information
on these methods.

Matrix Types
############
A :py:class:`Matrix` object does not need to represent only rotations,
but can be any linear transformation. Matrices will most commonly be
created using factory functions, however they can be instantiated similarly
to :py:class:`Vector`. They take either three iterables or a single iterable
that contains three iterables. Like Vector objects, each iterable must have
a length of exactly three and contain :python:`Real` elements.

.. code-block:: python

    >>> row1 = [1, 2, 3]
    >>> row2 = [4, 5, 6]
    >>> row3 = [7, 8, 9]
    >>> matrix = evs.Matrix(row1, row2, row3)
    >>> print(matrix)
    [[1, 2, 3],
    [4, 5, 6],
    [7, 8, 9]]
    >>> container = [row1, row2, row3]
    >>> matrix = evs.Matrix(container)
    >>> print(matrix)
    [[1, 2, 3]
    [4, 5, 6],
    [7, 8, 9]]

Using Matrices
**************
Matrices can be altered mathematically using similar operators as shown
with the vectors above (addition, subtraction, scalar multiplication/division).
Additionally, matrices can be multiplied together and with vectors. Two matrices
multiplied together produce another matrix, a matrix and vector multiplied
together produce a vector.

.. note::

    Matrix multiplication (any multiplication involving at least one Matrix
    type) is implemented in PyEVSpace using the matrix multiplication operator
    (:python:`@`). Attempting to use matrix multipliation with the :python:`*` operator
    will raise a :python:`TypeError`.

.. code-block:: python

    >>> matrix = evs.Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    >>> vector = evs.Vector(1, 2, 3)
    >>> result = matrix @ vector
    >>> print(result)
    [14, 32, 50]
    >>> result = matrix @ matrix
    >>> print(result)
    [[30, 36, 42]
    [66, 81, 96],
    [102, 126, 150]]
    >>> try:
    ...     result = matrix * vector
    ... except TypeError:
    ...     print("Encountered a TypeError")
    ...
    Encountered a TypeError

Matrix Methods
**************
Several common matrix utilities are implemented as instance methods
on the Matrix class. They are

* :py:meth:`Matrix.determinate`
* :py:meth:`Matrix.inverse`
* :py:meth:`Matrix.transpose`
* :py:meth:`Matrix.transpose_inplace`

Check the :doc:`API reference </reference/index>` for more information
on these methods.

Rotations
#########
Rotations of vectors are how a vector defined in one reference frame
"looks" in another. This is done by multiplying the vector by a very
specific matrix, which is computed depending on the definition of the
rotation. This is generally defined by the axes of rotations and the
angles around these axes.

Axes
****
Axes in PyEVSpace are enumerated values. They are defined using
integer constants at the module level as :python:`X_AXIS`,
:python:`Y_AXIS`, and :python:`Z_AXIS`. As these are integer
constants, and the values 0, 1, and 2 can be used in their place,
and allow for programaticly setting an axis dynamically, however
the first version should be preferred as it produces code which is
easier to infer the intent of.

RotationOrder
*************
A common class of rotations are called `Euler rotations <https://en.wikipedia.org/wiki/Euler_angles>`_.
These rotations are defined by a set of three axes and cooresponding
angles around which the respective axes are rotated. For example a
rotation around the X, then Y, then Z axes would be an XYZ rotation.
The :py:class:`RotationOrder` type defines these orders. While this
type can be instantiated, the module contains instances of the 12
most common orders. These include all permutations of the X, Y, and
Z axes, as well as all permutations where the first and last axes
are the same e.g. XYX, ZXZ.

EulerAngles
***********
The :py:class:`EulerAngles` object holds the angles that coorespond
to an Euler rotation. The class is just a container that holds three
angles (in radians) where the *ith* angle cooresponds to the *ith*
axis.

Creating Rotation Matrices
**************************
Matrices can be computed using the :py:method::`compute_rotation_matrix`
method. This function accepts many combinations of argument types so
check the :doc:`documentation</reference/index>` for more information
on all signatures. The following snippet shows some brief examples of
how to use the function.

.. code-block:: python

    >>> # from an angle and axis
    >>> matrix = evs.compute_rotation_matrix(math.pi, evs.X_AXIS)
    >>> # from an angle and vector (rotates around the vector)
    >>> matrix = evs.compute_rotation_matrix(math.pi, evs.Vector(1, 1, 1))
    >>> # using Euler angles
    >>> angles = EulerAngles(1, 2, 3)   # values in radians
    >>> matrix = evs.compute_rotation_matrix(XYZ, angles)
    >>> # extrinsic Euler rotation 
    >>> matrix = evs.compute_rotation_matrix(XYZ, angles, intrinsic=False)

Rotation Functions
******************
You can use derived matrices (as shown above) to apply the
transformations yourself, but PyEVSpace provides several helper
functions for internally computing the rotation matrix and apply
the rotation for you. The signatures for these functions are similar
to the :py:method::`compute_rotaiton_matrix` function. The following
snippet shows some brief examples of how to use the functions.

.. code-block:: python

    >>> # rotate from an offset reference frame to an inertial frame
    >>> # using angle axis
    >>> vector = evs.rotate_from(math.pi, evs.Z_AXIS, Vector(1, 1, 1))
    >>> print(vector)
    [-1, -1, 1]
    >>> vector = evs.rotate_from(math.pi, evs.Vector.E3, Vector(1, 1, 1))
    >>> print(vector)
    [-1, -1, 1]
    >>> angles = EulerAngles(math.pi / 2, math.pi / 2, math.pi / 2)
    >>> vector = evs.rotate_from(XYZ, angles, Vector(1, 1, 1))
    >>> print(vector)
    [1, -1, 1]
    >>> matrix = compute_rotation_matrix(XYZ, angles)
    >>> vector = evs.rotate_from(matrix, vector)
    >>> print(vector)
    [1, -1, 1]

.. note::
    All versions of :py:method::`rotate_to` have the same signatures as
    :py:method::`rotate_from`.
