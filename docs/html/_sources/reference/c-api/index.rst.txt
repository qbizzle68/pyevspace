.. include:: /global.rst

=====
C API
=====

The C API provides C level access to the same types and methods as the Python
API to extend or embed PyEVSpace in your own Python extensions. PyEVSpace
provides a capsule that can be imported which provides binary level access
to the types and methods of the package. This makes the capsule more like
an application binary interface, similar to Pythons own ABI. You only need
to include the evspace_api.h header and import the capsule, and you will
have C level access to PyEVSpace.

.. note::
    
    All methods use the explicit types found in PyEVSpace, however they are
    all derived from :c:`PyObject` and can therefore be cast to :c:`PyObject*`
    if needed.

Types
-----

.. c:struct:: EVSpace_Vector

    C struct that defines the Python Vector type. A pointer of this type can
    be cast to or from a :c:type:`PyObject`\ \*.

    .. code-block:: c

        typedef struct {
            PyObject_HEAD
            double *data;
        } EVSpace_Vector;

.. c:member:: double* EVSpace_Vector.data

    The internal data array for the vector. The memory is allocated in the
    :c:type:`EVSpace_Vector` constructors to an array of length three. The
    values of the array of course are mutable, however caution should be used
    with resect to changing the actual pointer value. Setting the pointer to a
    new array is much more efficient than copying memory into it, however if the 
    object is currently in use by a buffer object such as a :py:class:`memoryview`, 
    you may be unable to free the original data array without rendering any views
    of the object useless, causing a memory leak. If you must change the pointer
    value, be sure you have considered all upstream and downstream consequences
    of doing so.

    .. note::

        If you chose to change the value of the data pointer, you are responsible
        for also freeing the memory. If you do not you must be sure that any other
        resource with a reference to the memory will free it when it is no longer
        in use, or else this will result in a memory leak.

.. c:struct:: EVSpace_Matrix

    C struct that defines the Python Matrix type.

    .. code-block:: c
        
        typedef struct {
            PyObject_HEAD
            double *data;
        } EVSpace_Matrix;

.. c:member:: double* EVSpace_Matrix.data

    The internal data array for the matrix. The memory is allocated in the
    :c:type:`EVSpace_Matrix` constructors to a one-demensional array of length
    nine. The memory starts at index [0, 0] and increases by column first, then
    by row. The mapping from two-dimensional indices to one-dimension is 
    :math:`3 * row + col`. The :c:macro:`EVSpace_RC_INDEX` macro can be used for 
    cleanly mapping between dimensions.

    .. code-block: c

        // not great, row and column indices not readable
        data_array[5] = 3;
        // okay, excessive code/memory usage
        int row = 1;
        int col = 2;
        int index = 3 * row + col;
        data_array[index] = 3;
        // best use, clean and consice
        data_array[EVSpace_RC_INDEX(1, 2)] = 3;

    The :c:macro:`EVSpace_Matrix_COMP` handles the mapping for you, when 
    dealing with an :c:type:`EVSpace_Matrix` instance.

    The values of the array of course are mutable, however caution should be used
    with resect to changing the actual pointer value. Setting the pointer to a
    new array is much more efficient than copying memory into it, however if the 
    object is currently in use by a buffer object such as a :py:class:`memoryview`, 
    you may be unable to free the original data array without rendering any views
    of the object useless, causing a memory leak. If you must change the pointer
    value, be sure you have considered all upstream and downstream consequences
    of doing so.

    .. note::

        If you chose to change the value of the data pointer, you are responsible
        for also freeing the memory. If you do not you must be sure that any other
        resource with a reference to the memory will free it when it is no longer
        in use, or else this will result in a memory leak.

.. c:struct:: EVSpace_Angles

    C struct that defines the Python Angles type.

    .. code-block:: c
        
        typedef struct {
            PyObject_HEAD
            double alpha;
            double beta;
            double gamma;
        } EVSpace_Angles;

.. c:member:: double EVSpace_Angles.alpha

    The first angle of an Euler rotation, in radians.

.. c:member:: double EVSpace_Angles.beta

    The second angle of an Euler rotation, in radians.

.. c:member:: double EVSpace_Angles.gamma

    The third angle of an Euler rotation, in radians.

.. c:enum:: EVSpace_Axis

    An enumerated axis type to distinguish between axes, mostly used in describing
    Euler rotation orders.

    .. code-block:: c

        typedef enum {
            X_AXIS = 0,
            Y_AXIS = 1,
            Z_AXIS = 2
        } EVSpace_Axis;

.. c:enumerator:: EVSpace_Axis.X_AXIS

    Used to indicate the x-axis or an axis aliasing the x-axis in another named
    reference frame. Has a value of 0.

.. c:enumerator:: EVSpace_Axis.Y_AXIS

    Used to indicate the y-axis or an axis aliasing the y-axis in another named
    reference frame. Has a value of 1.

.. c:enumerator:: EVSpace_Axis.Z_AXIS

    Used to indicate the z-axis or an axis aliasing the z-axis in another named
    reference frame. Has a value of 2.

.. c:struct:: EVSpace_Order

    A type used to represent an Euler rotation, consisting of three axes, each
    indicated by an :c:enum:`EVSpace_Axis`.

    .. code-block:: c

        typedef struct {
            PyObject_HEAD
            EVSpace_Axis first;
            EVSpace_Axis second;
            EVSpace_Axis third;
        } EVSpace_Order;

    There exist twelve unique Euler rotations, all of which are instantiated in
    the PyEVSpace capsule. As a result, you likely do not need to instantiate
    an EVSpace_Order object.

.. c:struct:: EVSpace_ReferenceFrame

    This C type is an idealized reference frame, which works by storing an
    internal matrix that describes a rotation from an intertial (unrotated)
    reference frame to this reference frame. The type also supports offset
    frames as well.

    .. code-block:: c

        typedef struct {
            PyObject_HEAD
            EVSpace_Order* order;
            EVSpace_Angles* angles;
            EVSpace_Matrix* matrix;
            EVSpace_Vector* offset;
        } EVSpace_ReferenceFrame;

Capsule
^^^^^^^

.. c:alias:: EVSpace_CAPI

The full capsule documentation can be found :doc:`here <capsule>`. A list of
its contents follow:

Types
"""""

.. c:alias:: EVSpace_CAPI.VectorType
             EVSpace_CAPI.MatrixType
             EVSpace_CAPI.AnglesType
             EVSpace_CAPI.OrderType
             EVSpace_CAPI.RefFrameType

Constructors
""""""""""""

.. c:alias:: EVSpace_CAPI.Vector_FromArray
             EVSpace_CAPI.Vector_StealArray
             EVSpace_CAPI.Matrix_FromArray
             EVSpace_CAPI.Matrix_StealArray
             EVSpace_CAPI.Angles_New
             EVSpace_CAPI.Order_New
             EVSpace_CAPI.RefFrame_New

Vector numeric methods
""""""""""""""""""""""

.. c:alias:: EVSpace_CAPI.Vector_add
             EVSpace_CAPI.Vector_subtract
             EVSpace_CAPI.Vector_multiply
             EVSpace_CAPI.Vector_multiply_m
             EVSpace_CAPI.Vector_divide
             EVSpace_CAPI.Vector_iadd
             EVSpace_CAPI.Vector_isubtract
             EVSpace_CAPI.Vector_imultiply
             EVSpace_CAPI.Vector_idivide
             EVSpace_CAPI.Vector_negative

Matrix numeric methods
""""""""""""""""""""""

.. c:alias:: EVSpace_CAPI.Matrix_add
             EVSpace_CAPI.Matrix_subtract
             EVSpace_CAPI.Matrix_multiply_v
             EVSpace_CAPI.Matrix_multiply_m
             EVSpace_CAPI.Matrix_multiply_s
             EVSpace_CAPI.Matrix_divide
             EVSpace_CAPI.Matrix_iadd
             EVSpace_CAPI.Matrix_isubtract
             EVSpace_CAPI.Matrix_imultiply
             EVSpace_CAPI.Matrix_negative

Vector instance methods
"""""""""""""""""""""""

.. c:alias:: EVSpace_CAPI.Vector_mag
             EVSpace_CAPI.Vector_mag2
             EVSpace_CAPI.Vector_normalize

Vector module methods
"""""""""""""""""""""

.. c:alias:: EVSpace_CAPI.Vector_dot
             EVSpace_CAPI.Vector_cross
             EVSpace_CAPI.Vector_norm
             EVSpace_CAPI.Vector_vang
             EVSpace_CAPI.Vector_vxcl
             EVSpace_CAPI.Vector_proj

Matrix module methods
"""""""""""""""""""""

.. c:alias:: EVSpace_CAPI.Matrix_det
             EVSpace_CAPI.Matrix_transpose

Rotation order instances

.. c:alias:: Order_XYZ
             Order_XZY
             Order_YXZ
             Order_YZX
             Order_ZXY
             Order_ZYX
             Order_XYX
             Order_XZX
             Order_YXY
             Order_YZY
             Order_ZXZ
             Order_ZYZ

Rotation matrix methods
"""""""""""""""""""""""

.. c:alias:: EVSpace_CAPI.Get_matrix
             EVSpace_CAPI.Get_euler
             EVSpace_CAPI.Get_from_to

Vector rotate methods
"""""""""""""""""""""

.. c:alias:: EVSpace_CAPI.Rotate_axis_to
             EVSpace_CAPI.Rotate_axis_from
             EVSpace_CAPI.Rotate_euler_to
             EVSpace_CAPI.Rotate_euler_from
             EVSpace_CAPI.Rotate_matrix_to
             EVSpace_CAPI.Rotate_matrix_from
             EVSpace_CAPI.Rotate_offset_to
             EVSpace_CAPI.Rotate_offset_from

Reference frame functions
"""""""""""""""""""""""""

.. c:alias:: EVSpace_CAPI.Rotate_ref_to
             EVSpace_CAPI.Rotate_ref_from
             EVSpace_CAPI.Rotate_ref_to_ref
             EVSpace_CAPI.Rotate_ref_from_ref

Macros
^^^^^^

.. c:macro:: EVSpace_VECTOR_X(obj)

    Simplifies accessing the X component of a vector by casting `obj` to an
    EVSpace_Vector* and calling the underlying array.

    :param obj: a PyObject* or EVSpace_Vector* to get the X component of

.. c:macro:: EVSpace_VECTOR_Y(obj)

    Simplifies accessing the Y component of a vector by casting `obj` to an
    EVSpace_Vector* and calling the underlying array.

    :param obj: a PyObject* or EVSpace_Vector* to get the Y component of

.. c:macro:: EVSpace_VECTOR_Z(obj)

    Simplifies accessing the Z component of a vector by casting `obj` to an
    EVSpace_Vector* and calling the underlying array.

    :param obj: a PyObject* or EVSpace_Vector* to get the Z component of

.. c:macro:: EVSpace_RC_INDEX(row, col)

    Maps a row and column index to a single 1-dimensional contiguous array
    index. This is simply :c:`3 * row + col`, but looks much cleaner in code.

    .. code-block: c
        
        int row = 1;
        int col = 2;
        arr[EVSpace_RC_INDEX(row, col)] = 5;

    :param row: row index
    :param col: col index

.. c:macro:: EVSpace_MATRIX_COMP(obj, row, col)

    Simplifies accessing a component of `obj` by casting to a 
    EVSpace_Matrix* and calling the underlying array.

    :param obj: a PyObject* or EVSpace_Vector* to get the component of
    :param row: the row of the component to get
    :param col: the column of the component to get

.. c:macro:: Vector_Check(obj)
    
    Checks if `obj` is an `EVSpace_VectorType`.

    :param obj: a :c:`PyObject*` to check the type of

.. c:macro:: Matrix_Check(obj)

    Checks if `obj` is an `EVSpace_MatrixType`.

    :param obj: a :c:`PyObject*` to check the type of
    
.. c:macro:: Angles_Check(obj)

    Checks if `obj` is an `EVSpace_AnglesType`.

    :param obj: a :c:`PyObject*` to check the type of

.. c:macro:: Order_Check(obj)

    Checks if `obj` is an `EVSpace_OrderType`.

    :param obj: a :c:`PyObject*` to check the type of

.. c:macro:: ReferenceFrame_Check(obj)

    Checks if `obj` is an `EVSpace_ReferenceFrameType`.

    :param obj: a :c:`PyObject*` to check the type of

The following macros are all for calls to capsule functions. They are of the
form Evs_XXX, where XXX is the name of the capsule function. The Order
instances do not follow this rule, for example the macro for `Order_XYZ` is
just `Evs_XYZ`.

Constructors
""""""""""""

The constructor macros handle the `type` argument in the constructor call.

.. c:macro:: Evs_Vector_FromArray(array)

    Maps to :c:func:`EVSpace_CAPI.Vector_FromArray`.

.. c:macro:: Evs_Vector_StealArray(array)

    Maps to :c:func:`EVSpace_CAPI.Vector_StealArray`.

.. c:macro:: Evs_Matrix_FromArray(array)

    Maps to :c:func:`EVSpace_CAPI.Matrix_FromArray`.

.. c:macro:: Evs_Matrix_StealArray(array)

    Maps to :c:func:`EVSpace_CAPI.Matrix_StealArray`.

.. c:macro:: Evs_Angles_New(alpha, beta, gamma)

    Maps to :c:func:`EVSpace_CAPI.Angles_New`.

.. c:macro:: Evs_Order_New(order, angles)

    Maps to :c:func:`EVSpace_CAPI.Order_New`.

.. c:macro:: Evs_RefFrame_New(order, angles, offset)

    Maps to :c:func:`EVSpace_CAPI.RefFrame_New`.

Vector number methods
"""""""""""""""""""""

.. c:macro:: Evs_Vector_add(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Vector_add`.

.. c:macro:: Evs_Vector_subtract(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Vector_subtract`.

.. c:macro:: Evs_Vector_multiply(self, scalar)

    Maps to :c:func:`EVSpace_CAPI.Vector_multiply`.

.. c:macro:: Evs_Vector_multiply_m(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Vector_multiply_m`.

.. c:macro:: Evs_Vector_divide(self, scalar)

    Maps to :c:func:`EVSpace_CAPI.Vector_divide`.

.. c:macro:: Evs_Vector_iadd(self, other)

    Maps to :c:func:`EVSpace_CAPI.Vector_iadd`.

.. c:macro:: Evs_Vector_isubtract(self, other)

    Maps to :c:func:`EVSpace_CAPI.Vector_isubtract`.

.. c:macro:: Evs_Vector_imultiply(self, scalar)

    Maps to :c:func:`EVSpace_CAPI.Vector_imultiply`.

.. c:macro:: Evs_Vector_idivide(self, scalar)

    Maps to :c:func:`EVSpace_CAPI.Vector_idivide`.

.. c:macro:: Evs_Vector_negative(self)

    Maps to :c:func:`EVSpace_CAPI.Vector_negative`.

Matrix number methods
"""""""""""""""""""""

.. c:macro:: Evs_Matrix_add(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Matrix_add`.

.. c:macro:: Evs_Matrix_subtract(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Matrix_subtract`.

.. c:macro:: Evs_Matrix_multiply_v(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Matrix_multiply_v`.

.. c:macro:: Evs_Matrix_multiply_m(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Matrix_multiply_m`.

.. c:macro:: Evs_Matrix_multiply_s(self, scalar)

    Maps to :c:func:`EVSpace_CAPI.Matrix_multiply_s`.

.. c:macro:: Evs_Matrix_divide(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Matrix_divide`.

.. c:macro:: Evs_Matrix_iadd(self, other)

    Maps to :c:func:`EVSpace_CAPI.Matrix_iadd`.

.. c:macro:: Evs_Matrix_isubtract(self, other)

    Maps to :c:func:`EVSpace_CAPI.Matrix_isubtract`.

.. c:macro:: Evs_Matrix_imultiply(self, scalar)

    Maps to :c:func:`EVSpace_CAPI.Matrix_imultiply`.

.. c:macro:: Evs_Matrix_idivide(self, scalar)

    Maps to :c:func:`EVSpace_CAPI.Matrix_idivide`.

.. c:macro:: Evs_Matrix_negative(self)

    Maps to :c:func:`EVSpace_CAPI.Matrix_negative`.

Vector instance methods
"""""""""""""""""""""""

.. c:macro:: Evs_mag(self)

    Maps to :c:func:`EVSpace_CAPI.Vector_mag`.

.. c:macro:: Evs_mag2(self)

    Maps to :c:func:`EVSpace_CAPI.Vector_mag2`.

.. c:macro:: Evs_normalize(self)

    Maps to :c:func:`EVSpace_CAPI.Vector_normalize`.

Vector functions
""""""""""""""""

.. c:macro:: Evs_dot(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Vector_dot`.

.. c:macro:: Evs_cross(lhs, rhs)

    Maps to :c:func:`EVSpace_CAPI.Vector_cross`.

.. c:macro:: Evs_norm(self)

    Maps to :c:func:`EVSpace_CAPI.Vector_norm`.

.. c:macro:: Evs_vang(from, to)

    Maps to :c:func:`EVSpace_CAPI.Vector_vang`.

.. c:macro:: Evs_vxcl(vec, xcl)

    Maps to :c:func:`EVSpace_CAPI.Vector_vxcl`.

.. c:macro:: Evs_proj(proj, onto)

    Maps to :c:func:`EVSpace_CAPI.Vector_proj`.

Matrix functions
""""""""""""""""

.. c:macro:: Evs_det(self)

    Maps to :c:func:`EVSpace_CAPI.Matrix_det`.

.. c:macro:: Evs_transpose(self)

    Maps to :c:func:`EVSpace_CAPI.Matrix_transpose`.

Rotation order instances
""""""""""""""""""""""""

.. c:macro:: Evs_XYZ

    Maps to :c:var:`EVSpace_CAPI.Order_XYZ`.

.. c:macro:: Evs_XZY

    Maps to :c:var:`EVSpace_CAPI.Order_XZY`.

.. c:macro:: Evs_YXZ

    Maps to :c:var:`EVSpace_CAPI.Order_YXZ`.

.. c:macro:: Evs_YZX

    Maps to :c:var:`EVSpace_CAPI.Order_YZX`.

.. c:macro:: Evs_ZXY

    Maps to :c:var:`EVSpace_CAPI.Order_ZXY`.

.. c:macro:: Evs_ZYX

    Maps to :c:var:`EVSpace_CAPI.Order_ZYX`.

.. c:macro:: Evs_XYX

    Maps to :c:var:`EVSpace_CAPI.Order_XYX`.

.. c:macro:: Evs_XZX

    Maps to :c:var:`EVSpace_CAPI.Order_XZX`.

.. c:macro:: Evs_YXY

    Maps to :c:var:`EVSpace_CAPI.Order_YXY`.

.. c:macro:: Evs_YZY

    Maps to :c:var:`EVSpace_CAPI.Order_YZY`.

.. c:macro:: Evs_ZXZ

    Maps to :c:var:`EVSpace_CAPI.Order_ZXZ`.

.. c:macro:: Evs_ZYZ

    Maps to :c:var:`EVSpace_CAPI.Order_ZYZ`.

Rotation matrix functions
"""""""""""""""""""""""""

.. c:macro:: Evs_get_matrix(axis, angle)

    Maps to :c:func:`EVSpace_CAPI.Get_matrix`.

.. c:macro:: Evs_get_euler(order, angles)

    Maps to :c:func:`EVSpace_CAPI.Get_euler`.

.. c:macro:: Evs_from_to(orderFrom, anglesFrom, orderTo, anglesTo)

    Maps to :c:func:`EVSpace_CAPI.Get_from_to`.

Rotate vector functions
"""""""""""""""""""""""

.. c:macro:: Evs_axis_to(axis, angle, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_axis_to`.

.. c:macro:: Evs_axis_from(axis, angle, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_axis_from`.

.. c:macro:: Evs_euler_to(order, angles, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_euler_to`.

.. c:macro:: Evs_euler_from(order, angles, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_euler_from`.

.. c:macro:: Evs_matrix_to(matrix, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_matrix_to`.

.. c:macro:: Evs_matrix_from(matrix, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_matrix_from`.

.. c:macro:: Evs_offset_to(matrix, offset, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_offset_to`.

.. c:macro:: Evs_offset_from(matrix, offset, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_offset_from`.

Reference frame functions
"""""""""""""""""""""""""

.. c:macro:: Evs_ref_to(frame, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_ref_to`.

.. c:macro:: Evs_ref_from(frame, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_ref_from`.

.. c:macro:: Evs_ref_to_ref(self, frameTo, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_ref_to_ref`.

.. c:macro:: Evs_ref_from_ref(self, frameFrom, vector)

    Maps to :c:func:`EVSpace_CAPI.Rotate_ref_from_ref`.