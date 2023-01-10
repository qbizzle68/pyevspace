.. include:: /global.rst

===================
PyEVSpace C Capsule
===================

Importing the Capsule
---------------------

The capsule must be imported before it can be used. The public C API defines
a global capsule variable and a macro to handle the importing.

.. c:var:: static EVSpace_CAPI* EVSpaceAPI
    
    A global variable to hold the C API capsule.

.. c:macro:: EVSpace_IMPORT

    Imports the capsule into the global :c:var:`EVSpaceAPI` pointer. Place
    this macro after including the evspace_api.h header and you will have
    access to the capsule.

    .. code-block:: C

        #include <evspace_api.h>
        EVSpace_IMPORT;

The following macro also defines the name of the capsule if it is needed:

.. c:macro:: EVSpace_CAPSULE_NAME

    Expands to the name of the capsule needed for some of CPython's
    capsule methods.

C API Capsule
-------------

.. c:struct:: EVSpace_CAPI

    Python capsule object that contain all functions of the PyEVSpace C API.
    For more details on capsules check out the official Python
    `docs <https://docs.python.org/3/extending/extending.html#providing-a-c-api-for-an-extension-module>`_.
    The capsule can be imported using the :c:macro:`EVSpace_IMPORT` macro,
    which sets the global pointer :c:var:`EVSpaceAPI` using the
    :c:`PyCapsule_Import()` method. There are helper macros for every function
    inside the capsule to avoid cumbersome method calls. For example to create
    a new :c:type:`EVSpace_Vector`:

    .. code-block:: c
        
        new_vector_long = EVSpaceAPI->Vector_FromArray(arr, EVSpaceAPI->VectorType);
        new_vector_short = Vector_FromArray(arr);

    You can use these macros to massively reduce the length of your code. The
    struct is defined as follows.

    .. code-block:: c

        typedef struct {
            /* type objects */
            PyTypeObject* VectorType;
            PyTypeObject* MatrixType;
            PyTypeObject* AnglesType;
            PyTypeObject* OrderType;
            PyTypeObject* RefFrameType;

            /* constructors */
            EVSpace_Vector* (*Vector_FromArray)(double*, PyTypeObject*);
            EVSpace_Vector* (*Vector_StealArray)(double*, PyTypeObject*);
            EVSpace_Matrix* (*Matrix_FromArray)(double*, PyTypeObject*);
            EVSpace_Matrix* (*Matrix_StealArray)(double*, PyTypeObject*);
            EVSpace_Angles* (*Angles_New)(double, double, double, PyTypeObject*);
            EVSpace_Order*  (*Order_New)(EVSpace_Axis, EVSpace_Axis, EVSpace_Axis,
                                         PyTypeObject*);
            EVSpace_ReferenceFrame* (*RefFrame_New)(EVSpace_Order*, EVSpace_Angles*,
                                                    EVSpace_Vector*, PyTypeObject*);

            /* vector number methods */
            EVSpace_Vector* (*Vector_add)(const EVSpace_Vector*,
                                          const EVSpace_Vector*);
            EVSpace_Vector* (*Vector_subtract)(const EVSpace_Vector*,
                                               const EVSpace_Vector*);
            EVSpace_Vector* (*Vector_multiply)(const EVSpace_Vector*, double);
            EVSpace_Vector* (*Vector_multiply_m)(const EVSpace_Vector*,
                                                 const EVSpace_Matrix*);
            EVSpace_Vector* (*Vector_divide)(const EVSpace_Vector*, double);
            void (*Vector_iadd)(EVSpace_Vector*, const EVSpace_Vector*);
            void (*Vector_isubtract)(EVSpace_Vector*, const EVSpace_Vector*);
            void (*Vector_imultiply)(EVSpace_Vector*, double);
            void (*Vector_idivide)(EVSpace_Vector*, double);
            EVSpace_Vector* (*Vector_negative)(const EVSpace_Vector*);

            /* matrix number methods */
            EVSpace_Matrix* (*Matrix_add)(const EVSpace_Matrix*,
                                          const EVSpace_Matrix*);
            EVSpace_Matrix* (*Matrix_subtract)(const EVSpace_Matrix*, 
                                               const EVSpace_Matrix*);
            EVSpace_Vector* (*Matrix_multiply_v)(const EVSpace_Matrix*,
                                                 const EVSpace_Vector*);
            EVSpace_Matrix* (*Matrix_multiply_m)(const EVSpace_Matrix*,
                                                 const EVSpace_Matrix*);
            EVSpace_Matrix* (*Matrix_multiply_s)(const EVSpace_Matrix*, double);
            EVSpace_Matrix* (*Matrix_divide)(const EVSpace_Matrix*, double);
            void (*Matrix_iadd)(EVSpace_Matrix*, const EVSpace_Matrix*);
            void (*Matrix_isubtract)(EVSpace_Matrix*, const EVSpace_Matrix*);
            void (*Matrix_imultiply)(EVSpace_Matrix*, double);
            void (*Matrix_idivide)(EVSpace_Matrix*, double);
            EVSpace_Matrix* (*Matrix_negative)(const EVSpace_Matrix*);

            /* vector class methods */
            double (*Vector_mag)(const EVSpace_Vector*);
            double (*Vector_mag2)(const EVSpace_Vector*);
            void (*Vector_normalize)(EVSpace_Vector*);

            /* module methods */
            double (*Vector_dot)(const EVSpace_Vector*, const EVSpace_Vector*);
            EVSpace_Vector* (*Vector_cross)(const EVSpace_Vector*,
                                            const EVSpace_Vector*);
            EVSpace_Vector* (*Vector_norm)(const EVSpace_Vector*);
            double (*Vector_vang)(const EVSpace_Vector*, const EVSpace_Vector*);
            EVSpace_Vector* (*Vector_vxcl)(const EVSpace_Vector*,
                                           const EVSpace_Vector*);
            EVSpace_Vector* (*Vector_proj)(const EVSpace_Vector*,
                                           const EVSpace_Vector*);
            double (*Matrix_det)(const EVSpace_Matrix*);
            EVSpace_Matrix* (*Matrix_transpose)(const EVSpace_Matrix*);

            /* rotation orders */
            const EVSpace_Order* Order_XYZ;
            const EVSpace_Order* Order_XZY;
            const EVSpace_Order* Order_YXZ;
            const EVSpace_Order* Order_YZX;
            const EVSpace_Order* Order_ZXY;
            const EVSpace_Order* Order_ZYX;
            const EVSpace_Order* Order_XYX;
            const EVSpace_Order* Order_XZX;
            const EVSpace_Order* Order_YXY;
            const EVSpace_Order* Order_YZY;
            const EVSpace_Order* Order_ZXZ;
            const EVSpace_Order* Order_ZYZ;

            /* rotation matrix methods */
            EVSpace_Matrix* (*Get_matrix)(EVSpace_Axis, double);
            EVSpace_Matrix* (*Get_euler)(const EVSpace_Order*, const EVSpace_Angles*);
            EVSpace_Matrix* (*Get_from_to)(const EVSpace_Order*,
                                           const EVSpace_Angles*,
                                           const EVSpace_Order*, 
                                           const EVSpace_Angles*);

            /* rotate vector methods */
            EVSpace_Vector* (*Rotate_axis_to)(EVSpace_Axis, double, 
                                              const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_axis_from)(EVSpace_Axis, double, 
                                                const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_euler_to)(const EVSpace_Order*, 
                                               const EVSpace_Angles*,
                                               const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_euler_from)(const EVSpace_Order*, 
                                                 const EVSpace_Angles*,
                                                 const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_matrix_to)(const EVSpace_Matrix*, 
                                                const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_matrix_from)(const EVSpace_Matrix*, 
                                                  const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_offset_to)(const EVSpace_Matrix*, 
                                                const EVSpace_Vector*,
                                                const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_offset_from)(const EVSpace_Matrix*, 
                                                  const EVSpace_Vector*,
                                                  const EVSpace_Vector*);

            /* reference frame methods */
            EVSpace_Vector* (*Rotate_ref_to)(const EVSpace_ReferenceFrame*,
                                             const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_ref_from)(const EVSpace_ReferenceFrame*,
                                               const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_ref_to_ref)(const EVSpace_ReferenceFrame*,
                                                 const EVSpace_ReferenceFrame*,
                                                 const EVSpace_Vector*);
            EVSpace_Vector* (*Rotate_ref_from_ref)(const EVSpace_ReferenceFrame*,
                                                   const EVSpace_ReferenceFrame*,
                                                   const EVSpace_Vector*);

        } EVSpace_CAPI;

.. c:member:: PyTypeObject* EVSpace_CAPI.VectorType

    The static :c:type:`PyTypeObject` implementation of :py:class:`Vector`, 
    whose C type is :c:type:`EVSpace_Vector`.

.. c:member:: PyTypeObject* EVSpace_CAPI.MatrixType

    The static :c:type:`PyTypeObject` implementation of :py:class:`Matrix`,
    whose C type is :c:type:`EVSpace_Matrix`.

.. c:member:: PyTypeObject* EVSpace_CAPI.AnglesType

    The static :c:type:`PyTypeObject` implementation of :py:class:`Angles`,
    whose C type is :c:type:`EVSpace_Angles`.

.. c:member:: PyTypeObject* EVSpace_CAPI.OrderType

    The static :c:type:`PyTypeObject` implementation of :py:class:`Order`,
    whose C type is :c:type:`EVSpace_Order`.

.. c:member:: PyTypeObject* EVSpace_CAPI.RefFrameType

    The static :c:type:`PyTypeObject` implementation of
    :py:class:`ReferenceFrame`, whose C type is
    :c:type:`EVSpace_ReferenceFrame`.

.. _functions-label:

Capsule Members
---------------

Constructors
^^^^^^^^^^^^

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_FromArray(double* array, \
    PyTypeObject* type)

    Creates a new :c:type:`EVSpace_Vector` instance by copying the data from
    `array`. If `array` is NULL, the vector components will be initialized to 
    zero.

    The `array` parameter must have a size of at least three. Since the data is 
    only copied from the array, it is still the users responsibility to free the
    memory to avoid leaks.

    The `type` parameter must be the :c:`PyTypeObject` implementation of 
    :c:type:`EVSpace_Vector`, which can be found in the capsule as
    :c:type:`VectorType`. 

    :param array: array of size three with the x, y and z values
    :param type: the Python type of :c:type:`EVSpace_Vector`
    :return: a pointer to the new vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_StealArray(double* array, \
    PyTypeObject* type)

    Creates a new :c:type:`EVSpace_Vector` instance by stealing an array pointer
    for the underlying data array. This is a much more efficient constructor 
    than :c:func:`Vector_FromArray` because no copying is done. The underlying 
    data pointer is set to the `array` parameter, and `array` is set to NULL on 
    success.

    The `type` parameter must be the :c:`PyTypeObject` implementation of
    :c:type:`EVSpace_Vector`, which can be found in the capsule as 
    :c:type:`VectorType`.

    .. note::

        The `array` parameter is set to NULL only if this function succeeds. If
        the function does not succeed (NULL is returned) it is still the users
        responsibility to free `array` to avoid a memory leak.

    :param array: array of size three with the x, y and z value. set to NULL
        on success
    :param type: the Python type of :c:type:`EVSpace_Vector`
    :return: a pointer to the new vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Matrix* EVSpace_CAPI.Matrix_FromArray(double* array, \
    PyTypeObject* type)

    Creates a new :c:type:`EVSpace_Matrix` instance by copying the data from
    `array`. If `array` is NULL, the matrix components will be initialized to 
    zero.

    The `array` parameter must have a size of at least nine. Since the data is 
    only copied from the array, it is still the users responsibility to free the
    memory to avoid leaks.

    The `type` parameter must be the :c:`PyTypeObject` implementation of 
    :c:type:`EVSpace_Matrix`, which can be found in the capsule as
    :c:type:`MatrixType`. 

    :param array: array of size nine holding the matrix component values
    :param type: the Python type of :c:type:`EVSpace_Matrix`
    :return: a pointer to the new matrix
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Matrix* EVSpace_CAPI.Matrix_StealArray(double* array, \
    PyTypeObject* type)

    Creates a new :c:type:`EVSpace_Matrix` instance by stealing an array pointer
    for the underlying data array. This is a much more efficient constructor 
    than :c:func:`Matrix_FromArray` because no copying is done. The underlying 
    data pointer is set to the `array` argument, and `array` is set to NULL on 
    success.

    The `type` parameter must be the :c:`PyTypeObject` implementation of
    :c:type:`EVSpace_Matrix`, which can be found in the capsule as 
    :c:type:`MatrixType`.

    .. note::

        The `array` parameter is set to NULL only if this function succeeds. If
        the function does not succeed (NULL is returned) it is still the users
        responsibility to free `array` to avoid a memory leak.

    :param array: array of size nine holding the matrix component values
    :param type: the Python type of :c:type:`EVSpace_Matrix`
    :return: a pointer to the new matrix
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Angles* EVSpace_CAPI.Angles_New(double alpha, double beta, \
    double gamma, PyTypeObject* type)

    Creates a new :c:type:`EVSpace_Angles` instance setting the angles to the
    given arguments.

    :param alpha: first angle of an Euler rotation in radians
    :param beta: second angle of an Euler rotation in radians
    :param gamma: third angle of an euler rotation in radians
    :param type: the Python type of :c:type:`EVSpace_Angles`
    :return: a pointer to the new instance
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Order* EVSpace_CAPI.Order_New(EVSpace_Axis first, \
    EVSpace_Axis second, EVSpace_Axis third, PyTypeObject * type)

    Creates a new :c:type:`EVSpace_Order` instance with the given axes
    parameters as the type attributes.

    :param first: the first axis of rotation
    :param second: the second axis of rotation
    :param third: the third axis of rotation
    :param type: the Python type of :c:type:`EVSpace_Order`
    :return: a pointer to the new instance
    :retval NULL: if an error occurred

.. c:function:: EVSpace_ReferenceFrame* EVSpace_CAPI.RefFrame_New(\
    EVSpace_Order * order, EVSpace_Angles * angles, EVSpace_Vector * offset, \
    PyTypeObject * type)

    Creates a new :c:type:`EVSpace_ReferenceFrame` instance based on the
    Euler rotation orderand angle parameters.An optional offset vector
    can be set as well.

    :param order: the Euler rotation order
    :param angles: the angles associated with the Euler rotation
    :param offset: optional parameter to set the offset of a reference frame,
        should be NULL if there is no offset
    :param type: the Python type of :c:type:`EVSpace_ReferenceFrame`
    :return: a pointer to the new reference frame
    :retval NULL: if an error occurred

Vector numeric methods
^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_add(const EVSpace_Vector* \
    lhs, const EVSpace_Vector* rhs)

    Returns a new :c:type:`EVSpace_Vector` equal to :math:`lhs + rhs`.

    :param lhs: vector to be added to
    :param rhs: vector to add to
    :return: the sum of `lhs` and `rhs`
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_subtract(const \
    EVSpace_Vector* lhs, const EVSpace_Vector* rhs)

    Returns a new :c:type:`EVSpace_Vector` equal to :math:`lhs - rhs`.

    :param lhs: vector to be subtracted from
    :param rhs: vector to subtract
    :return: the difference of `lhs`, and `rhs`
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_multiply(const \
    EVSpace_Vector* lhs, double scalar)

    Returns a new :c:type:`EVSpace_Vector` equal to :math:`lhs * scalar`.

    :param lhs: vector to be multiplied
    :param rhs: scalar to multiply each element of `lhs` by
    :return: the product of `lhs` and `scalar`
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_multiply_m(\
    const EVSpace_Vector* vector, const EVSpace_Matrix* matrix)

    Computes the left vector multiplication of a matrix.

    :param vector: vector to multiply
    :param matrix: matrix transformation
    :return: the transformed vector
    :retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.Vector_divide(const \
    EVSpace_Vector* lhs, double scalar)

    Returns a new :c:type:`EVSpace_Vector` equal to :math:`lhs / scalar`.
    This is equivalent to 

    .. code-block:: c

        EVSpace_CAPI.EVSpace_Vector_multiply(lhs, 1.0 / scalar);

.. c:function:: void EVSpace_CAPI.Vector_iadd(EVSpace_Vector* self, \
    const EVSpace_Vector* other)

    Inplace vector addition. This function doesn't call any Python code and will
    always succeed.

    :param self: vector to be added to
    :param other: vector to add to `self`

.. c:function:: void EVSpace_CAPI.Vector_isubtract(EVSpace_Vector* self, \
    const EVSpace_Vector* other)

    Inplace vector subtraction. This function doesn't call any Python code and 
    will always succeed.

    :param self: vector to be subtracted from
    :param other: vector to subtract from `self`


.. c:function:: void EVSpace_CAPI.Vector_imultiply(EVSpace_Vector* self, \
    double scalar)

    Inplace scalar multiplication. This function doesn't call any Python code 
    and will always succeed.

    :param self: vector to multiply
    :param scalar: scalar to multiply `self` by

.. c:function:: void EVSpace_CAPI.Vector_idivide(EVSpace_Vector* self, \
    double scalar)

    Inplace scalar division. This function doesn't call any Python code and will
    always succeed.

    :param self: vector to divide
    :param scalar: scalar to divide `self` by

.. c:function:: PyObject* EVSpace_CAPI.Vector_negative(const \
    EVSpace_Vector* self)

    Urnary negative operator, returns a new vector pointing in the opposite
    direction of `self`.

    :param self: vector to negate
    :return: the negative vector of `self`
    :retval NULL: if an error occurrs

Matrix numeric methods
^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: PyObject* EVSpace_CAPI.Matrix_add(const EVSpace_Matrix* \
    lhs, const EVSpace_Matrix* rhs)

    Returns a new matrix equal to :math:`lhs + rhs`.

    :param lhs: first matrix
    :param rhs: second matrix
    :return: matrix sum of `lhs` and `rhs`
    :retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.Matrix_subtract(const \
    EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs)

    Returns a new matrix equal to :math:`lhs - rhs`.

    :param lhs: first matrix
    :param rhs: second matrix
    :return: matrix difference of `lhs` and `rhs`
    :retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.Matrix_multiply_v(const \
    EVSpace_Matrix* matrix, const EVSpace_Vector* vector)

    Left-hand matrix multiplication of a vector.

    :param matrix: matrix to multiply `vector` by
    :param vector: vector to multiply
    :return: a new transformed vector
    :retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.Matrix_multiply_m(const \
    EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs)
    
    Compound two transformations via matrix multiplication.

    :param lhs: first matrix
    :param rhs: second matrix
    :return: the compounded matrix
    :retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.Matrix_multiply_s(const \
    EVSpace_Matrix* self, double scalar)

    Scalar matrix multiplication.

    :param self: matrix to multipliy
    :param scalar: scalar to multiply each component of `matrix` by
    :return: matrix product
    :retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.Matrix_divide(const \
    EVSpace_Matrix* lhs, double scalar)

    Matrix scalar division.

    :param lhs: matrix to divide
    :param scalar: scalar to divide each component of `matrix` by
    :return: matrix quotient
    :retval NULL: if an error occurred

.. c:function:: void EVSpace_CAPI.Matrix_iadd(EVSpace_Matrix* self, \
    const EVSpace_Matrix* other)

    Inplace matrix addition, adding `other` to `self`

    :param self: matrix to be added to
    :param other: matrix to add to `self`

.. c:function:: void EVSpace_CAPI.Matrix_isubtract(EVSpace_Matrix* self, \
    const EVSpace_Matrix* other)

    Inplace matrix subtraction, subtracting `other` from `self`.

    :param self: matrix to be subtracted from
    :param other: matrix to subtract

.. c:function:: void EVSpace_CAPI.Matrix_imultiply(EVSpace_Matrix* \
    self, double scalar)

    Inplace scalar multiplication.

    :param self: matrix to multiply
    :param scalar: scalar to multiply each component of `matrix` by

.. c:function:: void EVSpace_CAPI.Matrix_idivide(EVSpace_Matrix* self, \
    double other)

    Inplace scalar division.

    :param self: matrix to divide
    :param scalar: scalar to divide each component of `matrix` by

.. c:function:: PyObject* EVSpace_CAPI.Matrix_negative(const \
    EVSpace_Matrix* matrix)

    Unary negative operator, negates each component of `matrix`.

    :param matrix: matrix to negate
    :return: negative matrix of `matrix`
    :retval NULL: if an error occurred

Vector instance methods
^^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: double EVSpace_CAPI.Vector_mag(const EVSpace_Vector* vector)

    Computes the magnitude, or length, of a vector. This function doesn't call any
    Python code and always succeeds.

    :param vector: vector to compute the magnitude of
    :return: the length of `vector`

.. c:function:: double EVSpace_CAPI.Vector_mag2(const EVSpace_Vector* \
    vector)

    Computes the square of the magnitude of a vector. This is more efficient than
    squaring the value returned from :c:func:`EVSpace_mag` and also avoids rounding
    errors.

    :param vector: vector to find the magnitude of
    :return: the square of the length of `vector`

.. c:function:: void EVSpace_CAPI.Vector_normalize(EVSpace_Vector* self)

    Normalizes a vector inplace, which maintains its direction but changes its
    length to 1.0.

    :param self: vector to normalize

Vector module methods
^^^^^^^^^^^^^^^^^^^^^

.. c:function:: double EVSpace_CAPI.Vector_dot(const EVSpace_Vector* lhs, const \
    EVSpace_Vector* rhs)

    Computes the dot product of two vectors.

    :param lhs: first vector
    :param rhs: second vector
    :return: the dot product of `lhs` and `rhs`

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_cross(const EVSpace_Vector* lhs, \
    const EVSpace_Vector* rhs)

    Computes the cross product of two vectors. This uses a right-hand coordinate
    frame. This vector can be negated to compute a cross product for a left-hand
    coordinate frame.

    :param lhs: first vector
    :param rhs: second vector
    :return: the right-handed cross product of `lhs` and `rhs`
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_norm(const EVSpace_Vector* vector)

    Returns a new vector equal to the norm of `self`. The returned vector points
    in the same difection as `self` but has a length of 1.0.

    :param self: vector to compute the norm of
    :return: a normalized version of `self`
    :retval NULL: if an error occurred

.. c:function:: double EVSpace_CAPI.Vector_vang(const EVSpace_Vector* from, \
    const EVSpace_Vector* to)

    Compute the angle between two vectors. This is always the shortest angle 
    between `from` and `to`.

    Note: don't be confused by the nomenclature of the parameters,
    :c:`EVSpace_vang(from, to)` is equal to :c:`EVSpace_vang(to, from)`.

    :param from: first vector
    :param to: second vector
    :return: the shortest angle between `from` and `to` in radians

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_vxcl(const EVSpace_Vector* vector, \
    const EVSpace_Vector* exclude)

    Creates a new vector equal to `vector` with all parts of `exclude` removed
    from it. This is in effect the vector projection of `vector` on the plane
    whose normal vector is `exclude`. The result of this function and the 
    `exclude` vector are linearly independent vectors.

    :param vector: starting vector
    :param exclude: vector to exclude from `vector`
    :return: the projection of `vector` onto the plane whose normal vector is
        `exclude`.
    :retval NULL: if an error occurred
    
.. c:function:: EVSpace_Vector* EVSpace_CAPI.Vector_proj(const EVSpace_Vector* proj, \
    const EVSpace_Vector* onto)

    The projection of one vector onto another. If the angle between vectors u and v
    is theta, u projected onto v is the vector pointing in the direction of v whose
    length is :math:`|u| * cos(theta)`. If the dot product of u and v is negative,
    this vector points in the opposite direction of v.

    :param proj: vector to project
    :param onto: vector to be projected on
    :return: the projection of `proj` onto `onto`
    :retval NULL: if an error occurred

Matrix module methods
^^^^^^^^^^^^^^^^^^^^^

.. c:function:: double EVSpace_CAPI.Matrix_det(const EVSpace_Matrix* matrix)

    Computes the determinate of a matrix.

    :param matrix: matrix to compute the determinate of
    :return: the determinate of `matrix`

.. c:function:: EVSpace_Matrix* EVSpace_CAPI.Matrix_transpose(const EVSpace_Matrix* \
    matrix)

    Returns a new matrix equal to the transpose of `matrix`. The returned matrix 
    of this function has the values of `matrix` but with the rows and columns
    switched.

    :param matrix: matrix to transpose
    :return: the transpose of `matrix`
    :retval NULL: if an error occurred

Rotation order instances
^^^^^^^^^^^^^^^^^^^^^^^^

.. c:var:: const EVSpace_Order* Order_XYZ
.. c:var:: const EVSpace_Order* Order_XZY
.. c:var:: const EVSpace_Order* Order_YXZ
.. c:var:: const EVSpace_Order* Order_YZX
.. c:var:: const EVSpace_Order* Order_ZXY
.. c:var:: const EVSpace_Order* Order_ZYX
.. c:var:: const EVSpace_Order* Order_XYX
.. c:var:: const EVSpace_Order* Order_XZX
.. c:var:: const EVSpace_Order* Order_YXY
.. c:var:: const EVSpace_Order* Order_YZY
.. c:var:: const EVSpace_Order* Order_ZXZ
.. c:var:: const EVSpace_Order* Order_ZYZ

Rotation matrix methods
^^^^^^^^^^^^^^^^^^^^^^^

The following functions create a rotation matrix, indicated by the function name.

.. c:function:: EVSpace_Matrix* EVSpace_CAPI.Get_matrix(EVSpace_Axis axis, \
    double angle)

    C equivalent to :py:func:`getMatrixAxis`.

    :param axis: axis to rotate around
    :param angle: angle in radians
    :return: the equivalent rotation matrix
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Matrix* EVSpace_CAPI.Get_euler(const EVSpace_Order* order, \
    const EVSpace_Angles* angles)

    C equivalent to :py:func:`getMatrixEuler`.

    :param order: Euler order 
    :param angles: rotation angles
    :return: the equivalent rotation matrix
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Matrix* EVSpace_CAPI.Get_from_to(const \
    EVSpace_Order* orderFrom, const EVSpace_Angles* anglesFrom, const \
    EVSpace_Order* orderTo, const EVSpace_Angles* anglesTo)

    Computes the rotation matrix that defines moving from one reference frame
    to another. Both reference frames must be defined as an Euler rotation. C
    equivalent to :py:func:`getMatrixFromTo`.

    :param orderFrom: Euler order of the reference frame moving from
    :param anglesFrom: rotation angles of the reference frame moving from
    :param orderTo: Euler order of the reference frame moving to
    :param anglesTo: rotation angles of the reference frame moving to
    :return: the equivalent rotation matrix
    :reval NULL: if an error occurred

Vector rotate methods
^^^^^^^^^^^^^^^^^^^^^

The following functions rotate vectors either to or from a reference frame,
indecated by the function name.

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_axis_to(EVSpace_Axis \
    axis, double angle, const EVSpace_Vector* vector)

    C equivalent to :py:func:`rotateAxisTo`.

    :param axis: rotation axis
    :param angle: rotation angle in radians
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_axis_from(EVSpace_Axis \
    axis, double angle, const EVSpace_Vector* vector)

    C equivalent to :py:func:`rotateAxisFrom`.

    :param axis: rotation axis
    :param angle: rotation angle in radians
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_euler_to(const \
    EVSpace_Order* order, const EVSpace_Angles* angles, const \
    EVSpace_Vector* vector)

    C equivalent to :py:func:`rotateEulerTo`.

    :param order: Euler rotation order
    :param angles: rotation angles
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_euler_from(const \
    EVSpace_Order* order, const EVSpace_Angles* angles, const \
    EVSpace_Vector* vector)

    C equivalent to :py:func:`rotateEulerFrom`.

    :param order: Euler rotation order
    :param angles: rotation angles
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_matrix_to(const \
    EVSpace_Matrix* matrix, const EVSpace_Vector* vector)

    C equivalent to :py:func:`rotateMatrixTo`.

    :param matrix: a rotation matrix
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_matrix_from(const \
    EVSpace_Matrix* matrix, const EVSpace_Vector* vector)

    C equivalent to :py:func:`rotateMatrixTo`.

    :param matrix: a rotation matrix
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_offset_to(const \
    EVSpace_Matrix* matrix, const EVSpace_Vector* offset, const \
    EVSpace_Vector* vector)

    C equivalent to :py:func:`rotateOffsetTo`.

    :param matrix: a rotation matrix
    :param offset: a vector pointing to the offset reference frame's origin
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_offset_from(const \
    EVSpace_Matrix* matrix, const EVSpace_Vector* offset, const \
    EVSpace_Vector* vector)

    C equivalent to :py:func:`rotateOffsetFrom`.

    :param matrix: a rotation matrix
    :param offset: a vector pointing to the offset reference frame's origin
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

Reference frame functions
^^^^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_ref_to(const \
    EVSpace_ReferenceFrame* frame, const EVSpace_Vector* vector)

    Rotates a vector to a reference frame via an EVSpace_ReferenceFrame
    object. C equivalent to :py:meth:`ReferenceFrame.rotateTo`.

    :param frame: reference frame object
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_ref_from(const \
    EVSpace_ReferenceFrame* frame, const EVSpace_Vector* vector)

    Rotates a vector from a reference frame via an EVSpace_ReferenceFrame
    object. C equivalent to :py:meth:`ReferenceFrame.rotateTo`.

    :param frame: reference frame object
    :param vector: vector to rotate
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_ref_to_ref(\
    const EVSpace_ReferenceFrame* frame, const EVSpace_ReferenceFrame* to, \
    const EVSpace_Vector* vector)

    Rotates a vector from a reference frame, to another. C equivalent to
    :py:meth:`ReferenceFrame.rotateToFrame`.

    :param frame: reference frame moving from
    :param to: reference frame moving to
    :return: the rotated vector
    :retval NULL: if an error occurred

.. c:function:: EVSpace_Vector* EVSpace_CAPI.Rotate_ref_from_ref(\
    const EVSpace_ReferenceFrame* frame, const EVSpace_ReferenceFrame* from, \
    const EVSpace_Vector* vector)

    Rotates a vector to a reference frame, from another. C equivalent to
    :py:meth:`ReferenceFrame.rotateFromFrame`.

    :param frame: reference frame moving from
    :param to: reference frame moving to
    :return: the rotated vector
    :retval NULL: if an error occurred