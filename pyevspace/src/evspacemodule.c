#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h> // PyMemberDef
#include <float.h> // DBL_EPSILON
#include <stdint.h> // int32_t, int64_t

/* don't need the extra's for the C API */
#define _EVSPACE_IMPL
#include <evspacemodule.h>
#include <evspace_vector.h>

/* allow source include */
#define __EVSPACE_SOURCE_INCLUDE__
#include "evspace_common.c"
//#include "evspace_vector.c"
#include "evspace_matrix.c"
#include "evspace_angles.c"
#include "evspace_rotation.c"
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
	.tp_name		= "pyevspace.EVector",
	.tp_basicsize	= sizeof(EVSpace_Vector),
	.tp_itemsize	= 0,
	.tp_repr		= (reprfunc)vector_repr,
	.tp_as_number	= &vector_as_number,
	.tp_as_sequence	= &vector_as_sequence,
	.tp_str			= (reprfunc)vector_str,
	.tp_as_buffer	= &vector_buffer,
	.tp_flags		= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE,
	.tp_doc			= vector_doc,
	.tp_richcompare	= (richcmpfunc)&vector_richcompare,
	.tp_iter		= (getiterfunc)vector_iter,
	.tp_methods		= vector_methods,
	.tp_new			= (newfunc)vector_new,
	.tp_free		= (freefunc)vector_free
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

static PySequenceMethods angles_as_sequence = {
	.sq_length = (lenfunc)vector_length,
	.sq_item = (ssizeargfunc)angles_get_item,
	.sq_ass_item = (ssizeobjargproc)angles_set_item
};

static PyMemberDef angles_members[] = {
	{"alpha", T_DOUBLE, offsetof(EVSpace_Angles, alpha), 0, 
		"first angle of a rotation"},
	{"beta", T_DOUBLE, offsetof(EVSpace_Angles, beta), 0,
		"second angle of a rotation"},
	{"gamma", T_DOUBLE, offsetof(EVSpace_Angles, gamma), 0,
		"third angle of a rotation"},
	{NULL}
};

static PyMethodDef angles_methods[] = {
	{"__reduce__", (PyCFunction)angles_reduce, METH_NOARGS,
		PyDoc_STR("__reduce__() -> (cls, state")},
	{NULL}
};

PyDoc_STRVAR(angles_doc, "data structure to hold the angles of an Euler rotation");

static PyTypeObject EVSpace_AnglesType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name		= "pyevspace.angles",
	.tp_basicsize	= sizeof(EVSpace_Angles),
	.tp_itemsize	= 0,
	.tp_repr		= (reprfunc)angles_repr,
	.tp_as_sequence = &angles_as_sequence,
	.tp_str			= (reprfunc)angles_str,
	.tp_flags		= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE,
	.tp_doc			= angles_doc,
	.tp_methods		= angles_methods,
	.tp_members		= angles_members,
	.tp_new			= (newfunc)angles_new
};

static PySequenceMethods order_as_sequence = {
	.sq_length = (lenfunc)vector_length,
	.sq_item = (ssizeargfunc)order_get_item,
	.sq_ass_item = (ssizeobjargproc)order_set_item
};

static PyMemberDef order_members[] = {
	{"first", T_INT, offsetof(EVSpace_Order, first), READONLY,
		"first axis of a rotation"},
	{"second", T_INT, offsetof(EVSpace_Order, second), READONLY,
		"second axis of a rotation"},
	{"third", T_INT, offsetof(EVSpace_Order, third), READONLY,
		"third axis of a rotation"},
	{NULL}
};

PyDoc_STRVAR(order_doc, "simple class to hold the axis order of an Euler rotation");

static PyTypeObject EVSpace_OrderType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyevspace.order",
	.tp_basicsize = sizeof(EVSpace_Order),
	.tp_itemsize = 0,
	.tp_repr = (reprfunc)order_repr,
	.tp_as_sequence = &order_as_sequence,
	.tp_str	= (reprfunc)order_str,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_SEQUENCE,
	.tp_doc = order_doc,
	.tp_members = order_members,
	.tp_new = (newfunc)order_new
};

static PyMemberDef rotation_members[] = {
	{"order", T_OBJECT_EX, offsetof(EVSpace_Rotation, order), READONLY,
		"order of axes in the rotation"},
	//{"angles", T_OBJECT_EX, offsetof(EVSpace_Rotation, angles), READONLY,
		//"angles of the rotations"},
	{"matrix", T_OBJECT_EX, offsetof(EVSpace_Rotation, matrix), READONLY,
		"internal matrix describing the rotation"},
	{NULL}
};

static PyGetSetDef rotation_getset[] = {
	{"angles", (getter)rotation_angles_getter, (setter)rotation_angles_setter, 
		"angles of the rotations", NULL},
	{"alpha", (getter)rotation_subangle_getter, (setter)rotation_subangle_setter,
		"alpha angle of the angles attribute", (void*)ROTATION_ANGLE_ALPHA},
	{"beta", (getter)rotation_subangle_getter, (setter)rotation_subangle_setter,
		"beta angle of the angles attribute", (void*)ROTATION_ANGLE_BETA},
	{"gamma", (getter)rotation_subangle_getter, (setter)rotation_subangle_setter,
		"gamma angle of the angles attribute", (void*)ROTATION_ANGLE_GAMMA},
	{NULL}
};

PyDoc_STRVAR(rotation_doc, "");

static PyTypeObject EVSpace_RotationType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyevspace.rotation",
	.tp_basicsize = sizeof(EVSpace_Rotation),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = rotation_doc,
	.tp_members = rotation_members,
	.tp_getset = rotation_getset,
	.tp_new = (newfunc)rotation_new
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

	capi->Vector_FromArray	= _vector_from_array;
	capi->Vector_StealArray	= _vector_steal_array;
	capi->Matrix_FromArray	= matrix_from_array;
	capi->Matrix_StealArray	= matrix_steal_array;

	capi->EVSpace_Vector_add		= _add_vector_vector;
	capi->EVSpace_Vector_subtract	= _subtract_vector_vector;
	capi->EVSpace_Vector_multiply	= _multiply_vector_scalar;
	capi->EVSpace_Vector_divide		= _divide_vector_scalar;
	capi->EVSpace_Vector_iadd		= _iadd_vector_vector;
	capi->EVSpace_Vector_isubtract	= _isubtract_vector_vector;
	capi->EVSpace_Vector_imultiply	= _imultiply_vector_scalar;
	capi->EVSpace_Vector_idivide	= _idivide_vector_scalar;
	capi->EVSpace_Vector_negative	= _negative_vector;

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

#define PYEVSPACE_ADD_DICT(dict, name, value_expr)			\
	{														\
		PyObject* value = (value_expr);						\
		if (!value)											\
			return -1;										\
															\
		if (PyDict_SetItemString(dict, name, value) < 0) {	\
			Py_DECREF(value);								\
			return -1;										\
		}													\
		Py_DECREF(value);									\
	}

#define PYEVSPACE_ADD_OBJECT(m, name, value_expr)		\
	{													\
		PyObject* value = (value_expr);					\
		if (!value)										\
			return -1;									\
														\
		if (PyModule_AddObject(m, name, value) < 0) {	\
			Py_DECREF(value);							\
			return -1;									\
		}												\
	}

#define PYEVSPACE_ADD_ORDER(m, cap, order, f, s, l)			\
	{														\
		PyObject* val = (PyObject*)new_order(f, s, l);		\
		PYEVSPACE_ADD_OBJECT(m, #order, val);				\
		cap->EVSpace_##order = (const EVSpace_Order*)val;	\
	}

static int
_pyevspace_exec(PyObject* module)
{
	PyTypeObject* types[] = {
		&EVSpace_VectorType,
		&EVSpace_MatrixType,
		&EVSpace_AnglesType,
		&EVSpace_OrderType,
		&EVSpace_RotationType
	};

	for (int i = 0; i < Py_ARRAY_LENGTH(types); i++) {
		if (PyModule_AddType(module, types[i]) < 0)
			return -1;
	}

	double arr[3] = { 1.0, 0.0, 0.0 };
	PyObject* dict = EVSpace_VectorType.tp_dict;
	PYEVSPACE_ADD_DICT(dict, "e1", new_vector(arr));
	arr[0] = 0.0, arr[1] = 1.0;
	PYEVSPACE_ADD_DICT(dict, "e2", new_vector(arr));
	arr[1] = 0.0, arr[2] = 1.0;
	PYEVSPACE_ADD_DICT(dict, "e3", new_vector(arr));

	double mat[9] = { 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0 };
	dict = EVSpace_MatrixType.tp_dict;
	PYEVSPACE_ADD_DICT(dict, "I", new_matrix(mat));

	if (PyModule_AddIntConstant(module, "X_AXIS", X_AXIS) < 0)
		return -1;

	if (PyModule_AddIntConstant(module, "Y_AXIS", Y_AXIS) < 0)
		return -1;

	if (PyModule_AddIntConstant(module, "Z_AXIS", Z_AXIS) < 0)
		return -1;

	EVSpace_CAPI* capi = get_evspace_capi();
	if (!capi)
		return -1;

	PYEVSPACE_ADD_ORDER(module, capi, XYZ, X_AXIS, Y_AXIS, Z_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, XZY, X_AXIS, Z_AXIS, Y_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, YXZ, Y_AXIS, X_AXIS, Z_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, YZX, Y_AXIS, Z_AXIS, X_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, ZXY, Z_AXIS, X_AXIS, Y_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, ZYX, Z_AXIS, Y_AXIS, X_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, XYX, X_AXIS, Y_AXIS, X_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, XZX, X_AXIS, Z_AXIS, X_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, YXY, Y_AXIS, X_AXIS, Y_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, YZY, Y_AXIS, Z_AXIS, Y_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, ZXZ, Z_AXIS, X_AXIS, Z_AXIS);
	PYEVSPACE_ADD_ORDER(module, capi, ZYZ, Z_AXIS, Y_AXIS, Z_AXIS);

	PyObject* capsule = PyCapsule_New(capi, EVSpace_CAPSULE_NAME, 
		evspace_destructor);
	if (!capsule) {
		free(capi);
		return -1;
	}

	if (PyModule_AddObject(module, "evspace_CAPI", capsule) < 0) {
		Py_DECREF(capsule);
		return -1;
	}

	return 0;
}

PyMODINIT_FUNC
PyInit__pyevspace(void)
{
	PyObject* module = PyModule_Create(&EVSpace_Module);
	if (!module)
		return NULL;

	if (_pyevspace_exec(module) < 0) {
		Py_DECREF(module);
		return NULL;
	}

	return module;
}

PyMODINIT_FUNC
PyInit__pyevspace2(void)
{	
	PyObject* m = NULL, * capsule = NULL;
	EVSpace_CAPI* capi = NULL;

	if (PyType_Ready(&EVSpace_VectorType) < 0)
		return NULL;

	if (PyType_Ready(&EVSpace_MatrixType) < 0)
		return NULL;

	EVSpace_Vector* vector 
		= (EVSpace_Vector*)_vector_from_array(NULL, &EVSpace_VectorType);
	if (!vector) {
		return NULL;
	}

	vector->data[0] = 1.0;
	if (PyDict_SetItemString(EVSpace_VectorType.tp_dict, "e1", (PyObject*)vector) < 0) {
		Py_DECREF(vector);
		return NULL;
	}
	Py_DECREF(vector);
	
	vector = (EVSpace_Vector*)_vector_from_array(NULL, &EVSpace_VectorType);
	if (!vector)
		return NULL;

	vector->data[1] = 1.0;
	if (PyDict_SetItemString(EVSpace_VectorType.tp_dict, "e2", (PyObject*)vector) < 0) {
		Py_DECREF(vector);
		return NULL;
	}
	Py_DECREF(vector);

	vector = (EVSpace_Vector*)_vector_from_array(NULL, &EVSpace_VectorType);
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
	if (PyModule_AddType(m, &EVSpace_MatrixType) < 0) {
		Py_DECREF(m);
		Py_DECREF(&EVSpace_VectorType);
		Py_DECREF(&EVSpace_MatrixType);
		return NULL;
	}

	Py_INCREF(&EVSpace_AnglesType);
	if (PyModule_AddType(m, &EVSpace_AnglesType) < 0) {
		Py_DECREF(m);
		Py_DECREF(&EVSpace_VectorType);
		Py_DECREF(&EVSpace_MatrixType);
		Py_DECREF(&EVSpace_AnglesType);
	}

	Py_INCREF(&EVSpace_OrderType);
	if (PyModule_AddType(m, &EVSpace_OrderType) < 0)
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
	Py_DECREF(&EVSpace_AnglesType);
	Py_DECREF(&EVSpace_OrderType);

	return NULL;
}