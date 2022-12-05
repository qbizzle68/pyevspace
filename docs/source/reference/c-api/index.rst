.. include:: /global.rst

=====
C API
=====

The C-API defines two structs that represent the EVector and EMatrix.

Types
-----
	
.. c:struct:: EVSpace_Vector

	C struct that defines the Python EVector type.

	.. code-block:: c

		typedef struct {
			PyObject_HEAD
			double *data;
			int itr_numer;
		} EVSpace_Vector;

.. c:struct:: EVSpace_Matrix

	C struct that defines the Python EMatrix type.

	.. code-block:: c
		
		typedef struct {
			PyObject_HEAD
			double (*data)[3];
		} EVSpace_Matrix;

Macros
------

.. c:macro:: EVSpace_VECTOR_GETX(o)

	Simplifies accessing the X component of a vector by casting `o` to an
	EVSpace_Vector* and calling the underlying array.

	:param o: a PyObject* or EVSpace_Vector* to get the X component of

.. c:macro:: EVSpace_VECTOR_GETY(o)

	Simplifies accessing the Y component of a vector by casting `o` to an
	EVSpace_Vector* and calling the underlying array.

	:param o: a PyObject* or EVSpace_Vector* to get the Y component of

.. c:macro:: EVSpace_VECTOR_GETZ(o)

	Simplifies accessing the Z component of a vector by casting `o` to an
	EVSpace_Vector* and calling the underlying array.

	:param o: a PyObject* or EVSpace_Vector* to get the Z component of

.. c:macro:: EVSpace_VECTOR_SETX(o, v)
	
	Simplifies setting the X component of a vector by casting `o` to an
	EVSpace_Vector* and setting the correct component of the underlying
	array to `v`.

	:param o: a PyObject* or EVSpace_Vector* to set the X component of
	:param v: the value to set the array component to

.. c:macro:: EVSpace_VECTOR_SETY(o, v)
	
	Simplifies setting the Y component of a vector by casting `o` to an
	EVSpace_Vector* and setting the correct component of the underlying
	array to `v`.

	:param o: a PyObject* or EVSpace_Vector* to set the Y component of
	:param v: the value to set the array component to

.. c:macro:: EVSpace_VECTOR_SETZ(o, v)
	
	Simplifies setting the Z component of a vector by casting `o` to an
	EVSpace_Vector* and setting the correct component of the underlying
	array to `v`.

	:param o: a PyObject* or EVSpace_Vector* to set the Z component of
	:param v: the value to set the array component to

.. c:macro:: EVSpace_MATRIX_GET(o, r, c)

	Simplifies accessing a component of `o` by casting to a 
	EVSpace_Matrix* and calling the underlying array.

	:param o: a PyObject* or EVSpace_Vector* to get the component of
	:param r: the row of the component to get
	:param c: the column of the component to get

.. c:macro:: EVSpace_MATRIX_SET(o, r, c, v)

	Simplifies accessing a component of `o` by casting to a 
	EVSpace_Matrix* and setting the correct component of the underlying
	array to `v`.

	:param o: a PyObject* or EVSpace_Vector* to get the component of
	:param r: the row of the component to set
	:param c: the column of the component to get
	:param v: the value to set the array component to