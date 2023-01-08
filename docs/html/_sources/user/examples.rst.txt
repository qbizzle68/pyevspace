PyEVSpace User Guide
====================

Examples
--------

Example #1
^^^^^^^^^^
How to find the vector that points from vector1 to vector2:

.. code-block:: python

    >>> from1To2 = vector2 - vector1

Example #2
^^^^^^^^^^
How to set a vector's length without changing its direction:

.. code-block:: python

    >>> length = 6.734
    >>> vector = Vector(1, 2, 3)
    >>>
    >>> # new vector
    >>> newVector = norm(vector) * length
    >>>
    >>> # change vector
    >>> normalize(vector)
    >>> vector *= length

Example #3
^^^^^^^^^^
Rotate vector to another reference frame

.. code-block:: python

    >>> angles = Angles(pi / 2, 1.0, 5.74)
    >>> matrix = getMatrixEuler(XYX, angles)
    >>>
    >>> # 'manually'
    >>> rotatedVector = transpose(matrix) * vector
    >>>
    >>> # using pyevspace methods
    >>> rotatedVector = rotateMatrixTo(matrix, vector)
    >>> # can also do without intermediate matrix
    >>> rotatedVector = rotateEulerTo(XYX, angles, vector)

Example #4
^^^^^^^^^^
Same as example #3 but with a ReferenceFrame object

.. code-block:: python

    >>> angles = Angles(pi / 2, 1.0, 5.74)
    >>> refFrame = ReferenceFrame(XYX, angles)
    >>> rotatedVector = refFrame.rotateTo(vector)