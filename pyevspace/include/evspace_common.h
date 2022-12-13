#ifndef __EVSPACE_COMMON_H
#define __EVSPACE_COMMON_H

#include <Python.h>
#include <float.h> // DBL_EPSILON
#include <stdint.h> // int32_t, int64_t

#define BUFFER_RELEASE_SHAPE	0x1

static int
__get_sequence_state(PyObject* arg, double* arr)
{
	assert(arr != NULL);

	char* err = "";
	// what do we do with err?
	PyObject* fast_sequence = PySequence_Fast(arg, err);
	if (!fast_sequence) {
		if (PyErr_Occurred() == PyExc_TypeError)
			PyErr_SetString(PyExc_TypeError, "parameter must be a sequence type");
		return -1;
	}

	Py_ssize_t size = PySequence_Fast_GET_SIZE(fast_sequence);
	if (size == 3) {
		PyObject** items = PySequence_Fast_ITEMS(fast_sequence);

		arr[0] = PyFloat_AsDouble(items[0]);
		if (arr[0] == -1.0 && PyErr_Occurred())
		{
			PyErr_SetString(
				PyExc_TypeError,
				"value must be a numeric type"
			);

			Py_DECREF(fast_sequence);
			return -1;
		}

		arr[1] = PyFloat_AsDouble(items[1]);
		if (arr[1] == -1.0 && PyErr_Occurred())
		{
			PyErr_SetString(
				PyExc_TypeError,
				"value must be a numeric type"
			);

			Py_DECREF(fast_sequence);
			return -1;
		}

		arr[2] = PyFloat_AsDouble(items[2]);
		if (arr[2] == -1.0 && PyErr_Occurred())
		{
			PyErr_SetString(
				PyExc_TypeError,
				"value must be a numeric type"
			);

			Py_DECREF(fast_sequence);
			return -1;
		}

		return 0;
	}
	else {
		PyErr_Format(
			PyExc_ValueError,
			"sequence must have exactly 3 elements, not %i",
			size
		);
		return -1;
	}
}

#define ULP_MAXIMUM	10 // this is a guess, 1 seems too stringent 

static int
__double_almost_eq(double a, double b)
{
	// check for really close values near zero
	if (fabs(a - b) < DBL_EPSILON)
		return 1;

	// signs dont match 
	if ((a < 0) != (b < 0))
		return 0;

	int64_t aInt = *(int64_t*)&a;	// bit magic
	int64_t bInt = *(int64_t*)&b;
	if (llabs(aInt - bInt) <= ULP_MAXIMUM)
		return 1;
	return 0;
}

static void
release_buffer(PyObject* obj, Py_buffer* view)
{
	if (view->internal != NULL) {
		if (*((int*)view->internal) & BUFFER_RELEASE_SHAPE)
			free(view->shape);
		free(view->internal);
	}
}

#endif // __EVSPACE_COMMON_H