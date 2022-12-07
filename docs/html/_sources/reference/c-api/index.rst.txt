.. include:: /global.rst

=====
C API
=====

The C-API defines two structs that represent the EVector and EMatrix. 

.. note::

	All methods in the C API capsule return a :c:type:`PyObject`\ \*.

Types
-----
	
.. c:struct:: EVSpace_Vector

	C struct that defines the Python EVector type. A pointer of this type can be
	cast to or from a :c:type:`PyObject`\ \*.

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

	C struct that defines the Python EMatrix type.

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

	The :c:macro:`EVSpace_Matrix_COMPONENT` handles the mapping for you when 
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

.. c:struct:: EVSpace_CAPI

	Python capsule object that contain all functions of the pyevspace C API. For 
	more details on capsules check out the official Python 
	`docs <https://docs.python.org/3/extending/extending.html#providing-a-c-api-for-an-extension-module>`_.
	The capsule can be imported using the :c:macro:`EVSpace_IMPORT` which sets
	the global pointer :c:var:`EVSpaceAPI` using the :c:`PyCapsule_Import()` 
	method. There are helper macros for every function inside the capsule to avoid
	cumbersome method calls. For example to create a new :c:type:`EVSpace_Vector`

	.. code-block:: c
		
		new_vector_long = EVSpaceAPI->Vector_FromArray(arr, EVSpaceAPI->VectorType);
		new_vector_short = Vector_FromArray(arr);

	you can use the macro to massively reduce the length of your code. The struct
	is defined as follows.

	.. code-block:: c

		typedef struct {
			/* type objects */
			PyTypeObject* VectorType;
			PyTypeObject* MatrixType;

			/* constructors */
			PyObject* (*Vector_FromArray)(double*, PyTypeObject*);
			PyObject* (*Vector_StealArray)(double*, PyTypeObject*);
			PyObject* (*Matrix_FromArray)(double*, PyTypeObject*);
			PyObject* (*Matrix_StealArray)(double*, PyTypeObject*);

			/* vector number methods */
			PyObject* (*EVSpace_Vector_add)(const EVSpace_Vector*, const EVSpace_Vector*);
			PyObject* (*EVSpace_Vector_subtract)(const EVSpace_Vector*, const EVSpace_Vector*);
			PyObject* (*EVSpace_Vector_multiply)(const EVSpace_Vector*, double);
			PyObject* (*EVSpace_Vector_divide)(const EVSpace_Vector*, double);
			void (*EVSpace_Vector_iadd)(EVSpace_Vector*, const EVSpace_Vector*);
			void (*EVSpace_Vector_isubtract)(EVSpace_Vector*, const EVSpace_Vector*);
			void (*EVSpace_Vector_imultiply)(EVSpace_Vector*, double);
			void (*EVSpace_Vector_idivide)(EVSpace_Vector*, double);
			PyObject* (*EVSpace_Vector_negative)(const EVSpace_Vector*);

			/* matrix number methods */
			PyObject* (*EVSpace_Matrix_add)(const EVSpace_Matrix*, const EVSpace_Matrix*);
			PyObject* (*EVSpace_Matrix_subtract)(const EVSpace_Matrix*, const EVSpace_Matrix*);
			PyObject* (*EVSpace_Matrix_multiply_vector)(const EVSpace_Matrix*, const EVSpace_Vector*);
			PyObject* (*EVSpace_Matrix_multiply_matrix)(const EVSpace_Matrix*, const EVSpace_Matrix*);
			PyObject* (*EVSpace_Matrix_multiply_scalar)(const EVSpace_Matrix*, double);
			PyObject* (*EVSpace_Matrix_divide)(const EVSpace_Matrix*, double);
			void (*EVSpace_Matrix_iadd)(EVSpace_Matrix*, const EVSpace_Matrix*);
			void (*EVSpace_Matrix_isubtract)(EVSpace_Matrix*, const EVSpace_Matrix*);
			void (*EVSpace_Matrix_imultiply_scalar)(EVSpace_Matrix*, double);
			void (*EVSpace_Matrix_idivide)(EVSpace_Matrix*, double);
			PyObject* (*EVSpace_Matrix_negative)(const EVSpace_Matrix*);

			/* vector class methods */
			double (*EVSpace_mag)(const EVSpace_Vector*);
			double (*EVSpace_mag_squared)(const EVSpace_Vector*);
			void (*EVSpace_normalize)(EVSpace_Vector*);

			/* module methods */
			double (*EVSpace_dot)(const EVSpace_Vector*, const EVSpace_Vector*);
			PyObject* (*EVSpace_cross)(const EVSpace_Vector*, const EVSpace_Vector*);
			PyObject* (*EVSpace_norm)(const EVSpace_Vector*);
			double (*EVSpace_vang)(const EVSpace_Vector*, const EVSpace_Vector*);
			PyObject* (*EVSpace_vxcl)(const EVSpace_Vector*, const EVSpace_Vector*);
			PyObject* (*EVSpace_proj)(const EVSpace_Vector*, const EVSpace_Vector*);
			double (*EVSpace_det)(const EVSpace_Matrix*);
			PyObject* (*EVSpace_transpose)(const EVSpace_Matrix*);

		} EVSpace_CAPI;

.. c:member:: PyTypeObject* EVSpace_CAPI.VectorType

	The static :c:type:`PyTypeObject` implementation of :c:type:`EVSpace_Vector`.

.. c:member:: PyTypeObject* EVSpace_CAPI.MatrixType

	The static :c:type:`PyTypeObject` implementation of :c:type:`EVSpace_Matrix`.

.. _functions-label:

Functions
---------

Constructors
^^^^^^^^^^^^

.. c:function:: PyObject* EVSpace_CAPI.Vector_FromArray(double* array, \
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

.. c:function:: PyObject* EVSpace_CAPI.Vector_StealArray(double* array, \
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

.. c:function:: PyObject* EVSpace_CAPI.Matrix_FromArray(double* array, \
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

.. c:function:: PyObject* EVSpace_CAPI.Matrix_StealArray(double* array, \
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

Vector numeric methods
^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Vector_add(const EVSpace_Vector* \
	lhs, const EVSpace_Vector* rhs)

	Returns a new :c:type:`EVSpace_Vector` equal to :math:`lhs + rhs`.

	:param lhs: vector to be added to
	:param rhs: vector to add to
	:return: the sum of `lhs` and `rhs`
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Vector_subtract(const \
	EVSpace_Vector* lhs, const EVSpace_Vector* rhs)

	Returns a new :c:type:`EVSpace_Vector` equal to :math:`lhs - rhs`.

	:param lhs: vector to be subtracted from
	:param rhs: vector to subtract
	:return: the difference of `lhs`, and `rhs`
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Vector_multiply(const \
	EVSpace_Vector* lhs, double scalar)

	Returns a new :c:type:`EVSpace_Vector` equal to :math:`lhs * scalar`.

	:param lhs: vector to be multiplied
	:param rhs: scalar to multiply each element of `lhs` by
	:return: the product of `lhs` and `scalar`
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Vector_divide(const \
	EVSpace_Vector* lhs, double scalar)

	Returns a new :c:type:`EVSpace_Vector` equal to :math:`lhs / scalar`.
	This is equivalent to 

	.. code-block:: c

		EVSpace_CAPI.EVSpace_Vector_multiply(lhs, 1.0 / scalar);

.. c:function:: void EVSpace_CAPI.EVSpace_Vector_iadd(EVSpace_Vector* self, \
	const EVSpace_Vector* other)

	Inplace vector addition. This function doesn't call any Python code and will
	always succeed.

	:param self: vector to be added to
	:param other: vector to add to `self`

.. c:function:: void EVSpace_CAPI.EVSpace_Vector_isubtract(EVSpace_Vector* self, \
	const EVSpace_Vector* other)

	Inplace vector subtraction. This function doesn't call any Python code and 
	will always succeed.

	:param self: vector to be subtracted from
	:param other: vector to subtract from `self`


.. c:function:: void EVSpace_CAPI.EVSpace_Vector_imultiply(EVSpace_Vector* self, \
	double scalar)

	Inplace scalar multiplication. This function doesn't call any Python code 
	and will always succeed.

	:param self: vector to multiply
	:param scalar: scalar to multiply `self` by

.. c:function:: void EVSpace_CAPI.EVSpace_Vector_idivide(EVSpace_Vector* self, \
	double scalar)

	Inplace scalar division. This function doesn't call any Python code and will
	always succeed.

	:param self: vector to divide
	:param scalar: scalar to divide `self` by

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Vector_negative(const \
	EVSpace_Vector* self)

	Urnary negative operator, returns a new vector pointing in the opposite
	direction of `self`.

	:param self: vector to negate
	:return: the negative vector of `self`
	:retval NULL: if an error occurrs

Matrix numeric methods
^^^^^^^^^^^^^^^^^^^^^^

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Matrix_add(const EVSpace_Matrix* \
	lhs, const EVSpace_Matrix* rhs)

	Returns a new matrix equal to :math:`lhs + rhs`.

	:param lhs: first matrix
	:param rhs: second matrix
	:return: matrix sum of `lhs` and `rhs`
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Matrix_subtract(const \
	EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs)

	Returns a new matrix equal to :math:`lhs - rhs`.

	:param lhs: first matrix
	:param rhs: second matrix
	:return: matrix difference of `lhs` and `rhs`
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Matrix_multiply_vector(const \
	EVSpace_Matrix* matrix, const EVSpace_Vector* vector)

	Left-hand matrix multiplication of a vector.

	:param matrix: matrix to multiply `vector` by
	:param vector: vector to multiply
	:return: a new transformed vector
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Matrix_multiply_matrix(const \
	EVSpace_Matrix* lhs, const EVSpace_Matrix* rhs)
	
	Compound two transformations via matrix multiplication.

	:param lhs: first matrix
	:param rhs: second matrix
	:return: the compounded matrix
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Matrix_multiply_scalar(const \
	EVSpace_Matrix* self, double scalar)

	Scalar matrix multiplication.

	:param self: matrix to multipliy
	:param scalar: scalar to multiply each component of `matrix` by
	:return: matrix product
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Matrix_divide(const \
	EVSpace_Matrix* lhs, double scalar)

	Matrix scalar division.

	:param lhs: matrix to divide
	:param scalar: scalar to divide each component of `matrix` by
	:return: matrix quotient
	:retval NULL: if an error occurred

.. c:function:: void EVSpace_CAPI.EVSpace_Matrix_iadd(EVSpace_Matrix* self, \
	const EVSpace_Matrix* other)

	Inplace matrix addition, adding `other` to `self`

	:param self: matrix to be added to
	:param other: matrix to add to `self`

.. c:function:: void EVSpace_CAPI.EVSpace_Matrix_isubtract(EVSpace_Matrix* self, \
	const EVSpace_Matrix* other)

	Inplace matrix subtraction, subtracting `other` from `self`.

	:param self: matrix to be subtracted from
	:param other: matrix to subtract

.. c:function:: void EVSpace_CAPI.EVSpace_Matrix_imultiply_scalar(EVSpace_Matrix* \
	self, double scalar)

	Inplace scalar multiplication.

	:param self: matrix to multiply
	:param scalar: scalar to multiply each component of `matrix` by

.. c:function:: void EVSpace_CAPI.EVSpace_Matrix_idivide(EVSpace_Matrix* self, \
	double other)

	Inplace scalar division.

	:param self: matrix to divide
	:param scalar: scalar to divide each component of `matrix` by

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_Matrix_negative(const \
	EVSpace_Matrix* matrix)

	Unary negative operator, negates each component of `matrix`.

	:param matrix: matrix to negate
	:return: negative matrix of `matrix`
	:retval NULL: if an error occurred

Vector type methods
^^^^^^^^^^^^^^^^^^^

.. c:function:: double EVSpace_CAPI.EVSpace_mag(const EVSpace_Vector* vector)

	Computes the magnitude, or length, of a vector. This function doesn't call any
	Python code and always succeeds.

	:param vector: vector to compute the magnitude of
	:return: the length of `vector`

.. c:function:: double EVSpace_CAPI.EVSpace_mag_squared(const EVSpace_Vector* \
	vector)

	Computes the square of the magnitude of a vector. This is more efficient than
	squaring the value returned from :c:func:`EVSpace_mag` and also avoids rounding
	errors.

	:param vector: vector to find the magnitude of
	:return: the square of the length of `vector`

.. c:function:: void EVSpace_CAPI.EVSpace_normalize(EVSpace_Vector* self)

	Normalizes a vector inplace, which maintains its direction but changes its
	length to 1.0.

	:param self: vector to normalize

.. c:function:: double EVSpace_CAPI.EVSpace_dot(const EVSpace_Vector* lhs, const \
	EVSpace_Vector* rhs)

	Computes the dot product of two vectors.

	:param lhs: first vector
	:param rhs: second vector
	:return: the dot product of `lhs` and `rhs`

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_cross(const EVSpace_Vector* lhs, \
	const EVSpace_Vector* rhs)

	Computes the cross product of two vectors. This uses a right-hand coordinate
	frame. This vector can be negated to compute a cross product for a left-hand
	coordinate frame.

	:param lhs: first vector
	:param rhs: second vector
	:return: the right-handed cross product of `lhs` and `rhs`
	:retval NULL: if an error occurred

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_norm(const EVSpace_Vector* vector)

	Returns a new vector equal to the norm of `self`. The returned vector points
	in the same difection as `self` but has a length of 1.0.

	:param self: vector to compute the norm of
	:return: a normalized version of `self`
	:retval NULL: if an error occurred

.. c:function:: double EVSpace_CAPI.EVSpace_vang(const EVSpace_Vector* from, \
	const EVSpace_Vector* to)

	Compute the angle between two vectors. This is always the shortest angle 
	between `from` and `to`.

	Note: don't be confused by the nomenclature of the parameters,
	:c:`EVSpace_vang(from, to)` is equal to :c:`EVSpace_vang(to, from)`.

	:param from: first vector
	:param to: second vector
	:return: the shortest angle between `from` and `to` in radians

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_vxcl(const EVSpace_Vector* vector, \
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
	
.. c:function:: PyObject* EVSpace_CAPI.EVSpace_proj(const EVSpace_Vector* proj, \
	const EVSpace_Vector* onto)

	The projection of one vector onto another. If the angle between vectors u and v
	is theta, u projected onto v is the vector pointing in the direction of v whose
	length is :math:`|u| * cos(theta)`. If the dot product of u and v is negative,
	this vector points in the opposite direction of v.

	:param proj: vector to project
	:param onto: vector to be projected on
	:return: the projection of `proj` onto `onto`
	:retval NULL: if an error occurred

Matrix type methods
^^^^^^^^^^^^^^^^^^^

.. c:function:: double EVSpace_CAPI.EVSpace_det(const EVSpace_Matrix* matrix)

	Computes the determinate of a matrix.

	:param matrix: matrix to compute the determinate of
	:return: the determinate of `matrix`

.. c:function:: PyObject* EVSpace_CAPI.EVSpace_transpose(const EVSpace_Matrix* \
	matrix)

	Returns a new matrix equal to the transpose of `matrix`. The returned matrix 
	of this function has the values of `matrix` but with the rows and columns
	switched.

	:param matrix: matrix to transpose
	:return: the transpose of `matrix`
	:retval NULL: if an error occurred

Macros
------

Type ease of use
^^^^^^^^^^^^^^^^

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

.. c:macro:: EVSpace_MATRIX_COMPONENT(obj, row, col)

	Simplifies accessing a component of `obj` by casting to a 
	EVSpace_Matrix* and calling the underlying array.

	:param obj: a PyObject* or EVSpace_Vector* to get the component of
	:param row: the row of the component to get
	:param col: the column of the component to get

Capsule ease of use
^^^^^^^^^^^^^^^^^^^

.. c:macro:: EVSpace_CAPSULE_NAME

	Unique identifier for the C API capsule.

The following macros are shorthand calls to the methods in the 
:c:type:`EVSpace_CAPI` struct. Details of each method can be found in the
:ref:`functions-label` section.

.. c:macro:: Vector_FromArray(array)
		
	See :c:type:`EVSpace_CAPI.Vector_FromArray`. 
	
	.. note::
		
		This macro handles the `type` argument in the constructor call.

.. c:macro:: Vector_StealArray(array)

	See :c:type:`EVSpace_CAPI.Vector_StealArray`.

	.. note::
		
		This macro handles the `type` argument in the constructor call.

.. c:macro:: Matrix_FromArray(array)

	See :c:type:`EVSpace_CAPI.Matrix_FromArray`.

	.. note::
		
		This macro handles the `type` argument in the constructor call.

.. c:macro:: Matrix_StealArray(array)

	See :c:type:`EVSpace_CAPI.Matrix_StealArray`.

	.. note::

		This macro handles the `type` argument in the constructor call.

.. c:macro:: EVSpace_Vector_add(rhs, lhs)

	See :c:type:`EVSpace_CAPI.EVSpace_Vector_add`.

.. c:macro:: EVSpace_Vector_subtract(rhs, lhs)

	See :c:type:`EVSpace_CAPI.EVSpace_Vector_subtract`.

.. c:macro:: EVSpace_Vector_multiply(self, scalar)

	See :c:type:`EVSpace_CAPI.EVSpace_Vector_multiply`.

.. c:macro:: EVSpace_Vector_divide(self, scalar)

	See :c:type:`EVSpace_CAPI.EVSpace_Vector_divide`.

.. c:macro:: EVSpace_Vector_iadd(self, other)

	See :c:type:`EVSpace_CAPI.EVSpace_Vector_iadd`.

.. c:macro:: EVSpace_Vector_isubtract(self, other)

	See :c:type:`EVSpace_CAPI.EVSpace_Vector_isubtract`.

.. c:macro:: EVspace_Vector_imultiply(self, scalar)

	See :c:type:`EVSpace_CAPI.EVspace_Vector_imultiply`.

.. c:macro:: EVSpace_Vector_idivide(self, scalar)

	See :c:type:`EVSpace_CAPI.EVSpace_Vector_idivide`.

.. c:macro:: EVSpace_Vector_negative(self)

	See :c:type:`EVSpace_CAPI.EVSpace_Vector_negative`.

.. c:macro:: EVSpace_Matrix_add(rhs, lhs)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_add`.

.. c:macro:: EVSpace_Matrix_subtract(rhs, lhs)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_subtract`.

.. c:macro:: EVSpace_Matrix_multiply_vector(rhs, lhs)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_multiply_vector`.

.. c:macro:: EVSpace_Matrix_multiply_matrix(rhs, lhs)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_multiply_matrix`.

.. c:macro:: EVSpace_Matrix_multiply_scalar(self, scalar)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_multiply_scalar`.

.. c:macro:: EVSpace_Matrix_divide(rhs, lhs)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_divide`.

.. c:macro:: EVSpace_Matrix_iadd(self, other)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_iadd`.

.. c:macro:: EVSpace_Matrix_isubtract(self, other)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_isubtract`.

.. c:macro:: EVSpace_Matrix_imultiply(self, scalar)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_imultiply`.

.. c:macro:: EVSpace_Matrix_idivide(self, scalar)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_idivide`.

.. c:macro:: EVSpace_Matrix_negative(self)

	See :c:type:`EVSpace_CAPI.EVSpace_Matrix_negative`.

.. c:macro:: EVSpace_mag(self)

	See :c:type:`EVSpace_CAPI.EVSpace_mag`.

.. c:macro:: EVSpace_mag_squared(self)

	See :c:type:`EVSpace_CAPI.EVSpace_mag_squared`.

.. c:macro:: EVSpace_normalize(self)

	See :c:type:`EVSpace_CAPI.EVSpace_normalize`.

.. c:macro:: EVSpace_dot(lhs, rhs)

	See :c:type:`EVSpace_CAPI.EVSpace_dot`.

.. c:macro:: EVSpace_cross(lhs, rhs)

	See :c:type:`EVSpace_CAPI.EVSpace_cross`.

.. c:macro:: EVSpace_norm(self)

	See :c:type:`EVSpace_CAPI.EVSpace_norm`.

.. c:macro:: EVSpace_vang(from, to)

	See :c:type:`EVSpace_CAPI.EVSpace_vang`.

.. c:macro:: EVSpace_vxcl(vec, xcl)

	See :c:type:`EVSpace_CAPI.EVSpace_vxcl`.

.. c:macro:: EVSpace_proj(proj, onto)

	See :c:type:`EVSpace_CAPI.EVSpace_proj`.

.. c:macro:: EVSpace_det(self)

	See :c:type:`EVSpace_CAPI.EVSpace_det`.

.. c:macro:: EVSpace_transpose(self)

	See :c:type:`EVSpace_CAPI.EVSpace_transpose`.