.. include:: /global.rst

=============================
ReferenceFrame API Reference
=============================

.. py:currentmodule:: pyevspace

.. contents:: Table of Contents
    :backlinks: none

The ReferenceFrame object allows the user to represent reference
frames without needing to keep track and correctly handle a
rotation matrix and offset vectors. The ReferenceFrame class
holds this information and handles rotating to and from the
reference frame for you.

.. py:class:: ReferenceFrame(order, angles[, *, offset])

    A class that holds all information for a reference frame, and
    methods to handling rotating between it and other frames.

    :param Order order: the Euler order for the rotation of the
        reference frame
    :param Angles angles: the associated angles for the Euler
        rotation of the reference frame
    :param Vector offset: optional vector when a reference frame's
        origin is offset 

Attributes
^^^^^^^^^^

.. py:attribute:: ReferenceFrame.order

    Read-only attribute of the Order object used to instantiate the
    ReferenceFrame.

    :type: Order

.. py:attribute:: ReferenceFrame.matrix

    Read-only attribute of the internal matrix used to compute vector
    rotations.

    :type: Matrix

.. py:attribute:: ReferenceFrame.angles

    The :py:class:`Angles` object used to instantate the 
    :py:class:`RefernceFrame`. Setting this value or any of its angle attributes
    will cause the internal matrix to be recomputed.
    
    .. code-block:: python

        >>> ref = ReferenceFrame(XYZ, Angles(1, 2, 3))
        >>> ref.matrix
        Matrix([0.411982, 0.0587266, 0.909297]
            [-0.681243, -0.642873, 0.350175]
            [0.605127, -0.763718, -0.224845])
        >>> ref.angles.alpha = 2
        >>> # ref.matrix updated
        >>> ref.matrix
        Matrix([0.411982, 0.0587266, 0.909297]
            [-0.877274, 0.295301, 0.378401]
            [-0.246294, -0.953598, 0.173178])

    :type: Angles

.. py:attribute:: ReferenceFrame.offset

    Get or set an offset vector for the reference frame and recompute the
    internal rotation matrix.

    :type: Vector

Instance Methods
^^^^^^^^^^^^^^^^

.. py:method:: ReferenceFrame.rotateTo(vector)

    Rotates vector to this reference frame.

    :param Vector vector: vector to be rotated
    :raises TypeError: if vector is not a Vector type
    :return: the rotated vector
    :rtype: Vector

.. py:method:: ReferenceFrame.rotateFrom(vector)

    Rotates vector from this reference frame.

    :param Vector vector: vector to be rotated
    :raises TypeError: if vector is not a Vector type
    :return: the rotated vector
    :rtype: Vector

.. py:method:: ReferenceFrame.rotateToFrame(frame, vector)

    Rotates vector from this reference frame to another reference frame.

    :param ReferenceFrame frame: reference frame to rotate vector to
    :param Vector vector: vector to be rotated
    :return: the rotated vector
    :rtype: Vector

.. py:method:: ReferenceFrame.rotateFromFrame(frame, vector)

    Rotates vector to this reference frame from another reference frame.

    :param ReferenceFrame frame: reference frame to rotate vector from
    :param Vector vector: vector to be rotated
    :return: the rotated vector
    :rtype: Vector