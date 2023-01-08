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