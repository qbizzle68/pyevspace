#ifndef __EVSPACE_COMMON_H
#define __EVSPACE_COMMON_H

#include <Python.h>

#define BUFFER_RELEASE_SHAPE	0x1

static int
get_sequence_state(PyObject* arg, double* arr);

static int
double_almost_eq(double a, double b);

#endif // __EVSPACE_COMMON_H