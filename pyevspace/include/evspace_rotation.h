#ifndef EVSPACE_ROTATION_H
#define EVSPACE_ROTATION_H

#include <Python.h>
#include <evspace_angles.h>
#include <evspace_vector.h>
#include <evspace_matrix.h>

// forward declaration
static PyTypeObject EVSpace_RotationType;

typedef struct {
	PyObject_HEAD
	EVSpace_Order* order;
	EVSpace_Angles* angles;
	EVSpace_Matrix* matrix;
} EVSpace_Rotation;


/* constructor */

static PyObject*
new_rotation_ex(const EVSpace_Order* order, const EVSpace_Angles* angles, 
	PyTypeObject* type);


/* generate matrices */

static EVSpace_Matrix*
get_rotation_matrix(EVSpace_Axis axis, double angle);

static EVSpace_Matrix*
get_euler_matrix(const EVSpace_Order* order, const EVSpace_Angles* angles);

static EVSpace_Matrix*
get_matrix_from_to(const EVSpace_Order* order_from, const EVSpace_Angles* angles_from,
	const EVSpace_Order* order_to, const EVSpace_Angles* angles_to);



/* rotation methods */

static EVSpace_Vector*
rotate_axis_to(EVSpace_Axis axis, double angle, const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_axis_from(EVSpace_Axis axis, double angle, const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_euler_to(const EVSpace_Order* order, const EVSpace_Angles* angles,
	const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_euler_from(const EVSpace_Order* order, const EVSpace_Angles* angles,
	const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_matrix_to(const EVSpace_Matrix* matrix, const EVSpace_Vector* vector);

static EVSpace_Vector*
rotate_matrix_from(const EVSpace_Matrix* matrix, const EVSpace_Vector* vector);



/* offset reference frames */

static EVSpace_Vector*
rotate_then_offset(const EVSpace_Matrix* matrix, const EVSpace_Vector* offset,
	const EVSpace_Vector* vector);

static EVSpace_Vector*
undo_rotate_then_offset(const EVSpace_Matrix* matrix, const EVSpace_Vector* offset,
	const EVSpace_Vector* vector);

#endif // EVSPACE_ROTATION_H
