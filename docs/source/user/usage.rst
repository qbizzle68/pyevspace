PyEVSpace User Guide
====================

.. role:: python(code)
    :language: python

Usage
-----
To use the module simply import pyevspace into your project

.. code-block:: python

    import pyevspace as evs

Creating a :py:class:`Vector`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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

Creating a :py:class:`Matrix`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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

The :py:class:`Vector` and :py:class:`Matrix` support the standard operators
for respective types. Both right and left multiplication of vectors and
matrices are supported. The following demonstrate the possible operations:

.. code-block:: python

    >>> vector1 = Vector(1, 2, 3)
    >>> vector2 = Vector(4, 5, 6)
    >>> matrix1 = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    >>> matrix2 = Matrix((3, 2, 1), (6, 5, 4), (9, 8, 7))
    >>>
    >>> vectorSum = vector1 + vector2
    >>> vectorDiff = vector1 - vector2
    >>> vectorMultS = vector1 * 2
    >>> vectorMultM = vector1 @ matrix1
    >>> vectorDiv = vector1 / 42
    >>> vector1 += vector2
    >>> vector1 -= vector2
    >>> vector1 *= 1.5
    >>> vector1 /= 34
    >>> vectorNeg = -vector1
    >>>
    >>> matrixSum = matrix1 + matrix2
    >>> matrixDiff = matrix1 - matrix2
    >>> matrixMultS = matrix1 * 3.14
    >>> matrixMultV = matrix1 @ vector1
    >>> matrixMultM = matrix1 @ matrix2
    >>> matrixDiv = matrix1 / 3.0
    >>> matrix1 += matrix2
    >>> matrix1 -= matrix2
    >>> matrix1 *= 1.1
    >>> matrix1 /= 2.2
    >>> matrixNeg = -matrix1

Generating Matrices
^^^^^^^^^^^^^^^^^^^

Rotation matrices can be generated using the getMatrix*
:ref:`methods <mat-gen-label>`.

.. code-block:: python

    >>> vector = Vector(1, 2, 3)
    >>> axisRotation = getMatrixAxis(X_AXIS, 1.234)
    >>> axisRotatedVector = axisRotation @ vector
    >>>
    >>> angles = Angles(1.0, 2.0, 3.0) # angles in radians
    >>> eulerRotation = getMatrixEuler(ZYX, angles)
    >>> eulerRotatedVector = eulerRotation @ vector
    >>>
    >>> anglesTo = Angles(3.0, 2.0, 1.0)
    >>> rotation = getMatrixFromTo(XYZ, angles, YZY, anglesTo)
    >>> fromToVector = vector @ rotation

Rotating Vectors
^^^^^^^^^^^^^^^^

The rotate*To and rotate*From :ref:`methods <rotate-meth-label>` will
handle all the nuance of rotating a vector for you. The same rotated
vectors created above can be equivalently computed below:

.. code-block:: python

    >>> vector = Vector(1, 2, 3)
    >>> axisRotatedVector = rotateAxisFrom(X_AXIS, 1.234, vector)
    >>>
    >>> angles = Angles(1.0, 2.0, 3.0)
    >>> eulerRotatedVector = rotateEulerFrom(ZYX, angles, vector))
    >>>
    >>> anglesTo = Angles(3.0, 2.0, 1.0)
    >>> rotation = getMatrixFromTo(XYZ, angles, YZY, anglesTo)
    >>> fromToVector = rotateMatrixTo(rotation, vector)

Reference Frames
^^^^^^^^^^^^^^^^

ReferenceFrames can also manage rotating vectors for you.

.. code-block:: python

    >>> frameA = ReferenceFrame(XZX, Angles(1.0, 4.0, 2.1))
    >>> frameB = ReferenceFrame(ZXY, Angles(1.57, 3.14, 2.5), offset=Vector(0, 1, 1))
    >>>
    >>> vector = Vector(1, 1, 1)
    >>> fromAToB = frameA.rotateToFrame(frameB, vector)

The following example shows four ways to compute the same rotated vector:

.. code-block:: python

    >>> vector = Vector(1, 2, 3)
    >>> order = XYZ
    >>> angles = Angles(1.0, 2.0, 3.0)
    >>> matrix = getEulerMatrix(order, angles)
    >>>
    >>> # manually rotate to reference frame
    >>> rotatedVector = vector @ matrix
    >>> # use method
    >>> rotatedVector = rotateMatrixTo(matrix, vector)
    >>> # use method without needing matrix
    >>> rotatedVector = rotateEulerTo(order, angles, vector)
    >>> # use ReferenceFrame object
    >>> frame = ReferenceFrame(order, angles)
    >>> rotatedVector = frame.rotateTo(vector)