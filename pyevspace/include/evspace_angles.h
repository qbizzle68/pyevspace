#ifndef EVSPACE_ANGLES_H
#define EVSPACE_ANGLES_H

#include <Python.h>
#include <math.h>

#define PI		3.14159265358979323846
#define PI_2	1.57079632679489661923

typedef struct {
	PyObject_HEAD
	double alpha;
	double beta;
	double gamma;
}EVSpace_Angles;

// forward declaration
static PyTypeObject EVSpace_AnglesType;

static PyObject*
new_angles_ex(double alpha, double beta, double gamma, PyTypeObject* type);

#endif // EVSPACE_ANGLES_H
