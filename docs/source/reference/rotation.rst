.. include:: /global.rst

======================
Rotation API Reference
======================

.. py:currentmodule:: pyevspace

About Rotations
===============

Rotations are ways of moving between two reference frames. A reference frame
is a set of coordinate axes, which in a vector space define the vector values
for specific positions. Different systems require different reference frames
for describing things inside them, and it is quite common to want to know how
the same direction "looks", or what the coordinates of the same point in space
are in another reference frame.

PyEVSpace provides the means for defining reference frames relative to one
another, and a set of functions for applying the rotations to a vector using
these definitions.

.. todo: should probably talk about active vs passive rotations here

Rotation Types
==============

Here we describe several variables and types that allows us to
define rotations.

Axis Variables
--------------

Axes are designated by enumerated values. These are implemented
as integer constants at the module level, and represent the x,
y, and z axes of any reference frame.

.. py:data:: X_AXIS

    :value: 0
    :type: int

.. py:data:: Y_AXIS

    :value: 1
    :type: int

.. py:data:: Z_AXIS

    :value: 2
    :type: int

It is valid to use integer literals in place of one of these
variables, which may be useful for dynamically selecting axes
within a program. It is still recommended to use the above
variables when possible so that intent of code is more explicit.

RotationOrder Type
------------------

.. py:class:: RotationOrder(first: int, second: int, third: int)

    For muli-axis rotations, namely Euler rotations, the RotationOrder
    type is used to declare which, and in what order individual rotations
    are applied. For example, a rotation which should first revolve around
    the x-axis, then the y-axis, and finally the z-axis would use the
    :data:`XYZ` rotation order variable. PyEVSpace defines the twelve common
    `rotation orders <https://en.wikipedia.org/wiki/Euler_angles#Classic_Euler_angles>`_
    at the module level, which are all RotationOrder instances.

    While new RotationOrder variables can be instantiated, the :ref:`global order <order_globals>`
    instances should be prefered.

    :param first: the first axis of the rotation
    :param second: the second axis of the rotation
    :param third: the third axis of the rotation
    :raises TypeError: if any argument is not an :class:`int` type
    :raises ValueError: if any argument is not in the range [0-2]

Other Constructors
^^^^^^^^^^^^^^^^^^

.. py:method:: RotationOrder.__new__(cls: type) -> RotationOrder

    Creates a new uninitialized RotationOrder object. Because there is
    no initialization, each axis defaults to :data:`X_AXIS`.

    :param cls: the type of the variable to create
    :raises TypeError: if `cls` is not a subtype of RotationOrder

.. py:method:: RotationOrder.__init__(first: int, second: int, third: int)

    Initialize a RotationOrder to the axes parameters.

    :param first: the first axis of the rotation
    :param second: the second axis of the rotation
    :param third: the third axis of the rotation
    :return: a new RotationOrder object
    :raises TypeError: if any argument is not an :class:`int` type
    :raises ValueError: if any argument is not in the range [0-2]

Instance Methods
^^^^^^^^^^^^^^^^

.. py:method:: RotationOrder.__len__() -> int

    Returns the length of the container, which is always `3`.

    :return: this method always returns `3`

.. py:method:: RotationOrder.__getitem__(index: int) -> int

    Returns the axis of the order depending on `index`.

    :param index: the index of the axis to retrieve
    :return: the axis at `index`
    :raises IndexError: if `index` is not an :class:`int` type or
        doesn't provide :meth:`__index__`
    :raises ValueError: if `index` is not in the range [0-2] (after
        adjusting for negative indexing)

.. py:method:: RotationOrder.__repr__() -> str

    Returns a string representation of the order representative
    of a constructor call.

    :return: a string representation of `self`

.. py:method:: RotationOrder.__str__() -> str

    Returns a string representation of the order that looks
    similar to a :class:`list` of axis values.

    :return: a string representation of `self`

.. py:method:: RotationOrder.__hash__() -> int

    Because RotationOrder isntances are immutable, they can be hashed
    to support use in situations that require hashable types.

    :return: a hash of the rotation order

.. py:method:: RotationOrder.__reduce__()

    Provides support for pickling and copying.

    :return: a tuple containing the type and internal state of `self`
    :rtype: tuple[type, tuple[int, int, int]]

Logical Operators
^^^^^^^^^^^^^^^^^

.. py:method:: RotationOrder.__eq__(other: RotationOrder) -> bool

    Compares the axis values of two orders for equality.

    :return: True if each respective axis in `self` and `other` are equal,
        False otherwise.

.. py:method:: RotationOrder.__ne__(other: RotationOrder) -> bool

    Compares the axis values of two orders for inequality.

    :return: True if any respective axis in `self` and `other` are different,
        False otherwise

Properties
^^^^^^^^^^

.. py:property:: RotationOrder.first

    The first axis of rotation in the rotation order.

    :type: int
    :readonly: this value cannot be changed

.. py:property:: RotationOrder.second

    The second axis of rotation in the rotation order.

    :type: int
    :readonly: this value cannot be changed

.. py:property:: RotationOrder.third

    The third axis of rotation in the rotation order.

    :type: int
    :readonly: this value cannot be changed

.. _order_globals:

Global Variables
^^^^^^^^^^^^^^^^

.. py:data:: XYZ

    :type: RotationOrder
    :value: RotationOrder(X_AXIS, Y_AXIS, Z_AXIS)

.. py:data:: XZY

    :type: RotationOrder
    :value: RotationOrder(X_AXIS, Z_AXIS, Y_AXIS)

.. py:data:: YXZ

    :type: RotationOrder
    :value: RotationOrder(Y_AXIS, X_AXIS, Z_AXIS)

.. py:data:: YZX

    :type: RotationOrder
    :value: RotationOrder(Y_AXIS, Z_AXIS, X_AXIS)

.. py:data:: ZXY

    :type: RotationOrder
    :value: RotationOrder(Z_AXIS, X_AXIS, Y_AXIS)

.. py:data:: ZYX

    :type: RotationOrder
    :value: RotationOrder(Z_AXIS, Y_AXIS, X_AXIS)

.. py:data:: XYX

    :type: RotationOrder
    :value: RotationOrder(X_AXIS, Y_AXIS, X_AXIS)

.. py:data:: XZX

    :type: RotationOrder
    :value: RotationOrder(X_AXIS, Z_AXIS, X_AXIS)

.. py:data:: YXY

    :type: RotationOrder
    :value: RotationOrder(Y_AXIS, X_AXIS, Y_AXIS)

.. py:data:: YZY

    :type: RotationOrder
    :value: RotationOrder(Y_AXIS, Z_AXIS, Y_AXIS)

.. py:data:: ZXZ

    :type: RotationOrder
    :value: RotationOrder(Z_AXIS, X_AXIS, Z_AXIS)

.. py:data:: ZYZ

    :type: RotationOrder
    :value: RotationOrder(Z_AXIS, Y_AXIS, Z_AXIS)

EulerAngles
-----------

.. py:class:: EulerAngles()
    EulerAngles(alpha: Real, beta: Real, gamma: Real)

    An EulerAngles object is a container class for angles, used in
    conjunction with the :class:`RotationOrder`` type. Each angle
    (alpha, beta, and gamma) describes the angle of rotation for
    each respective axis in a :class:`RotationOrder`.

    The EulerAngles type is a mutable type, as it's common for rotations
    to need to be continually updated for rotating reference frames.

    If no parameters are provided to the constructor all values are
    initialized to `0.0`. The constructor only accepts three or zero
    parameters, all unneeded angles should explicitly set to `0.0`.

    :param alpha: the first angle of the rotation
    :param beta: the second angle of the rotation
    :param gamma: the third angle of the rotation
    :raises TypeError: if `alpha`, `beta`, or `gamma` are not :class:`Real`
        types

Other Constructors
^^^^^^^^^^^^^^^^^^

.. py:method:: EulerAngles.__new__(cls: type) -> EulerAngles

    Creates an uninitialized :class:`EulerAngles` instance. Each angle
    defaules to `0.0`.

    :param cls: type of the instance to create
    :return: a new EulerAngles object
    :raises TypeError: if `cls` is not a subtype of :class:`EulerAngles`

.. py:method:: EulerAngles.__init__()
    EulerAngles.__init__(alpha: Real, beta: Real, gamma: Real)

    Initializes an :class:`EulerAngles` variable to the specified angles.
    If no angles are provided they default to `0.0`. This method only
    accepts three or zero parameters, all unneeded angles shoulld
    explicitly set to `0.0`.

    :param alpha: the first angle of the rotation
    :param beta: the second angle of the rotation
    :param gamma: the third angle of the rotation
    :raises TypeError: if `alpha`, `beta`, or `gamma` are not :class:`Real`
        types

Instance Methods
^^^^^^^^^^^^^^^^

.. py:method:: EulerAngles.__len__() -> int

    Returns the length of the container, which is always `3`.

    :return: this method always returns `3`

.. py:method:: EulerAngles.__getitem__(index: int) -> float

    Returns the indexed angle of the container.

    :param index: the index of the angle to retrieve
    :return: the indexed angle
    :raises TypeError: if `index` is not an :class:`int` type
        and does not provided :meth:`__index__`
    :raises IndexError: if `index` is not in the range [0-2] (after
        adjusting for negative indexing)

.. py:method:: EulerAngles.__setitem__(index: int, value: Real) -> Real

    Sets the angle at `index` to `value`.

    :param index: the index of the angle to set
    :param value: the value to set the indexed angle
    :return: `value`
    :raises TypeError: if `index` is not an :class:`int` type and does
        not provide :meth:`__index__`
    :raises TypeError: if `value` is not a :class:`Real` type
    :raises IndexError: if `index` is not in the range [0-2] (after
        adjusting for negative indexing)

.. py:method:: EulerAngles.__repr__() -> str

    Returns a string representation of `self`, representative of a
    contructor call.

    :return: a string representation of `self`

.. py:method:: EulerAngles.__str__() -> str

    Returns a string representation of `self`, similar to a :class:`list`
    of :class:`float` s.

    :return: a string representation of `self`

.. py:method:: EulerAngles.__reduce__()

    Provides support for pickling and copying.

    :return: a tuple containing the type and internal state of `self`
    :rtype: tuple[type, tuple[float, float, float]]

Explaining Relative Reference Frames
====================================

In PyEVSpace, we need to ability to define a reference frame relative
to another. In the strictest definition of the word, an *inertial*
reference frame is a **non-accelerating** reference frame, either
at rest or moving at a constant velocity. In this documentation we
will use *inertial* to mean a base reference frame. Now, a
reference frame that fits this description for your project may be
an entirely different reference frame in another project, but they
both fit this definition! Every single reference frame will be
different **relative** to another, but as long as you're consistent
within the scope of your project, this will work.

We have one final, hopefully not *too* confusing point to make. The
same rotation between two non-inertial reference frames, and a rotated
reference frame relative to an inertial frame produces the same
rotation matrix. The key takeaway here is that as we define reference
frames, they are always relative to another, consistent reference frame.
A *reference* reference frame, if you'd like.

.. code-block:: python

    >>> # Define reference frame A as a 90 degree rotation around
    >>> # an inertial frame's x-axis:
    >>> frame_a = compute_rotation_matrix(math.pi / 2, X_AXIS)
    >>> # define refrence frame B as two 90 degree rotations around
    >>> # the x-axis, then y-axis relative to an inertial frame:
    >>> angles = EulerAngles(math.pi / 2, math.pi / 2, 0)
    >>> frame_b = compute_rotation_matrix(XYZ, angles)
    >>>
    >>> # here, frame_b only differs by the final y-axis rotation,
    >>> # therefore to define a rotation matrix moving from
    >>> # frame_a -> frame_b would just be a 90 degree rotation
    >>> # around the y-axis of frame_a. Knowing how these frames
    >>> # differ relative to a truely inertial frame is not needed:
    >>> relative_matrix = compute_rotation_matrix(math.pi / 2, Y_AXIS)

The above code snippet demonstrates that even though a reference frame
may not truely be inertial, in the context of this documentation, we can
call it inertial in order to define our second frame.

Generating Rotation Matrices
============================

Generating rotation matricies is as simple as passing the correct
arguments to :func:`compute_rotation_matrix`. Rotation matricies
can be derived from:

- single axis rotations, defined by a coordinate axis or a vector
- multi-axis Euler rotations, involving three successive rotations

Defining A Reference Frame
--------------------------

Each reference frame is defined by the parameters passed to the
:func:`compute_rotation_matrix` (similar to the *rotate_*()* methods).
The reference frame is defined **relative** to another frame. As explained
above, you only need to define the relative rotation between the two
frames. The possible ways to define a rotation are

- angle, axis (enumerated value)
- angle, axis (an axis defined by a vector)
- euler order, euler angles (can be intrinsic or extrinsic)
- between two non-inertial reference frames using euler orders and euler angles

Any time a rotation involves only two axis rotations (instead of three), a
:class:`RotationOrder` instance should be chosen where the third axis can
be any axis, and will become redundant by setting the cooresponding
:class:`EulerAngles` third angle to `0.0`.

Intrinsic Vs. Extrinsic
-----------------------

Intrinsic and extrinsic rotations mostly apply to Euler rotations. An
*intrinsic* rotation is a rotation whose subsequent rotation axes are
applied to the rotated frame. For example, in an XYZ intrinsic
rotation, the coordinate axes are rotated around the x-axis, then next
rotation is applied to where the y-axis ends up due to the first rotation.
Again, the final z-axis rotation is applied where the z-axis ends up after
the first two rotations.

*Extrinsic* rotations are rotations applied to all axes in the original
inertial reference frame, not where the axes end up after preceeding
rotations. While intrinsic rotations are generally more easy to define
in real world systems, extrinsic matrices can still be computed, but
:func:`compute_rotation_matrix` defaults to intrinsic rotations.

Computing Matricies
-------------------

.. py:function:: compute_rotation_matrix(angle: Real, axis: int | Vector) -> Matrix
    compute_rotation_matrix(order: RotationOrder, angles: EulerAngles, *, intrinsic: bool = True) -> Matrix
    compute_rotation_matrix(order_from: RotationOrder, angles_from: EulerAngles, order_to: RotationOrder,\
        angles_to: EulerAngles, *, intrinsic_from: bool = True, intrinsic_to: bool = True) -> Matrix

    Computes a rotation matrix based on the parameter types and values.
    The first form rotates a coordinate system around an enumerated axis
    or a :class:`Vector` object. The second form produces a matrix between
    a reference frame defined by a set of :class:`RotationOrder` and
    :class:`EulerAngles`. This form defaults to an intrinsic rotation,
    and can be controlled by the `intrinsic` keyword.

    Rotations only need to be defined from one reference frame relative
    to another, however in practice it may be very difficult to simplify
    the relative rotations. For moving **from** one reference frame **to**
    another, a matrix can be produced to "undo" the rotation of the **from**
    frame to an inertial one, and another matrix to move from the inertial
    frame to the **to** reference frame. These matrices can be combined
    into a single matrix, and is what is returned by the third form of
    this function.

    :param angle: angle for a single axis rotation in radians
    :param axis: single axis rotation axis as an enumerated value or a
        Vector type
    :param order: the rotation order for a simple Euler rotation
    :param angles: the Euler angles for a simple Euler rotation
    :param order_from: the rotation order moving from in complex Euler rotations
    :param angles_from: the Euler angles moving from in complex Euler rotations
    :param order_to: the rotation order moving to in complex Euler rotations
    :param angles_from: the Euler angles moving to in complex Euler rotations
    :keyword intrinsic: True (default) for an intrinsic Euler rotation, False
        for an extrinsic Euler rotation 
    :keyword intrinsic_from: True (default) for intrinsic Euler rotation moving from
        in complex Euler rotations, False for an extrinsic rotation
    :keyword intrinsic_to: True (default) for intrinsic Euler rotation moving to
        in complex Euler rotations, False for an extrinsic rotation
    :return: the rotation matrix
    :raises TypeError: if any parameters are not their explicit type
    :raises ValueError: if `axis` is provided as an integer but is not in the range of [0-2]

Rotation Functions
==================

The following rotation functions abstract away the logic for applying
rotation matrices to vectors. The nomenclature is designed to simplify
the thinking of how matrix-vector multiplication needs to be applied.
The function :func:`rotate_from` rotates a vector **from** a rotated
reference frame to an inertial frame.  The function :func:`rotate_to`
rotates a vector from an inertial reference frame **to** a rotated
reference frame.  The key note here is the suffix describes the rotated
frame: *rotate_from* is **from** the rotated frame, *rotate_to* is **to**
the rotated frame.

For reference frames that are difficult to define relative to another,
but easier to define relative to an inertial frame, the :func:`rotate_between`
function allows you to provide both reference frame definitions, and
a single matrix is produced that will move vectors between these
reference frames.

The origin of two reference frames may also have offset origins. When
using :func:`rotate_to` and :func:`rotate_from` the :python:`offset`
keyword can be used to define the offset vector, which points from
the inertial frame to the rotated frame. In the :func:`rotate_between`
function, use the :python:`offset_from` or :python:`offset_to` keyword
arguments to achieve the same thing for the respective reference frames.

.. note::

    The :python:`offset` keywords do not follow the **to**, **from**
    nomenclature. The offset vector **ALWAYS** points from the inertial
    reference frame's origin to the rotated reference frame's origin.

.. py:function:: rotate_from(matrix: Matrix, vector: Vector, *, offset: Optional[Vector] = None) -> Vector
    rotate_from(angle: Real, axis: int | Vector, vector: Vector, *, offset: Optional[Vector] = None) -> Vector
    rotate_from(order: RotationOrder, angles: EulerAngles, vector: Vector, *, intrinsic: bool = True,\
        offset: Optional[Vector] = None) -> Vector

    Rotates a vector **from** a rotated reference frame, to an inertial
    reference frame. The first form takes an already produced rotation
    matrix and applies it to `vector`, accounting for any difference
    in the origin defined by `offset`. This version of the function is
    useful for applying the same rotation to multiple vectors, as the
    rotation matrix isn't generated multiple times when using the other
    versions of this function.

    The second form defines a rotation by a single angle rotation around
    an axis, which can be either an enumerated axis value or by an axis
    defined by a :class:`Vector` instance.

    The second form defines an Euler rotation using a :class:`RotationOrder`
    and :class:`EulerAngles`. In this case the `intrinsic` keyword is used to define
    an intrinsic or extrinsic rotation.

    :param matrix: a pre-computed rotation matrix
    :param angle: the angle of a single axis rotation
    :param axis: the axis of a single axis rotation
    :param order: the order for an Euler rotation
    :param angles: the angles for an Euler rotation
    :param vector: the vector to rotate
    :keyword offset: an optional vector that points from an inertial reference
        frame's origin to the rotated reference frame's origin
    :keyword intrinsic: defines an intrinsic (default) or extrinsic Euler rotation
    :return: the rotated vector
    :raises TypeError: if any parameters are not their explicit type
    :raises ValueError: if `axis` is provided as an integer but is not in the range of [0-2]

.. py:function:: rotate_to(matrix: Matrix, vector: Vector, *, offset: Optional[Vector] = None) -> Vector
    rotate_to(angle: Real, axis: int | Vector, vector: Vector, *, offset: Optional[Vector] = None) -> Vector
    rotate_to(order: RotationOrder, angles: EulerAngles, vector: Vector, *, intrinsic: bool = True,\
        offset: Optional[Vector] = None) -> Vector

    Rotates a vector from an inertial reference frame, **to** a rotated
    reference frame. The first form takes an already produced rotation
    matrix and applies it to `vector`, accounting for any difference
    in the origin defined by `offset`. This version of the function is
    useful for applying the same rotation to multiple vectors, as the
    rotation matrix isn't generated multiple times when using the other
    versions of this function.

    The second form defines a rotation by a single angle rotation around
    an axis, which can be either an enumerated axis value or by an axis
    defined by a :class:`Vector` instance.

    The second form defines an Euler rotation using a :class:`RotationOrder`
    and :class:`EulerAngles`. In this case the `intrinsic` keyword is used to define
    an intrinsic or extrinsic rotation.

    :param matrix: a pre-computed rotation matrix
    :param angle: the angle of a single axis rotation
    :param axis: the axis of a single axis rotation
    :param order: the order for an Euler rotation
    :param angles: the angles for an Euler rotation
    :param vector: the vector to rotate
    :keyword offset: an optional vector that points from an inertial reference
        frame's origin to the rotated reference frame's origin
    :keyword intrinsic: defines an intrinsic (default) or extrinsic Euler rotation
    :return: the rotated vector
    :raises TypeError: if any parameters are not their explicit type
    :raises ValueError: if `axis` is provided as an integer but is not in the range of [0-2]

.. py:function:: rotate_between(order_from: RotationOrder, angles_from: EulerAngles, order_to: RotationOrder,\
    angles_to: EulerAngles, vector: Vector, *, intrinsic_from: bool = True, intrinsic_to: bool = True, offset_from: Optional[Vector] = None, offset_to: Optional[Vector] = None) -> Vector

    Rotates a vector between two reference frames. Each reference frame
    is defined relative to an intermediate inertial frame, which may be easier
    to define. All parameters behave the same as the related
    :func:`rotate_from` and :func:`rotate_to` functions.

    If one of the reference frames may be a single axis rotation, it still
    must be defined using Euler rotation semantics. For example, for a frame
    defined by an angle :python:`a` around the x-axis, use any of the
    :class:`RotationOrder` objects that start with an `X`, like :python:`XYZ`,
    :python:`XZY`, etc. and an :class:`EulerAngles` object with only the
    first angle being non-zero: :python:`EulerAngles(a, 0.0, 0.0)`.

    :param order_from: the rotation order of the Euler rotation moving from
    :param angles_from: the angles of the Euler rotation moving from
    :param order_to: the rotation order of the Euler rotation moving to
    :param angles_to: the angles of the Euler rotation moving to
    :param vector: the vector to rotate
    :keyword intrinsic_from: determines if the rotation from is intrinsic (default) or extrinsic
    :keyword intrinsic_to: determines if the rotation to is intrinsic (default) or extrinsic
    :keyword offset_from: offset of the from reference frame origin relative to the inertial frame (default is None)
    :keyword offset_to: offset of the to reference frame origin relative to the inertial frame (default is None)
    :return: the rotated vector
    :raises TypeError: if any parameters are not their explicit type
