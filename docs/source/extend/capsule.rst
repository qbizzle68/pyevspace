=============
C API Capsule
=============

Importing the Capsule
=====================

The PyEVSpace :c:type:`PyCapsule` provides C API support for creating and managing
the Python layer objects when including PyEVSpace in an extension module.
Your project should include the :file:`include/pyevspace-api.hpp` header
file into the module importing PyEVSpace. This file is usable for extensions
written in either C or C++. The global static variable :c:var:`PyEVSpace_API`
can be properly filled by calling :c:func:`PyEVSpace_ImportCapsule`, and is
then available globally to all functions needing the capsule.

Due to :doc:`C++ compatibilitiy issues <compatibility>`, the wrapped C++
objects cannot be exposed directly, and so the capsule provides a stable
C ABI interface for managing the Python level objects defined by the module.
This is done using C primitives, and all functions allow creation, access,
or modification of Python objects using primitives, usually in arrays. There
are also definitions of the :c:type:`PyTypeObject` structs, which supports
the ability to handle functionality of the Python objects through the CPython
layer to access methods not supported by the capsule.

All of the capsule functions are tested by building small extension modules
that use these functions. They can be referenced as `examples <https://github.com/qbizzle68/pyevspace/blob/master/tests/_test_capsule/capsule_consumer_module.c>`_
for how to use the capsule functions, but keep in mind they are farily trivial,
only meant to test behavior.

.. versionchanged:: 0.16.0
    Because the module is now multi-phase initialized, there is no global
    state, and therefore there is no global :c:expr:`PyTypeObject*` variables for
    use in type checking and creating new instances. While the capsule methods
    that create and return new :c:expr:`PyObject*` can lookup the module
    state, any caller already has direct access to the types via the capsule.
    Therefore, these functions now require the caller to pass the :c:expr:`PyTypeObject*`
    of the desired return type (or a subclass of the type) to the capsule method.

Capsule Utilities
-----------------

.. c:var:: PyEVSPace_CAPI* PyEVSpace_API

    A global static pointer to the PyEVSpace capsule struct :c:struct:`PyEVSpace_CAPI`
    This value can be filled using the :c:func:`PyEVSpace_ImportCapsule` function.

    .. versionremoved:: 0.16.0
        As the module uses multi-phase initialization there can no longer be
        any modifiable global state. Therefore this variable is removed and
        any user of the capsule needs to define a pointer to a capsule instance
        somewhere within their module layout.

.. c:function:: int PyEVSpace_ImportCapsule(PyEVSpace_CAPI** out)

    Imports the PyEVSpace module if not already imported, and fills the `out`
    parameter to the capsule pointer attached to the PyEVSpace module. The
    source of the `out` parameter is up to the caller to define, if creating
    a single-phase intialized module a global static variable is most common,
    or within your own module's state for multi-phased initialization is best.
    The :c:macro:`PYEVSPACE_CAPSULE_VERSION` macro should be used to verify the
    :c:member:`PyEVSpace_CAPI.version` value of the capsule matches the version
    that is included in your extension.
    
    .. versionchanged:: 0.16.0
        This function now takes a required :c:expr:`PyEVSpace_CAPI**` argument
        to set the capsule to, as there is no longer a global instance
        of this type (see :c:var:`EVSpace_API`).

    :retval 0: on successful import of the capsule
    :retval -1: if an error occurred

.. c:macro:: PYEVSPACE_CAPSULE_NAME

    The name of the capsule used during import. If manually importing the capsule
    (and not via :c:func:`PyEVSpace_ImportCapsule`), this is the string value
    that should be passed to :c:func:`PyCapsule_Import`.

.. c:macro:: PYEVSPACE_CAPSULE_VERSION

    The version of the PyEVSpace capsule defined in the header. This should be
    verified against :c:var:`PyEVSpace_CAPI.version` to ensure the version
    of the imported capsule and the header definition are compatible.

    .. versionchanged:: 0.16.0
        This value was bumped up as there are backwards incompatible changes
        within the capsule.

Capsule Definition
------------------

.. c:struct:: PyEVSpace_CAPI

    The C struct containing the definition of the PyEVSpace :c:type:`PyCapsule`.
    An instance of this type is cast to a :c:expr:`void*` to create the capsule.
    The exact definition is shown here, and further documentation of each
    member is given below.

    .. code-block:: c

        typedef struct
        {
            int version;

            // PyTypeObjects
            PyTypeObject* Vector_Type;
            PyTypeObject* Matrix_Type;
            PyTypeObject* EulerAngles_Type;
            PyTypeObject* RotationOrder_Type;
            PyTypeObject* ReferenceFrame_Type;

            // evspace state getters
            int (*PyEVSpaceVector_GetState)(PyObject*, double[3]);
            int (*PyEVSpaceMatrix_GetState)(PyObject*, double[9]);
            int (*PyEVSpaceAngles_GetState)(PyObject*, double[3]);
            int (*PyEVSpaceOrder_GetState)(PyObject*, unsigned int[3]);
            int (*PyEVSpaceFrame_GetState)(PyObject*, unsigned int[3], double[3], double[3], int*);
            PyObject* (*PyEVSpaceFrame_GetOrder)(PyObject*);
            PyObject* (*PyEVSpaceFrame_GetAngles)(PyObject*);
            PyObject* (*PyEVSpaceFrame_GetOffset)(PyObject*);
            int (*PyEVSpaceFrame_GetIntrinsic)(PyObject*);

            // Python type constructors via state
            PyObject* (*PyEVSpaceVector_FromState)(PyTypeObject*, double[3]);
            PyObject* (*PyEVSpaceMatrix_FromState)(PyTypeObject*, double[9]);
            PyObject* (*PyEVSpaceAngles_FromState)(PyTypeObject*, double[3]);
            PyObject* (*PyEVSpaceOrder_FromState)(PyTypeObject*, unsigned int[3]);
            PyObject* (*PyEVSpaceFrame_FromState)(PyTypeObject*, unsigned int[3], double[3], double*, int);

            // Python type modifiers
            int (*PyEVSpaceVector_SetState)(PyObject*, double[3]);
            int (*PyEVSpaceMatrix_SetState)(PyObject*, double[9]);
            int (*PyEVSpaceAngles_SetState)(PyObject*, double[3]);
            int (*PyEVSpaceFrame_SetAngles)(PyObject*, double[3]);
            int (*PyEVSpaceFrame_SetOffset)(PyObject*, double*);

        } PyEVSpace_CAPI;
    
    .. versionchanged:: 0.16.0
        The PyEVSpace*_FromState methods now require a :c:type:`PyTypeObject`
        pointer to the type they are meant to create.

Capsule Documentation
=====================

Capsule Version
---------------

The version of the compiled capsule imported must have the same version
as the :c:struct:`PyEVSpace_CAPI` definition included in your extension
module. This is done by comparing :c:var:`PyEVSpace_CAPI.version` with
:c:macro:`PYEVSPACE_CAPSULE_VERSION` after importing the capsule. If they
do not match you should set an exception and return an error status.

.. c:var:: int PyEVSpace_CAPI.version

    The API version of the imported capsule.

PyTypeObjects
-------------

.. versionchanged:: 0.16.0
    These are now heap types, created from :c:type:`PyType_Spec` definitions
    for each module created.

.. c:var:: PyTypeObject* PyEVSpace_CAPI.Vector_Type
    
    The PyTypeObject for the :py:class:`pyevspace.Vector` type.

.. c:var:: PyTypeObject* PyEVSpace_CAPI.Matrix_Type

    The PyTypeObject for the :py:class:`pyevspace.Matrix` type.

.. c:var:: PyTypeObject* PyEVSpace_CAPI.EulerAngles_Type

    The PyTypeObject for the :py:class:`pyevspace.EulerAngles` type.

.. c:var:: PyTypeObject* PyEVSpace_CAPI.RotationOrder_Type

    The PyTypeObject for the :py:class:`pyevspace.RotationOrder` type.

.. c:var:: PyTypeObject* PyEVSpace_CAPI.ReferenceFrame_Type

    The PyTypeObject for the :py:class:`pyevspace.ReferenceFrame` type.

State Getters
-------------

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceVector_GetState(PyObject* obj, double state[3])

    Get the internal state of a :py:class:`pyevspace.Vector` as a PyObject
    into a :c:expr:`double` array. The state is a simple array containing the
    x, y, and z values of the vector. If successful return `0`, otherwise
    return `-1`.

    :param obj: the :py:class:`pyevspace.Vector` instance to get the state of
    :param state: the output array that will contain the vector state. This
        is only filled if the function succeeds
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceMatrix_GetState(PyObject* obj, double state[9])

    Get the internal state of a :py:class:`pyevspace.Matrix` as a PyObject
    into a :c:expr:`double` array. The state is a 1-dimensional array of
    concatenated rows of the matrix components. This means the indices
    0-2 contain the first row, 3-5 contain the second row, and 6-8 contain
    the third row. For using a 1-dimensional array in nested for loops,
    the index mapping is :c:expr:`state[row * 3 + column] = value`.

    Returns `0` on success or `-1` on failure.

    :param obj: the :py:class:`pyevspace.Matrix` instance to get the state of
    :param state: the output array containing the matrix state as a 1-dimension
        contiguous array. This is only filled if the function succeeds
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set 

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceAngles_GetState(PyObject* obj, double state[3])

    Get the internal state of a :py:class:`pyevspace.EulerAngles` as a PyObject
    into a :c:expr:`double` array. The state contains the three rotation angles
    of the container. Returns `0` on success or `-1` on failure.

    :param obj: the :py:class:`pyevspace.EulerAngles` instance to get the state of
    :param state: the output array that will contain the angles state. This
        is only filled if the function succeeds
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceOrder_GetState(PyObject* obj, unsigned int state[3])

    Get the internal state of a :py:class:`pyevspace.RotationOrder` as a PyObject
    into a :c:expr:`unsigned int` array. The state contains the three rotation
    axes of the object as enumerated axis values. The values are:

    ==== =====
    Axis Value
    ==== =====
    X    0
    Y    1
    Z    2
    ==== =====
    
    Returns `0` on success or `-1` on failure.

    :param obj: the :py:class:`pyevspace.RotationOrder` instance to get the state of
    :param state: the output array that will contain the order state. This is
        only filled if the function succeeds
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceFrame_GetState(PyObject* obj, unsigned int order[3], double angles[3], double offset[3], int* intrinsic)

    Get the internal state of a :py:class:`pyevspace.ReferenceFrame` as a PyObject
    into respective state variables. The `order` and `angles` parameters behave
    identically to their respective type state getters above. The `offset` parameter
    is filled as the internal state of the offset vector of the reference frame,
    and the `intrinsic` parameter is set to `1` for intrinsic and `0` for extrinsic
    rotations. This function returns `0` on success or `-1` on failure.

    .. note::

        The internal value for :py:attr:`pyevspace.ReferenceFrame.offset` may
        be :py:data:`None` when no offset is given during construction. Since
        the state must be returned as a :c:expr:`double` array, there is no
        clean way of signaling when the value is :py:data:`None`. Since this
        value is mathematically analogous to the zero vector, when the offset
        is :py:data:`None`, all `offset` components will be set to zero.

    :param obj: the :py:class:`pyevspace.ReferenceFrame` instance to get the state of
    :param order: the output array that will contain the order state
    :param angles: the output array that will contain the angles state
    :param offset: the output array that will contain the offset state
    :param intrinsic: the output variable that will contain the intrinsic
        value. Note the type of this parameter is a pointer, so the address
        of the local intrinsic variable should be passed
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. c:function:: PyObject* PyEVSpace_CAPI.PyEVSpaceFrame_GetOrder(PyObject* obj)

    Get a PyObject version of the :py:class:`pyevspace.RotationOrder` from
    a :py:class:`pyevspace.ReferenceFrame` type as a PyObject. The returned
    object is a representation of the order only, and any modification has
    no affect on the original reference frame. The returned object is a
    *new reference* owned by the caller.

    :param obj: the reference frame to get the rotation order of
    :return: a new reference to a rotation order
    :retval NULL: if an error occurred with an appropriate exception set

.. c:function:: PyObject* PyEVSpace_CAPI.PyEVSpaceFrame_GetAngles(PyObject* obj)

    Get a PyObject version of the :py:class:`pyevspace.EulerAngles` from
    a :py:class:`pyevspace.ReferenceFrame` type as a PyObject. The returned
    object is a representation of the angles only, and any modification has
    no affect on the original reference frame. The returned object is a
    *new reference* owned by the caller.

    :param obj: the reference frame to get the rotation angles of
    :return: a new reference to a set of rotation angles
    :retval NULL: if an error occurred with an appropriate exception set

.. c:function:: PyObject* PyEVSpace_CAPI.PyEVSpaceFrame_GetOffset(PyObject* obj)

    Get a PyObject version of the offset value from a :py:class:`pyevspace.ReferenceFrame`
    type as a PyObject. This may be a :py:class:`pyevspace.Vector` instance,
    or :c:var:`Py_None`. The returned object is a representation of the offset only, and any
    modification has no affect on the original reference frame. The returned
    object is a *new reference* owned by the caller.

    :param obj: the reference frame to get the offset value of
    :return: a new reference to the offset vector, or Py_None
    :retval NULL: if an error occurred with an appropriate exception set

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceFrame_GetIntrinsic(PyObject* obj)

    Get the value of the intrinsic variable from a
    :py:class:`pyevspace.ReferenceFrame` type as a PyObject. This will be `1`
    if intrinsic, `0` if extrinsic, or `-1` if an exception occurred.

    :param obj: the reference frame to get the intrinsic value of
    :return: an equivalent boolean value answering if the rotation is intrinsic
    :retval 1: if the reference frame rotation is intrinsic
    :retval 0: if the reference frame rotation is extrinsic
    :retval -1: if an error occurred with an appropriate exception set

PyObject Constructors
---------------------

.. c:function:: PyObject* PyEVSpace_CAPI.PyEVSpaceVector_FromState(PyTypeObject* type, double state[3])

    Creates a :c:expr:`PyObject*` for a :py:class:`pyevspace.Vector` or
    a subclass of that type, depending on `type`, from the vector state
    as a C :c:expr:`double`. If wanting to create a :py:class:`pyevspace.Vector`
    instance, `type` should be :c:member:`PyEVSpace_CAPI.Vector_Type`. The
    returned object is a *new reference* owned by the caller.

    :param type: the type of instance to create. This must be exactly or
        a subclass of :c:member:`PyEVSpace_CAPI.Vector_Type`
    :param state: the state of the vector to return
    :return: a new vector as a PyObject
    :retval NULL: if an error occurred with an appropriate exception set

    .. versionchanged:: 0.16.0
        This function now takes a :c:expr:`PyTypeObject*` referencing the type
        to create.

.. c:function:: PyObject* PyEVSpace_CAPI.PyEVSpaceMatrix_FromState(PyTypeObject* type, double state[9])

    Creates a :c:expr:`PyObject*` for a :py:class:`pyevspace.Matrix` or
    a subclass of that type, depending on `type`, from the matrix state
    as a C :c:expr:`double`. If wanting to create a :py:class:`pyevspace.Matrix`
    instance, `type` should be :c:member:`PyEVSpace_CAPI.Matrix_Type`. The
    returned object is a *new reference* owned by the caller.

    :param type: the type of instance to create. This must be exactly or
        a subclass of :c:member:`PyEVSpace_CAPI.Matrix_Type`
    :param state: the state of the matrix to return
    :return: a new matrix as a PyObject
    :retval NULL: if an error occurred with an appropriate exception set

    .. versionchanged:: 0.16.0
        This function now takes a :c:expr:`PyTypeObject*` referencing the type
        to create.

.. c:function:: PyObject* PyEVSpace_CAPI.PyEVSpaceAngles_FromState(PyTypeObject* type, double state[3])

    Creates a :c:expr:`PyObject*` for a :py:class:`pyevspace.EulerAngles` or
    a subclass of that type, depending on `type`, from the angles state
    as a C :c:expr:`double`. If wanting to create a :py:class:`pyevspace.EulerAngles`
    instance, `type` should be :c:member:`PyEVSpace_CAPI.EulerAngles_Type`.
    The returned object is a *new reference* owned by the caller.

    :param type: the type of instance to create. This must be exactly or
        a subclass of :c:member:`PyEVSpace_CAPI.EulerAngles_Type`
    :param state: the state of the Euler angles to return
    :return: a new Euler angles object as a PyObject
    :retval NULL: if an error occurred with an appropriate exception set

    .. versionchanged:: 0.16.0
        This function now takes a :c:expr:`PyTypeObject*` referencing the type
        to create.

.. c:function:: PyObject* PyEVSpace_CAPI.PyEVSpaceOrder_FromState(PyTypeObject* type, unsigned int state[3])

    Creates a :c:expr:`PyObject*` for a :py:class:`pyevspace.RotationOrder` or
    a subclass of that type, depending on `type`, from the order state as a
    :c:expr:`double`. If wanting to create a :py:class:`pyevspace.RotationOrder`
    instance, `type` should be :c:member:`PyEVSpace_CAPI.RotationOrder_Type`.
    The returned object is a *new reference* owned by the caller.

    :param type: the type of instance to create. This must be exactly or
        a subclass of :c:member:`PyEVSpace_CAPI.RotationOrder_Type`
    :param state: the state of the rotation order to return
    :return: a new rotation order as a PyObject
    :retval NULL: if an error occurred with an appropriate exception set

    .. versionchanged:: 0.16.0
        This function now takes a :c:expr:`PyTypeObject*` referencing the type
        to create.

.. c:function:: PyObject* PyEVSpace_CAPI.PyEVSpaceFrame_FromState(PyTypeObject* type, unsigned int order[3], double angles[3], double* offset, int intrinsic)

    Creates a :c:expr:`PyObject*` for a :py:class:`pyevspace.ReferenceFrame` type or
    a subclass of that type, depending on `type`, using the parameters similar
    to those as :c:func:`PyEVSpaceFrame_GetState`, with the exception of the
    `offset` type. As this is an input parameter and not an output parameter,
    `offset` may be :c:expr:`NULL`. Otherwise, `offset` is treated to have type
    :c:expr:`double[3]`, and anything else may cause undefined behavior, or
    worse, crash the program.

    If wanting to create a :py:class:`pyevspace.ReferenceFrame` instance,
    `type` should be :c:member:`PyEVSpace_CAPI.ReferenceFrame_Type`. The
    returned object is a *new reference* owned by the caller.

    :param type: the type of instance to create. This must be exactly or
        a subclass of :c:member:`PyEVSpace_CAPI.ReferenceFrame_Type`
    :param order: the rotation order state of the reference frame
    :param angles: the rotation angles state of the reference frame
    :param offset: :c:expr:`NULL` to be equivalent to :c:var:`Py_None`, otherwise
        the vector offset state of the reference frame
    :param intrinsic: `1` for an intrinsic rotation, `0` for extrinsic
    :return: a new reference frame as a PyObject
    :retval NULL: if an error occurred with an appropriate exception set

    .. versionchanged:: 0.16.0
        This function now takes a :c:expr:`PyTypeObject*` referencing the type
        to create.

PyObject Modifiers
------------------

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceVector_SetState(PyObject* obj, double state[3])

    Sets the state of a :py:class:`pyevspace.Vector` as a PyObject from a 
    C :c:expr:`double` array. Returns `0` on success and `-1` on failure.

    :param obj: the vector object to modify
    :param state: the state to set the vector to
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceMatrix_SetState(PyObject* obj, double state[9])

    Sets the state of a :py:class:`pyevspace.Matrix` as a PyObject from a
    1-dimensional contiguous C :c:expr:`double` array. The layout of this
    array is explained in :c:func:`PyEVSpaceMatrix_GetState`. Returns `0`
    on success and `-1` on failure.

    :param obj: the matrix object to modify
    :param state: the state to set the matrix to
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceAngles_SetState(PyObject* obj, double state[3])

    Sets the state of a :py:class:`pyevspace.EulerAngles` as a PyObject from
    a C :c:expr:`double` array. Returns `0` on success and `-1` on failure.

    :param obj: the Euler angles object to modify
    :param state: the state to set the Euler angles to
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceFrame_SetAngles(PyObject* obj, double state[3])

    Sets the rotation angles state of a :py:class:`pyevspace.ReferenceFrame`
    as a PyObject from a C :c:expr:`double` array. Returns `0` on success and
    `-1` on failure.

    :param obj: the reference frame object to modify
    :param state: the state to set the reference frame's rotation angles to
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set

.. c:function:: int PyEVSpace_CAPI.PyEVSpaceFrame_SetOffset(PyObject* obj, double* offset)

    Sets the offset state of a :py:class:`pyevspace.ReferenceFrame` as a
    PyObject from a C :c:expr:`double` array. `offset` may be :c:expr:`NULL`
    to set the reference frame's offset value to :c:var:`Py_None`, otherwise
    `offset` is treated to have type :c:expr:`double[3]`, and anything else
    may cause undefined behavior, or worse, crash the program.

    Returns `0` on success and `-1` on failure.

    :param obj: the reference frame object to modify
    :param state: the state to set the offset value to (may be :c:expr:`NULL`
        for :c:var:`Py_None`)
    :retval 0: on success
    :retval -1: on failure with an appropriate exception set
