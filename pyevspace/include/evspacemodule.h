#ifndef EVSPACE_MODULE_H
#define EVSPACE_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

/* C types for EVectorand EMatrix */
typedef struct {
	PyObject_HEAD
	double *data;			/* x, y, z component of vectors */
	int itr_number;
	Py_ssize_t length;
} EVSpace_Vector;

typedef struct {
	PyObject_HEAD
	double (*data)[3];		/* row by column ordering */
} EVSpace_Matrix;


/* define macros for accessing EVSpace_Vector array values */
#define EVSpace_VECTOR_GETX(o)		(((EVSpace_Vector*)o)->data[0])
#define EVSpace_VECTOR_GETY(o)		(((EVSpace_Vector*)o)->data[1])
#define EVSpace_VECTOR_GETZ(o)		(((EVSpace_Vector*)o)->data[2])
#define EVSpace_VECTOR_SETX(o, v)	(EVSpace_VECTOR_GETX(o) = (v))
#define EVSpace_VECTOR_SETY(o, v)	(EVSpace_VECTOR_GETY(o) = (v))
#define EVSpace_VECTOR_SETZ(o, v)	(EVSpace_VECTOR_GETZ(o) = (v))


/* define macros for accessing EVSpace_Matrix values */
/**
 * Mapping from row, column indices: (3 * row) + column.
 * Bitwise achieved by (3 * r) + c => ((2 + 1) * r) + c
 *	=> (2 * r) + r + c => (r << 1) + r + c
 */
/*#define ROWCOL_TOINDEX(r, c)	(((r)<<1) + (r) + (c))
#define EVSpace_MATRIX_GET(o, r, c)		(((EVSpace_Matrix*)o)->data[ROWCOL_TOINDEX(r, c)])
#define EVSpace_MATRIX_SET(o, r, c, v)	(EVSpace_MATRIX_GET(o, r, c) = (v))*/
#define EVSpace_MATRIX_GET(o, r, c)		(((EVSpace_Matrix*)o)->data[r][c])
#define EVSpace_MATRIX_SET(o, r, c, v)	(EVSpace_MATRIX_GET(o, r, c) = (v))


/* Define structure for C API */
typedef struct {
	/* type objects */
	PyTypeObject* VectorType;
	PyTypeObject* MatrixType;

	/* constructors */
	PyObject* (*Vector_FromArray)(double*, PyTypeObject*);
	PyObject* (*Vector_StealArray)(double*, PyTypeObject*);
	PyObject* (*Matrix_FromArray)(double(*)[3], PyTypeObject*);
	PyObject* (*Matrix_StealArray)(double(*)[3], PyTypeObject*);

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
	//void (*EVSpace_Matrix_imultiply_matrix)(EVSpace_Matrix*, const EVSpace_Matrix*);
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
	double (*EVSpace_det)(const EVSpace_Matrix*);
	PyObject* (*EVSpace_transpose)(const EVSpace_Matrix*);

} EVSpace_CAPI;

#define EVSpace_CAPSULE_NAME "pyevspace.evspace_CAPI"


/* This is only used by the API and should not be included in
 * evspacemodule.c since it doesn't use the capsule.
 */
#ifndef _EVSPACE_IMPL
/* Define global variable for the C API and a macro for setting it. */
static EVSpace_CAPI* EVSpaceAPI = NULL;

#define EVSpace_IMPORT EVSpaceAPI = (EVSpace_CAPI *)PyCapsule_Import(EVSpace_CAPSULE_NAME, 0)

/* macros for simplified constructor calls */
#define Vector_FromValues(x, y, z)		EVSpaceAPI->Vector_FromValues(x, y, z, EVSpace_CAPI->VectorType)
#define Matrix_FromArray(arr)			EVSpaceAPI->Matrix_FromArray(arr, EVSpace_CAPI->MatrixType)
#define Matrix_StealArray(arr)			EVSpaceAPI->Matrix_StealArray(arr, EVSpace_CAPI->MatrixType)

/* macros for C API */
#define EVSpace_Vector_add(rhs, lhs, ans)			EVSpaceAPI->EVSpace_Vector_add(rhs, lhs, ans)
#define EVSpace_Vector_subtract(rhs, lhs, ans)		EVSpaceAPI->EVSpace_Vector_subtract(rhs, lhs, ans)
#define EVSpace_Vector_multiply(rhs, lhs, ans)		EVSpaceAPI->EVSpace_Vector_multiply(rhs, lhs, ans)
#define EVSpace_Vector_divide(rhs, lhs, ans)		EVSpaceAPI->EVSpace_Vector_divide(rhs, lhs, ans)
#define EVSpace_Vector_iadd(self, other)			EVSpaceAPI->EVSpace_Vector_iadd(self, other)
#define EVSpace_Vector_isubtract(self, other)		EVSpaceAPI->EVSpace_Vector_isubtract(self, other)
#define EVSpace_Vector_imultilpy(self, other)		EVSpaceAPI->EVSpace_Vector_imultiply(self, other)
#define EVSpace_Vector_idivide(self, other)			EVSpaceAPI->EVSpace_Vector_idivide(self, other)
#define EVSpace_Vector_negative(self)				EVSpaceAPI->EVspace_Vector_negative(self)
#define EVSpace_Matrix_add(rhs, lhs)				EVSpaceAPI->EVSpace_Matrix_add(rhs, lhs)
#define EVSpace_Matrix_subtract(rhs, lhs)			EVSpaceAPI->EVSpace_Matrix_subtract(rhs, lhs)
#define EVSpace_Matrix_multiply_vector(rhs, lhs)	EVSpaceAPI->EVSpace_Matrix_multiply_vector(rhs, lhs)
#define EVSpace_Matrix_multiply_matrix(rhs, lhs)	EVSpaceAPI->EVSpace_Matrix_multiply_matrix(rhs, lhs)
#define EVSpace_Matrix_multiply_scalar(rhs, lhs)	EVSpaceAPI->EVSpace_Matrix_multiply_scalar(rhs, lhs)
#define EVSpace_Matrix_divide(rhs, lhs)				EVSpaceAPI->EVSpace_Matrix_divide(rhs, lhs)
#define EVSpace_Matrix_iadd(self, other)			EVSpaceAPI->EVSpace_Matrix_iadd(self, other)
#define EVSpace_Matrix_isubtract(self, other)		EVSpaceAPI->EVSpace_Matrix_isubtract(self, other)
#define EVSpace_Matrix_imultiply(self, other)		EVSpaceAPI->EVSpace_Matrix_imultiply(self, other)
#define EVSpace_Matrix_idivide(self, other)			EVSpaceAPI->EVSpace_Matrix_idivide(self, other)
#define EVSpace_Matrix_negative(self)				EVSpaceAPI->EVSpace_Matrix_negative(self)
/* vector class level methods */
#define EVSpace_mag(self)			EVSpaceAPI->EVSpace_mag(self)
#define EVSpace_mag_squared(self)	EVSpaceAPI->EVSpace_mag_squared(self)
#define EVSpace_normalize(self)		EVSpaceAPI->EVSpace_normalize(self)
/* module level methods */
#define EVSpace_dot(lhs, rhs)		EVSpaceAPI->EVSpace_dot(lhs, rhs)
#define EVSpace_cross(lhs, rhs)		EVSpaceAPI->EVSpace_cross(lhs, rhs)
#define EVSpace_norm(self)			EVSpaceAPI->EVSpace_norm(self)
#define EVSpace_vang(lhs, rhs)		EVSpaceAPI->EVSpace_vang(lhs, rhs)
#define EVSpace_vxcl(lhs, rhs)		EVSpaceAPI->EVSpace_vxcl(lhs, rhs)
#define EVSpace_det(lhs)			EVSpaceAPI->EVSpace_det(lhs)
#define EVSpace_transpose(self)		EVSpaceAPI->EVSpace_transpose(self)

#endif // !defined(_EVSPACE_IMPL)

#ifdef __cplusplus
}
#endif

#endif // !defined(EVSPACE_MODULE_H)
