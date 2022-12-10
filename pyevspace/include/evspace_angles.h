#ifndef EVSPACE_ANGLES_H
#define EVSPACE_ANGLES_H

#include <Python.h>
#include <math.h>

#define PI		3.14159265358979323846
#define PI_2	1.57079632679489661923

// forward declaration
static PyTypeObject EVSpace_AnglesType;

#define Angles_Check(o)		PyObject_TypeCheck(o, &EVSpace_AnglesType)

typedef struct {
	PyObject_HEAD
	double alpha;
	double beta;
	double gamma;
}EVSpace_Angles;

static PyObject*
new_angles_ex(double alpha, double beta, double gamma, PyTypeObject* type);

typedef enum {
	X_AXIS = 0,
	Y_AXIS = 1,
	Z_AXIS = 2
} EVSpace_Axis;

static PyTypeObject EVSpace_OrderType;

typedef struct {
	PyObject_HEAD
	EVSpace_Axis first;
	EVSpace_Axis second;
	EVSpace_Axis third;
} EVSpace_Order;

// will expand an EVSpace_Order* to the axis for method calls
#define ORDER_AS_AXIS(o)	o->first, o->second, o->third

static PyObject*
new_order_ex(EVSpace_Axis first, EVSpace_Axis second, EVSpace_Axis third, 
	PyTypeObject* type);

static PyObject*
order_new(PyTypeObject* type, PyObject* args, PyObject* Py_UNUSED(_));

// put the order instances here? how do we fill them for the public API?

#endif // EVSPACE_ANGLES_H
