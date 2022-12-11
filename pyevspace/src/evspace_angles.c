

//#ifdef __EVSPACE_SOURCE_INCLUDE__

#ifndef EVSPACE_ANGLES_C
#define EVSPACE_ANGLES_C

#include <evspace_angles.h>

// check if we ended up using this
static double
modulate_angle(double angle)
{
	if (angle == PI)
		return 0;
	else if (angle > PI)
		return fmod(angle, PI);
	else {
		return fmod(angle, PI) + PI;
	}
}

//static PyObject*
//new_angles_ex(double alpha, double beta, double gamma, PyTypeObject* type)











//static PyObject*
//new_order_ex(EVSpace_Axis first, EVSpace_Axis second, EVSpace_Axis third, 
//	PyTypeObject* type)






#endif // EVSPACE_ANGLES_C
//#endif // __EVSPACE_SOURCE_INCLUDE__
