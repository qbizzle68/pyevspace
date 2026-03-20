.. include:: /global.rst

=============================
ReferenceFrame API Reference
=============================

.. py:currentmodule:: pyevspace

The ReferenceFrame Type
=======================

The :class:`ReferenceFrame` type allows for a reference frame definition
within a single object. The :class:`ReferenceFrame` type also provides
rotation methods similar to the :func:`rotate_from`, :func:`rotate_to`,
and :func:`rotate_between` functions. 

The :class:`ReferenceFrame` type holds an internal rotation matrix that
is easily updatable via updating the rotation angles. This means the
:class:`ReferenceFrame` type is ideal for handling many rotations as
the type easily persists, updates, and applies the rotation matrix
for you.

.. py:class:: ReferenceFrame(order: RotationOrder, angles: EulerAngles, *, intrinsic: bool = True, offset: Optional[Vector] = None)

    Holds the definition of a reference frame and assists in rotating
    vectors to and from, and even between other ReferenceFrame objects.

    The ReferenceFrame type should be thought of just like reference
    frames discussed in the :doc:`rotation API <rotation>`. The special
    function of this type is to provide the arguments to the constructor
    once, and simply manage any angle adjustments using the :meth:`set_angles`
    method and efficiently apply the rotations with the various *rotate_*()*
    methods provided.

    .. versionchanged:: 0.16.0
        ReferenceFrame is now subclassable, and is no longer an immutable type,
        meaning class members can be directly modified or added.

    :param order: the Euler order of the rotation of the reference frame
    :param angles: the Euler angles of the rotation of the reference frame
    :keyword intrinsic: True (default) for an intrinsic rotation, False for extrinsic
    :keyword offset: optionally define an offset of the origin relative to
        an inertial frame (default = :data:`None`)
    :raises TypeError: if any of the parameters do not have their specified type

Other Constructors
------------------

.. py:method:: ReferenceFrame.__new__(cls: type) -> Self

    Create a new, uninitialized `type` object if `type` is a subclass of
    :class:`ReferenceFrame`.

    .. versionchanged:: 0.16.0
        `type` can be a subtype of :class:`ReferenceFrame`, and an object of that
        type will be created and returned.

    .. versionchanged:: 0.16.0
        The returned value is no longer initialized, and :meth:`ReferenceFrame.__init__`
        should be used to initialize it. The default order is :data:`XYZ`, the angles
        are defaulted to zero and offset is set to :data:`None`.

.. py:method:: ReferenceFrame.__init__(self, order: RotationOrder, angles: EulerAngles, *,\
    intrinsic: bool = True, offset: Optional[Vector] = None)

    Initializes a :class:`ReferenceFrame` to the argument values. This method and arguments
    behave identically to the :class:`ReferenceFrame` constructor.

    :param order: the Euler order of the rotation of the reference frame
    :param angles: the Euler angles of the rotation of the reference frame
    :keyword intrinsic: True (default) for an intrinsic rotation, False for extrinsic
    :keyword offset: optionally define an offset of the origin relative to
        an inertial frame (default = :data:`None`)
    :raises TypeError: if any of the parameters do not have their specified type

    .. versionadded:: 0.16.0

Subclassing :class:`ReferenceFrame`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As of version `0.16.0` the :class:`ReferenceFrame` class supports subclassing. As the
:class:`ReferenceFrame` class is implemented as a C struct, it has a specific memory
layout, and as a consequence of that it is a :deco:`distjoint-base`. This
means :class:`ReferenceFrame` cannot be combined with any other types whose layout
is defined as a C struct when inheriting, which most Python built-in types
such as :class:`list` or :class:`tuple` are. For example combining :class:`ReferenceFrame`
and :class:`list` as base classes will raise a :exc:`TypeError`:

.. code-block:: python

	>>> class Foo(ReferenceFrame, list):
	...     pass
	...
	TypeError: multiple bases have instance lay-out conflict

This limitation does not apply when combining a :deco:`distjoin-base` with a
pure python class, and of course is not an issue if :class:`ReferenceFrame` is the only
base class for a derived type.

Other types that interact with :class:`ReferenceFrame` may also be subclasses, for example
:class:`Vector` or :class:`Matrix`. Because the base types of this module do not know how
to initialize any derived types, these base types are always returned by :class:`ReferenceFrame`
methods. For example if :class:`MyType` inherits from :class:`Vector`, calling
:meth:`ReferenceFrame.rotate_to` with a :class:`MyType` instance will always return a
:class:`Vector`. For different behavior you will need to create a subclass of :class:`ReferenceFrame`
and overload the functions so that they use the return value of the parent function to create
instances of :class:`MyType` as desired.

Instance Members
================

Properties
----------

.. py:property:: ReferenceFrame.intrinsic

    The intrinsic value provided to the constructor.

    :type: bool
    :readonly: this value cannot be changed

.. py:property:: ReferenceFrame.offset

    The offset of the origin of the reference frame.

    :type: this must be a :class:`Vector` or :data:`None``

.. py:property:: ReferenceFrame.order

    The Euler rotation order of the reference frame.

    :type: :class:`RotationOrder`
    :readonly: this value cannot be changed

Methods
-------

.. py:method:: ReferenceFrame.get_angles() -> EulerAngles

    Returns a copy of the current values of the internal rotation
    angles.

    .. note::

        The returned value is only a copy. Changing the values on the
        returned object will not affect the ReferenceFrame instance.
        See :meth:`ReferenceFrame.set_angles` for changing the
        ReferenceFrame angle values.

    :return: a copy of the internal angle state as an :class:`EulerAngles` object

.. py:method:: ReferenceFrame.set_angles(angles: EulerAngles) -> None
    ReferenceFrame.set_angles(*, alpha: Optional[SupportsFloat] = None, \
    beta: Optional[SupportsFloat] = None, gamma: Optional[SupportsFloat]) -> None

    Updates the angles of the rotation. The angles can be updated from an
    :class:`EulerAngles` instance, or individually from `alpha`, `beta`,
    or `gamma` values. The second form accepts keyword only arguments, and
    only non-None values will be updated. Each time an angle is updated, the
    internal rotation matrix is also updated. It is therefore most efficient
    to provide all angles at once, as the matrix is only updated a single time.
    
    The individual angle version of this method is more efficient as
    the angles don't need to be copied from an :class:`EulerAngles`
    instance, but the first form is provided for convenience.

    :param angles: an EulerAngles instance to update the rotation angles to
    :keyword alpha: the first angle of the rotation to update (default is :data:`None`)
    :keyword beta: the second angle of the rotation to update (default is :data:`None`)
    :keyword gamma: the third angle of the rotation to update (default is :data:`None`)
    :return: :data:`None`
    :raises TypeError: if `angles` is not an EulerAngles instance
    :raises TypeError: if any keyword values cannot be converted to a :class:`float`

.. py:method:: ReferenceFrame.get_matrix() -> Matrix:

    Returns a copy of the current state of the internal rotation matrix.
    This is mostly meant for introspective/validation purposes, however
    it can be used like any other matrix, but keep in mind it is merely
    a copy, and any modification will not be reflected on the internal
    state of the ReferenceFrame object it was retrieved from.

    :return: a copy of the internal rotation matrix

.. py:method:: ReferenceFrame.rotate_to(vector: Vector) -> Vector
    ReferenceFrame.rotate_to(frame: ReferenceFrame, vector: Vector) -> Vector

    Rotates a vector from the inertial reference frame that `self`
    is currently defined against, to this reference frame. If `frame` is
    provided, the method behaves like :func:`rotate_between`, rotating
    from `frame` **to** `self`.

    .. note::
        The suffix applies to the calling object i.e. `self`, so this
        method always rotates a vector **to** `self`.

.. py:method:: ReferenceFrame.rotate_from(vector: Vector) -> Vector
    ReferenceFrame.rotate_from(frame: ReferenceFrame, vector: Vector) -> Vector

    Rotates a vector from this reference frame to the inertial reference
    frame that `self` is currently defined against. If `frame` is provided,
    the method behaves like :func:`rotate_between`, rotating to `frame`
    **from** `self`.

    .. note::
        The suffix applies to the calling object i.e. `self`, so this
        method always rotates a vector **from** `self`.
