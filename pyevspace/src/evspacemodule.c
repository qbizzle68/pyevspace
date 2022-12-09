#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <float.h> // DBL_EPSILON
#include <stdint.h> // int32_t, int64_t

/* don't need the extra's for the C API */
#define _EVSPACE_IMPL
#include <evspacemodule.h>

/* allow source include */
#define __EVSPACE_SOURCE_INCLUDE__
#include "evspace_common.c"
#include "evspace_vector.c"
#include "evspace_matrix.c"
#undef __EVSPACE_SOURCE_INCLUDE__


/* EVSpace_VectorType definition */

static PyNumberMethods vector_as_number = {
	.nb_add					= (binaryfunc)vector_add,
	.nb_subtract			= (binaryfunc)vector_subtract,
	.nb_multiply			= (binaryfunc)vector_multiply,
	.nb_negative			= (unaryfunc)vector_negative,
	.nb_inplace_add			= (binaryfunc)vector_iadd,
	.nb_inplace_subtract	= (binaryfunc)vector_isubtract,
	.nb_inplace_multiply	= (binaryfunc)vector_imultiply,
	.nb_true_divide			= (binaryfunc)vector_divide,
	.nb_inplace_true_divide	= (binaryfunc)vector_idivide
};

static PySequenceMethods vector_as_sequence = {
	.sq_length		= (lenfunc)vector_length,
	.sq_item		= (ssizeargfunc)vector_get_item,
	.sq_ass_item	= (ssizeobjargproc)vector_set_item
};

static PyBufferProcs vector_buffer = {
	.bf_getbuffer		= (getbufferproc)vector_buffer_get,
	.bf_releasebuffer	= (releasebufferproc)buffer_release
};

static PyMethodDef vector_methods[] = {
	{"mag", (PyCFunction)vector_magnitude, METH_NOARGS, PyDoc_STR("mag() -> the magnitude of a vector")},
	{"mag2", (PyCFunction)vector_magnitude_square, METH_NOARGS, PyDoc_STR("mag2() -> the square of the maginitude of a vector")},
	{"normalize", (PyCFunction)vector_normalize, METH_NOARGS, PyDoc_STR("normalize() -> normalize a vector in place")},
	{"__reduce__", (PyCFunction)vector_reduce, METH_NOARGS, PyDoc_STR("__reduce__() -> (cls, state")},
	{NULL}
};

PyDoc_STRVAR(vector_doc, "Data type representing a 3 dimensional vector in a Euclidean vector space.");

static PyTypeObject EVSpace_VectorType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name	= "pyevspace.EVector",
	.tp_basicsize = sizeof(EVSpace_Vector),
	.tp_itemsize = 0,
	.tp_repr	= (reprfunc)vector_repr,
	.tp_as_number	= &vector_as_number,
	.tp_as_sequence	= &vector_as_sequence,
	.tp_str	= (reprfunc)vector_str,
	.tp_as_buffer	= &vector_buffer,
	.tp_flags	= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE,
	.tp_doc	= vector_doc,
	.tp_richcompare	= (richcmpfunc)&vector_richcompare,
	.tp_iter	= (getiterfunc)vector_iter,
	.tp_methods	= vector_methods,
	.tp_new	= (newfunc)vector_new,
	.tp_free	= (freefunc)vector_free
};


/* EVSpace_MatrixType definition*/

static PyNumberMethods matrix_as_number = {
	.nb_add				= (binaryfunc)matrix_add,
	.nb_subtract		= (binaryfunc)matrix_subtract,
	.nb_multiply		= (binaryfunc)matrix_multiply,
	.nb_negative		= (unaryfunc)negative_matrix,
	.nb_inplace_add		= (binaryfunc)matrix_iadd,
	.nb_inplace_subtract = (binaryfunc)matrix_isubtract,
	.nb_inplace_multiply = (binaryfunc)matrix_imultiply,
	.nb_true_divide		= (binaryfunc)matrix_divide,
	.nb_inplace_true_divide = (binaryfunc)matrix_idivide
};

static PyMappingMethods matrix_as_mapping = {
	.mp_subscript	= (binaryfunc)matrix_get_item,
	.mp_ass_subscript	= (objobjargproc)matrix_set_item
};

static PyBufferProcs matrix_buffer = {
	.bf_getbuffer	= (getbufferproc)matrix_buffer_get,
	.bf_releasebuffer	= (releasebufferproc)buffer_release
};

static PyMethodDef matrix_methods[] = {
	{"__reduce__", (PyCFunction)matrix_reduce, METH_NOARGS, 
		PyDoc_STR("__reduce__() -> (cls, state")},
	{NULL}
};

PyDoc_STRVAR(matrix_doc, "Data type for a matrix in a Euclidean vector space.");

static PyTypeObject EVSpace_MatrixType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name		= "pyevspace.EMatrix",
	.tp_basicsize	= sizeof(EVSpace_Matrix),
	.tp_itemsize	= 0,
	.tp_repr		= (reprfunc)matrix_repr,
	.tp_as_number	= &matrix_as_number,
	.tp_as_mapping	= &matrix_as_mapping,
	.tp_str			= (reprfunc)matrix_str,
	.tp_as_buffer	= &matrix_buffer,
	.tp_flags		= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_MAPPING,
	.tp_doc			= matrix_doc,
	.tp_richcompare	= (richcmpfunc)&matrix_richcompare,
	.tp_methods		= matrix_methods,
	.tp_new			= (newfunc)matrix_new,
	.tp_free		= (freefunc)matrix_free
};


/* capsule definition */

static inline 
EVSpace_CAPI* get_evspace_capi(void) 
{
	EVSpace_CAPI* capi = malloc(sizeof(EVSpace_CAPI));
	if (!capi) {
		PyErr_NoMemory();
		return NULL;
	}

	capi->VectorType = &EVSpace_VectorType;
	capi->MatrixType = &EVSpace_MatrixType;

	capi->Vector_FromArray	= vector_from_array;
	capi->Vector_StealArray	= vector_steal_array;
	capi->Matrix_FromArray	= matrix_from_array;
	capi->Matrix_StealArray	= matrix_steal_array;

	capi->EVSpace_Vector_add		= add_vector_vector;
	capi->EVSpace_Vector_subtract	= subtract_vector_vector;
	capi->EVSpace_Vector_multiply	= multiply_vector_scalar;
	capi->EVSpace_Vector_divide		= divide_vector_scalar;
	capi->EVSpace_Vector_iadd		= iadd_vector_vector;
	capi->EVSpace_Vector_isubtract	= isubtract_vector_vector;
	capi->EVSpace_Vector_imultiply	= imultiply_vector_scalar;
	capi->EVSpace_Vector_idivide	= idivide_vector_scalar;
	capi->EVSpace_Vector_negative	= negative_vector;

	capi->EVSpace_Matrix_add				= add_matrix_matrix;
	capi->EVSpace_Matrix_subtract			= subtract_matrix_matrix;
	capi->EVSpace_Matrix_multiply_vector	= multiply_matrix_vector;
	capi->EVSpace_Matrix_multiply_matrix	= multiply_matrix_matrix;
	capi->EVSpace_Matrix_multiply_scalar	= multiply_matrix_scalar;
	capi->EVSpace_Matrix_divide				= divide_matrix_scalar;
	capi->EVSpace_Matrix_iadd				= iadd_matrix_matrix;
	capi->EVSpace_Matrix_isubtract			= isubtract_matrix_matrix;
	capi->EVSpace_Matrix_imultiply_scalar	= imultiply_matrix_scalar;
	capi->EVSpace_Matrix_idivide			= idivide_matrix_scalar;
	capi->EVSpace_Matrix_negative			= capsule_matrix_negative;

	capi->EVSpace_mag			= capsule_vector_magnitude;
	capi->EVSpace_mag_squared	= capsule_vector_magnitude2;
	capi->EVSpace_normalize		= capsule_vector_normalize;

	capi->EVSpace_dot		= capsule_vector_dot;
	capi->EVSpace_cross		= capsule_vector_cross;
	capi->EVSpace_norm		= capsule_vector_norm;
	capi->EVSpace_vang		= capsule_vang;
	capi->EVSpace_vxcl		= capsule_vxcl;
	capi->EVSpace_proj		= capsule_proj;
	capi->EVSpace_det		= capsule_determinate;
	capi->EVSpace_transpose	= capsule_transpose;

	return capi;
}


/* module definition */

static void 
evspace_destructor(PyObject* capi) 
{
	void* ptr = PyCapsule_GetPointer(capi, EVSpace_CAPSULE_NAME);
	free(ptr);
}

static PyMethodDef evspace_methods[] = {
	{"dot", (PyCFunction)vector_dot, METH_FASTCALL,
		PyDoc_STR("dot(lhs, rhs) -> dot product of two EVectors")},
	{"cross", (PyCFunction)vector_cross, METH_FASTCALL,
		PyDoc_STR("cross(lhs, rhs) -> cross product of two EVectors")},
	{"norm", (PyCFunction)vector_norm, METH_FASTCALL,
		PyDoc_STR("norm(vector) -> a normalized version of an EVector")},
	{"vang", (PyCFunction)vector_vang, METH_FASTCALL,
		PyDoc_STR("vang(from, to) -> the shortest angle between two EVector's")},
	{"vxcl", (PyCFunction)vector_vxcl, METH_FASTCALL,
		PyDoc_STR("vxcl(vector, exclude) -> vector with exclude excluded from it")},
	{"proj", (PyCFunction)vector_proj, METH_FASTCALL,
		PyDoc_STR("proj(proj, onto) -> proj projected onto onto")},
	{"det", (PyCFunction)matrix_determinate, METH_FASTCALL,
		PyDoc_STR("det(matrix) -> the determinate of a EMatrix")},
	{"transpose", (PyCFunction)matrix_transpose, METH_FASTCALL,
		PyDoc_STR("transpose(matrix) -> the transpose of an EMatrix")},
	{NULL}
};

PyDoc_STRVAR(evspace_doc, "A 3-dimensional Euclidean vector space module with a vector and matrix type as well as necessary methods to use them.");

static PyModuleDef EVSpace_Module = {
	PyModuleDef_HEAD_INIT,
	.m_name		= "_pyevspace",
	.m_doc		= evspace_doc,
	.m_size		= -1,
	.m_methods	= evspace_methods
};

PyMODINIT_FUNC
PyInit__pyevspace(void)
{	
	PyObject* m = NULL, * capsule = NULL;
	EVSpace_CAPI* capi = NULL;

	if (PyType_Ready(&EVSpace_VectorType) < 0)
		return NULL;

	if (PyType_Ready(&EVSpace_MatrixType) < 0)
		return NULL;

	EVSpace_Vector* vector 
		= (EVSpace_Vector*)vector_from_array(NULL, &EVSpace_VectorType);
	if (!vector) {
		return NULL;
	}

	vector->data[0] = 1.0;
	if (PyDict_SetItemString(EVSpace_VectorType.tp_dict, "e1", (PyObject*)vector) < 0) {
		Py_DECREF(vector);
		return NULL;
	}
	Py_DECREF(vector);
	
	vector = (EVSpace_Vector*)vector_from_array(NULL, &EVSpace_VectorType);
	if (!vector)
		return NULL;

	vector->data[1] = 1.0;
	if (PyDict_SetItemString(EVSpace_VectorType.tp_dict, "e2", (PyObject*)vector) < 0) {
		Py_DECREF(vector);
		return NULL;
	}
	Py_DECREF(vector);

	vector = (EVSpace_Vector*)vector_from_array(NULL, &EVSpace_VectorType);
	if (!vector)
		return NULL;

	vector->data[2] = 1.0;
	if (PyDict_SetItemString(EVSpace_VectorType.tp_dict, "e3", (PyObject*)vector) < 0) {
		Py_DECREF(vector);
		return NULL;
	}
	Py_DECREF(vector);

	EVSpace_Matrix* matrix = (EVSpace_Matrix*)new_matrix_empty;
	if (!matrix) {
		return NULL;
	}
	
	Matrix_COMP(matrix, 0, 0) = 1.0;
	Matrix_COMP(matrix, 1, 1) = 1.0;
	Matrix_COMP(matrix, 2, 2) = 1.0;
	if (PyDict_SetItemString(EVSpace_MatrixType.tp_dict, "I", (PyObject*)matrix) < 0) {
		Py_DECREF(matrix);
		return NULL;
	}
	Py_DECREF(matrix);

	m = PyModule_Create(&EVSpace_Module);
	if (!m)
		return NULL;


	Py_INCREF(&EVSpace_VectorType);
	if (PyModule_AddType(m, &EVSpace_VectorType) < 0) {
		Py_DECREF(m);
		Py_DECREF(&EVSpace_VectorType);
		return NULL;
	}

	Py_INCREF(&EVSpace_MatrixType);
	if (PyModule_AddType(m, &EVSpace_MatrixType) < 0)
		goto cleanup;

	capi = get_evspace_capi();
	if (!capi)
		goto cleanup;

	capsule = PyCapsule_New(capi, EVSpace_CAPSULE_NAME, evspace_destructor);	
	if (!capsule) {
		free(capi);
		goto cleanup;
	}

	if (PyModule_AddObject(m, "evspace_CAPI", capsule) < 0) {
		Py_DECREF(capsule);
		goto cleanup;
	}

	return m;

cleanup:

	Py_DECREF(m);
	Py_DECREF(&EVSpace_VectorType);
	Py_DECREF(&EVSpace_MatrixType);

	return NULL;
}