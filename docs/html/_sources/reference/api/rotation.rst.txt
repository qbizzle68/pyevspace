.. include:: /global.rst

======================
Rotation API Reference
======================

.. py:currentmodule:: pyevspace

.. contents:: Table of Contents
    :backlinks: none

The rotation portion of the module provides methods for generating
rotation matrices, as well as methods for handling the rotations.

The getMatrix* methods return a matrix based on specific rotation
types: axis, Euler and from one reference frame to another.

The rotate* methods work similarly to the getMatrix* methods, but
they handle deriving the necessary matrices and how to use them
internally. This allows the most inexperienced user to safely
rotate vectors between reference frames and keeps code clean and
concise.

How Rotations Work
------------------
Pure rotations work like a map from a vector space to itself. If the
original reference frame is our normal x, y, z coordinate frame, a
rotated vector is that original vector expressed with a differenct
set of *basis* vectors.

Basis Vectors
^^^^^^^^^^^^^
Simply speaking, basis vectors are the set of vectors that allow us to
represent every vector in a vector space. For a Euclidean vector space
these are usually the unit vectors pointing towards each positive axis,
namely (1, 0, 0), (0, 1, 0) and (0, 0, 1), also called e1, e2 and e3
respectively. Any vector in our vector space then, can be written as a 
linear combination of our basis vectors: (a, b, c) = e1 * a + e2 * b + 
e3 * c. Other sets of basis vectors also exist, as well show shortly.

Rotation Matrix
^^^^^^^^^^^^^^^
A rotation matrix is simply a mapping of vectors. When right multiplying
a matrix by a vector, we create a new vector that is a linear combination
of the columns of our matrix (called the column space). In essence, we can
think of these column vectors as a basis for a rotated reference frame in
the same vector space. 

For a simple example, think of the identity matrix as a rotation. Of course
we know this is a rotation of zero radians, since no vector will be changed
by multiplying by the identity matrix. This can be demonstrated by looking
at the identity matrix's columns:

.. code-block:: python
    
    >>> Matrix.id
    Matrix([1, 0, 0]
        [0, 1, 0]
        [0, 0, 1])

This first column, which represents where
the x-axis is mapped to, is the x-axis (1, 0, 0). The second and third columns
show the y and z axes also map to themselves, which is why each vector remains
the same after being multiplied by the identity matrix.

A more complex example can be seen using the Euler rotation around the axes
X, Y and Z, with angles 1.0, 2.0 and 3.0 radians. This yields the rotation
matrix:

.. code-block:: python

    >>> getMatrixEuler(XYZ, Angles(1.0, 2.0, 3.0))
    Matrix([0.411982, 0.0587266, 0.909297]
        [-0.681243, -0.642873, 0.350175]
        [0.605127, -0.763718, -0.224845])

Here the x-axis maps to the vector (0.411982, -0.681243, 0.605127). The y and z
axes map to the second and third column vectors respectively. Because these 
vectors are only rotations of our original axis vectors, they remain orthogonal
to each other, meaning they are linearly independent, and since there's three of
them in a 3-dimensional vector space, they form a basis. 

The significance of
this basis is it represents how the three axes of the reference frame defined
by the Euler rotation are represented in our original basis. Therefore, by
multiplying a vector from the rotated reference frame by our rotation matrix,
we get a representation of that vector in the original basis. *This is
equivalent to rotating the vector from the rotated reference frame, to the
original reference frame.* Simply put, if we multiply our vectors on the right,
the rotation matrix tells us how to move vectors from a rotated reference frame
to the unrotated reference frame.

Overly simplifying a few things, the rotation matrix that describes how to move
the other direction between reference frames is simply the inverse of the
original rotation matrix. Luckily, the inverse of a pure rotation matrix is its
transpose, which is simple to compute. Therefore, multiplying a vector from our
original reference frame on the right by the *transpose* of our matrix from
above, rotates said vector *to* the rotated reference frame.

Speeding Things Along
^^^^^^^^^^^^^^^^^^^^^
As you may have already noticed, multiplying the transpose of a matrix on the
right generates the same output as if we multiplied the original matrix on the
left. This is good for us, because it means we don't need to transpose our
matrices, which otherwise would add more time to our computations. In summary,
for an unrotated reference frame *A*, if we have a pure rotation matrix that
describes a mapping to reference frame *B*, multiplying the matrix by a
vector on the right rotates the vector from B to A. Multiplying the matrix by
a vector on the left rotates the vector from A to B. Note the opposite is true
if you inverse (transpose) the matrix.

If M is a matrix representing a rotation from reference frame A to reference
frame B:

================= =========== ======================
Mapping Direction Matrix Form Operation
================= =========== ======================
A -> B            normal      vB = vA @ M
A -> B            transposed  vB = transpose(M) @ vA
B -> A            normal      vA = M @ vB
B -> A            transposed  vA = vB @ transpose(M)
================= =========== ======================

While several reference frame mappings exist, as long as they are pure rotations
(no offset origins) a single matrix is enough describe the mapping. This allows
us to use the rules above to rotate any vector using the matrix returned by one
of the getMatrix* methods. To simplify all of this, PyEVSpace implements a
collection of methods to handle all of the above for you. The rotate*To and
rotate*From methods remove all of the complexities of rotations, while also
eliminating the need for you to create a matrix yourself. While you now know how
to rotate vectors yourself, it is advised to use these methods for cleaner code,
to reduce errors and faster execution of the rotation.


Generating Rotation Matrices
----------------------------

The pyevspace module provides the following methods for generating
rotation matrices.

========================== =================================== ======================================
Method                     Rotation Type                       Reference
========================== =================================== ======================================
:py:meth:`getMatrixAxis`   Rotation around a single axis       `Rotation Matrix
                                                               <https://en.wikipedia.org/wiki/
                                                               Rotation_matrix#In_three_dimensions>`_
:py:meth:`getMatrixEuler`  Three axis Euler rotation           `Euler Rotation <https://en.wikipedia
                                                               .org/wiki/Euler_angles>`_
:py:meth:`getMatrixFromTo` From one reference frame to another ---
========================== =================================== ======================================

Rotating Vectors
----------------

The following methods are used to rotate vectors without needing to
generate a rotation matrix yourself. Each method 'type' refers to a
rotation type, and has two related methods, a 'from' and a 'to' method.

.. note::

    An inertial reference frame refers to an un-rotated and constant
    reference frame. In theory, this need not be exactly true, only
    true relative to the derived reference frame rotating from or to.
    In most cases, the corresponding rotation matrix for the inertial
    reference frame is the identity matrix: :py:attr:`Matrix.id`.

=========================== ================================= =================================
Method                      From Reference Frame              To Reference Frame
=========================== ================================= =================================
:py:meth:`rotateAxisFrom`   The axis and angle specified      An inertial reference frame
:py:meth:`rotateAxisTo`     An inertial reference frame       The axis and angle specified
:py:meth:`rotateEulerFrom`  The frame resulting from the      An inertial reference frame
                            order and angles specified
:py:meth:`rotateEulerTo`    An inertial reference frame       The frame resulting from the
                                                              order and angles specified
:py:meth:`rotateMatrixFrom` The frame described by the matrix An inertial reference frame
:py:meth:`rotateMatrixTo`   An inertial reference frame       The frame described by the matrix
:py:meth:`rotateOffsetFrom` The offset frame specified        An inertial reference frame
:py:meth:`rotateOffsetTo`   An inertial reference frame       The offset frame specified
=========================== ================================= =================================

Rotation Types
--------------

Enumerated Axes
^^^^^^^^^^^^^^^

A few types are needed to help describe an Euler rotation. An Euler rotation
consists of an order of axes about which consecutive axis rotations occur,
and the angles by which they're rotated.

To specify an axis around which a rotation occurs, there are three
enumerated constants.

.. py:data:: X_AXIS

    Enumerated constant to specify the X_AXIS.

    :type: int
    :value: 0

.. py:data:: Y_AXIS

    Enumerated constant to specify the Y_AXIS.

    :type: int
    :value: 1

.. py:data:: Z_AXIS

    Enumerated constant to specify the Z_AXIS.

    :type: int
    :value: 2

Order
^^^^^

There are 12 unique Euler rotation orders, which are already defined, so it
is unlikely to need to instantiate another :py:class:`Order` object, however
the class is still documented below.

.. py:class:: Order(firstAxis, secondAxis, thirdAxis)

    A class that specifies the order of axes in an Euler rotation. The
    axis arguments are technically int values, but the enumerated axis
    values should be used to avoid confusion and made code cleaner and
    easier to read.

    :param int firstAxis: the first axis of an Euler rotation
    :param int secondAxis: the second axis of an Euler rotation
    :param int thirdAxis: the third axis of an Euler rotation

Attributes
""""""""""

.. py:attribute:: Order.first

    The first axis of the rotation order.

    :type: int

.. py:attribute:: Order.second

    The second axis of the rotation order.

    :type: int

.. py:attribute:: Order.third

    The third axis of the rotation order.

    :type: int

Instance Methods
""""""""""""""""

.. py:method:: Order.__len__()

    Returns the length of the order for sequencing.

    :return: always returns 3
    :rtype: int

.. py:method:: Order.__getitem__(index)

    Returns the indexed axis.

    :param int index: the index of the order to get
    :raises TypeError: if index is not an int type
    :raises IndexError: if index is not in [0, 2]
    :return: the indexed axis
    :rtype: int

.. py:method:: Order.__setitem__(index, value)

    to be removed?

.. py:method:: Order.__repr__()

    Converts the order to a string representation, representative
    of a constructor call.

    :return: a string representation of the order
    :rtype: str

.. py:method:: Order.__str__()

    Converts the order to a string representation.

    :return: a string representation of the order
    :rtype: str

.. py:method:: Order.__reduce__()

    Returns a tuple of objects capable of reconstructing an
    order object for copying and pickling.

    :return: a tuple with the constructor and rotations
    :rtype: tuple

Angles
^^^^^^

.. py:class:: Angles(alpha, beta, gamma)

    A class to hold the angles that correspond to the axes in the 
    :py:class:`Order` type. The angle corresponds to the respective
    axis in its associated order object.

    :param float alpha: the angle in radians around the first axis
    :param float beta: the angle in radians around the second axis
    :param float gamma: the angle in radians around the third axis

Attributes
""""""""""

.. py:attribute:: Angles.alpha

    The first angle of a rotation.

.. py:attribute:: Angles.beta

    The second angle of a rotation.

.. py:attribute:: Angles.gamma
    
    The third angle of a rotation.

Instance Methods
""""""""""""""""

.. py:method:: Angles.__len__()

    Returns the lengh of the angles for sequencing.

    :return: always returns 3
    :rtype: int

.. py:method:: Angles.__getitem__(index)

    Returns the indexed angle.

    :param int index: the index of the angle to get
    :raises TypeError: if index is not an int type
    :raises IndexError: if index is not in [0, 2]
    :return: the indexed angle
    :rtype: float

.. py:method:: Angles.__setitem__(index, value)

    Sets the indexed angle to value.

    :param int index: the index of the angle to get
    :param float value: the value to set the angle in radians
    :raises TypeError: if index is not an int type or value is not a 
        numeric type
    :raises IndexError: if index is not in [0, 2]

.. py:method:: Angles.__repr__()

    Converts the angles to a string representation, representative
    of a constructor call.

    :return: a string representation of the angles
    :rtype: str

.. py:method:: Angles.__str__()

    Converts the angles to a string representation.

    :return: a string representation of the angles
    :rtype: str

.. py:method:: Angles.__reduce__()

    Returns a tuple of objects capable of reconstructing an
    angles object for copying and pickling.

    :return: a tuple with the constructor and angles
    :rtype: tuple

.. _mat-gen-label:

Matrix Generator Methods
------------------------

.. py:function:: getMatrixAxis(axis, angle)

    Creates a matrix that describes a rotation around an axis by an angle.

    :param int axis: axis of rotation, should be one of the defined
        enumerated axes
    :param numeric angle: angle of rotation in radians
    :raises TypeError: if axis is not an int or angle is not a numeric type
    :raises ValueError: if axis is not in [0-2]
    :return: the rotation matrix
    :rtype: Matrix

.. py:function:: getMatrixEuler(order, angles)

    Creates a matrix that describes an Euler rotation by its order and the
    associated angles.

    :param Order order: the Euler order of rotation
    :param Angles angles: the angles associated with the order axes
    :raises TypeError: if order is not an Order type or angles is not an 
        Angles type
    :return: the rotation matrix
    :rtype: Matrix

.. py:function:: getMatrixFromTo(orderFrom, anglesFrom, orderTo, anglesTo)

    Creates a matrix that describes a rotation between two reference frames,
    where each frame is defined as an Euler rotation.

    :param Order orderFrom: the Euler order of the reference frame moving from
    :param Angles anglesFrom: the angles for the reference frame moving from
    :param Order orderTo: the Euler order of the reference frame moving to
    :param Angles anglesTo: the angles for the reference frame moving to
    :raises TypeError: if orderFrom or orderTo are not Order types, or
        anglesFrom or anglesTo are not Angles types
    :return: the rotation matrix
    :rtype: Matrix

.. _rotate-meth-label:

Rotation Methods
----------------

.. py:function:: rotateAxisFrom(axis, angle, vector)

    Rotates a vector from a reference frame defined by an axis rotation.

    :param int axis: axis of rotation, should be one of the defined
        enumerated axes
    :param float angle: angle of rotation in radians
    :param Vector vector: vector to rotate
    :raises TypeError: if axis is not an int, angle is not a float or 
        vector is not a Vector type
    :raises ValueError: if axis is not in [0, 2]
    :return: the rotated vector
    :rtype: Vector

.. py:function:: rotateAxisTo(axis, angle, vector)

    Rotates a vector to a reference frame defined by an axis rotation.

    :param int axis: axis of rotation, should be one of the defined
        enumerated axes
    :param float angle: angle of rotation in radians
    :param Vector vector: vector to rotate
    :raises TypeError: if axis is not an int, angle is not a float or 
        vector is not a Vector type
    :raises ValueError: if axis is not in [0, 2]
    :return: the rotated vector
    :rtype: Vector

.. py:function:: rotateEulerFrom(order, angles, vector)

    Rotates a vector from a reference frame defined by an Euler rotation.

    :param Order order: the Euler order of rotation
    :param Angles angles: the angles associated with the order axes
    :param Vector vector: the vector to rotate
    :raises TypeError: if order is not an Order type, angles are not an
        Angles type or vector is not a Vector type
    :return: the rotated vector
    :rtype: Vector

.. py:function:: rotateEulerTo(order, angles, vector)

    Rotates a vector to a reference frame defined by an Euler rotation.

    :param Order order: the Euler order of rotation
    :param Angles angles: the angles associated with the order axes
    :param Vector vector: the vector to rotate
    :raises TypeError: if order is not an Order type, angles are not an
        Angles type or vector is not a Vector type
    :return: the rotated vector
    :rtype: Vector

.. py:function:: rotateMatrixFrom(matrix, vector)

    Rotates a vector from a reference frame defined by a rotation matrix.

    :param Matrix matrix: the rotation matrix
    :param Vector vector: the vector to rotate
    :raises TypeError: if matrix is not a Matrix type or vector is not
        a Vector type
    :return: the rotated vector
    :rtype: Vector

.. py:function:: rotateMatrixTo(matrix, vector)

    Rotates a vector to a reference frame defined by a rotation matrix.

    :param Matrix matrix: the rotation matrix
    :param Vector vector: the vector to rotate
    :raises TypeError: if matrix is not a Matrix type or vector is not
        a Vector type
    :return: the rotated vector
    :rtype: Vector

.. py:function:: rotateOffsetFrom(matrix, offset, vector)

    Rotates a vector from an offset reference frame.

    :param Matrix matrix: the rotation matrix
    :param Vector vector: the vector to rotate
    :raises TypeError: if matrix is not a Matrix type or offset and vector
        are not Vector types
    :return: the rotated vector:
    :rtype: Vector

.. py:function:: rotateOffsetTo(matrix, offset, vector)

    Rotates a vector to an offset reference frame.

    :param Matrix matrix: the rotation matrix
    :param Vector vector: the vector to rotate
    :raises TypeError: if matrix is not a Matrix type or offset and vector
        are not Vector types
    :return: the rotated vector:
    :rtype: Vector