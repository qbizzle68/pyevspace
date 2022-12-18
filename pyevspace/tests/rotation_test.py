import unittest

from pyevspace import X_AXIS, Y_AXIS, Z_AXIS, XYZ, XZY, YXZ, YZX, ZXY, ZYX, XYX, XZX, YZY, YXY, ZXZ, ZYZ, \
    Vector, Matrix, Angles, Order, getMatrixAxis, getMatrixEuler, getMatrixFromTo, rotateAxisTo, rotateAxisFrom, \
    rotateEulerFrom, rotateEulerTo, rotateMatrixFrom, rotateMatrixTo, rotateOffsetTo, rotateOffsetFrom
from math import pi
from .rotation_answers import *

eps = 1e-6


class TestRotation(unittest.TestCase):

    angles = [i * pi / 4 for i in range(8)]
    angs90 = Angles(pi / 2, pi / 2, pi / 2)
    axes = [X_AXIS, Y_AXIS, Z_AXIS]
    orders = [XYZ, XZY, YXZ, YZX, ZXY, ZYX, XYX, XZX, YXY, YZY, ZXZ, XYX]
    vectors = [Vector.e1, Vector.e2, Vector.e3]
    vector_empty = Vector()
    matrix_empty = Matrix()
    offset = Vector((1, 1, 1))

    @staticmethod
    def matrix_equal(lhs, rhs, epsilon=eps):
        for row in range(3):
            for col in range(3):
                if abs(lhs[row, col] - rhs[row, col]) > epsilon:
                    return False
        return True

    @staticmethod
    def vector_equal(lhs, rhs, epsilon=eps):
        for l, r in zip(lhs, rhs):
            if abs(l - r) > epsilon:
                return False
        return True

    def test_rotation_rotation_matrix(self):
        # sentinel test to ensure tests also fail when incorrect
        mat = getMatrixAxis(Y_AXIS, pi/4)
        wrongAns = rotation_matrix_answers[X_AXIS][0]
        self.assertFalse(self.matrix_equal(mat, wrongAns), msg='getMatrixAxis fail sentinel')

        # test rotation matrices for each axis and each quadrant
        for axis in self.axes:
            for ang, ans in zip(self.angles, rotation_matrix_answers[axis]):
                mat = getMatrixAxis(axis, ang)
                msg = f'rotation from axis: {axis}, angle: {ang}'
                self.assertTrue(self.matrix_equal(mat, ans), msg=msg)

        # test invalid axis range
        #   axis > max axis value
        with self.assertRaises(ValueError, msg='getMatrixAxis axis arg ValueError'):
            getMatrixAxis(3, 0)
        #   negative axis value
        with self.assertRaises(ValueError, msg='getMatrixAxis axis arg ValueError'):
            getMatrixAxis(-1, 0)

        # test invalid argument types
        #   invalid type for angle argument
        with self.assertRaises(TypeError, msg='getMatrixAxis angle arg TypeError'):
            getMatrixAxis(X_AXIS, 'a')
        #   invalid type for axis argument
        with self.assertRaises(TypeError, msg='getMatrixAxis axis arg TypeError'):
            getMatrixAxis('a', 0)

    def test_rotation_euler_matrix(self):
        # sentinel test to ensure tests also fail when incorrect
        mat = getMatrixEuler(XYZ, self.angs90)
        wrongAns = rotation_euler_answers[XZY][2]
        self.assertFalse(self.matrix_equal(mat, wrongAns), msg='getMatrixEuler fail sentinel')

        # test euler rotation for each euler order and each quadrant
        for order in self.orders:
            for ang, ans in zip(self.angles, rotation_euler_answers[order]):
                angs = Angles(ang, ang, ang)
                mat = getMatrixEuler(order, angs)
                msg = f'rotation from order: {order}, angles: {angs}'
                self.assertTrue(self.matrix_equal(mat, ans), msg=msg)

        # test invalid argument types
        #   invalid type for order argument
        with self.assertRaises(TypeError, msg='getMatrixEuler order arg TypeError'):
            getMatrixEuler(3, Angles(1, 2, 3))
        #   invalid type for angles argument
        with self.assertRaises(TypeError, msg='getMatrixEuler angles arg TypeError'):
            getMatrixEuler(XYZ, 7)

    @staticmethod
    def order_str_to_label(order):
        orderStr = str(order)
        return f'{orderStr[1]}{orderStr[9]}{orderStr[17]}'

    def test_rotation_from_to_matrix(self):
        # sentinel test to ensure tests also fail when incorrect
        mat = getMatrixFromTo(XYZ, self.angs90, YXZ, self.angs90)
        wrongAns = rotation_from_to_answers[XYZ][YZX]
        self.assertFalse(self.matrix_equal(mat, wrongAns), msg='getMatrixFromTo fail sentinel')

        # test from each frame to each frame
        for orderFrom in self.orders:
            fromLabel = self.order_str_to_label(orderFrom)
            for orderTo in self.orders:
                ans = rotation_from_to_answers[orderFrom][orderTo]
                mat = getMatrixFromTo(orderFrom, self.angs90, orderTo, self.angs90)
                toLabel = self.order_str_to_label(orderTo)
                msg = f'rotation matrix from to: from order: {fromLabel}, to order: {toLabel}'
                self.assertTrue(self.matrix_equal(mat, ans), msg=msg)

        # test invalid argument types
        #   invalid type for orderFrom argument
        with self.assertRaises(TypeError, msg='getMatrixFromTo orderFrom arg TypeError'):
            getMatrixFromTo(0, self.angs90, XYZ, self.angs90)
        #   invalid type for anglesFrom argument
        with self.assertRaises(TypeError, msg='getMatrixFromTo anglesFrom arg TypeError'):
            getMatrixFromTo(XYZ, 0, XYZ, self.angs90)
        #   invalid type for orderTo argument
        with self.assertRaises(TypeError, msg='getMatrixFromTo orderTo arg TypeError'):
            getMatrixFromTo(XYZ, self.angs90, 0, self.angs90)
        #   invalid type for anglesTo argument
        with self.assertRaises(TypeError, msg='getMatrixFromTo anglesTo arg TypeError'):
            getMatrixFromTo(XYZ, self.angs90, XYZ, 0)

    def test_rotation_axis_to(self):
        # sentinel test to ensure tests also fail when incorrect
        vec = rotateAxisTo(X_AXIS, pi/2, Vector.e2)
        wrongAns = rotation_axis_to_answers[X_AXIS][Z_AXIS]
        self.assertFalse(self.vector_equal(vec, wrongAns), msg='rotateAxisTo fail sentinel')

        # test rotating axis to by checking where basis vector map to in 90 degree rotations
        for axisRotation in self.axes:
            for axisMapped in self.axes:
                ans = rotation_axis_to_answers[axisRotation][axisMapped]
                vec = rotateAxisTo(axisRotation, pi/2, self.vectors[axisMapped])
                msg = f'rotate axis to, axis of rotation: {axisRotation}, axis tested: {axisMapped}'
                self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test exception valid axis range
        #   axis larger than max value
        with self.assertRaises(ValueError, msg='rotateAxisTo axis arg IndexError'):
            rotateAxisTo(3, 0, self.vector_empty)
        #   negative axis value
        with self.assertRaises(ValueError, msg='rotateAxisTo axis arg IndexError'):
            rotateAxisTo(-1, 0, self.vector_empty)

        # test type exceptions
        #   invalid axis type
        with self.assertRaises(TypeError, msg='rotateAxisTo axis arg TypeError'):
            rotateAxisTo('a', 0, self.vector_empty)
        #   invalid angle type
        with self.assertRaises(TypeError, msg='rotateAxisTo angle arg TypeError'):
            rotateAxisTo(X_AXIS, 'a', self.vector_empty)
        #   invalid vector type
        with self.assertRaises(TypeError, msg='rotateAxisTo vector arg TypeError'):
            rotateAxisTo(X_AXIS, 0, 1)

    def test_rotation_axis_from(self):
        # sentinel test to ensure tests also fail when incorrect
        vec = rotateAxisFrom(X_AXIS, pi/2, Vector.e2)
        wrongAns = rotation_axis_from_answers[X_AXIS][Z_AXIS]
        self.assertFalse(self.vector_equal(vec, wrongAns), msg='rotateAxisFrom fail sentinel')

        # test rotating axis from by checking where basis vectors map to in 90 degree rotations
        for axisRotation in self.axes:
            for axisMapped in self.axes:
                ans = rotation_axis_from_answers[axisRotation][axisMapped]
                vec = rotateAxisFrom(axisRotation, pi/2, self.vectors[axisMapped])
                msg = f'rotate axis from, axis of rotation: {axisRotation}, axis tested: {axisMapped}'
                self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test exception valid axis range
        #   axis larger than max value
        with self.assertRaises(ValueError, msg='rotateAxisFrom axis arg IndexError'):
            rotateAxisFrom(3, 0, self.vector_empty)
        #   negative axis value
        with self.assertRaises(ValueError, msg='rotateAxisFrom axis arg IndexError'):
            rotateAxisFrom(-1, 0, self.vector_empty)

        # test type exceptions
        #   invalid axis type
        with self.assertRaises(TypeError, msg='rotateAxisFrom axis arg TypeError'):
            rotateAxisFrom('a', 0, self.vector_empty)
        #   invalid angle type
        with self.assertRaises(TypeError, msg='rotateAxisFrom angle arg TypeError'):
            rotateAxisFrom(X_AXIS, 'a', self.vector_empty)
        #   invalid vector type
        with self.assertRaises(TypeError, msg='rotateAxisFrom vector arg TypeError'):
            rotateAxisFrom(X_AXIS, 0, 1)

    def test_rotation_euler_to(self):
        # sentinel test to ensure tests also fail when incorrect
        vec = rotateEulerTo(XYZ, self.angs90, Vector.e1)
        wrongAns = rotation_euler_to_answers[XYZ][Y_AXIS]
        self.assertFalse(self.vector_equal(vec, wrongAns), msg='rotateEulerTo fail sentinel')

        # test euler rotate by checking where basis vectors map to in 90 degree rotations
        for order in self.orders:
            for axis in self.axes:
                vec = rotateEulerTo(order, self.angs90, self.vectors[axis])
                msg = f'rotate euler to, order: {order}, axis tested: {axis}'
                ans = rotation_euler_to_answers[order][axis]
                self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test type exceptions
        #   invalid order type
        with self.assertRaises(TypeError, msg='rotateEulerTo order arg TypeError'):
            rotateEulerTo('a', self.angs90, self.vector_empty)
        #   invalid angles type
        with self.assertRaises(TypeError, msg='rotateEulerTo angles arg TypeError'):
            rotateEulerTo(XYZ, 'a', self.vector_empty)
        #   invalid vector type
        with self.assertRaises(TypeError, msg='rotateEulerTo vector arg TypeError'):
            rotateEulerTo(XYZ, self.angs90, 1)

    def test_rotation_euler_from(self):
        # sentinel test to ensure tests also fail when incorrect
        vec = rotateEulerFrom(XYZ, self.angs90, Vector.e1)
        wrongAns = rotation_euler_from_answers[XYZ][Y_AXIS]
        self.assertFalse(self.vector_equal(vec, wrongAns), msg='rotateEulerFrom fail sentinel')

        # test euler rotate by checking where basis vectors map to in 90 degree rotations
        for order in self.orders:
            for axis in self.axes:
                vec = rotateEulerFrom(order, self.angs90, self.vectors[axis])
                msg = f'rotate euler to, order: {order}, axis tested: {axis}'
                ans = rotation_euler_from_answers[order][axis]
                self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test type exceptions
        #   invalid order type
        with self.assertRaises(TypeError, msg='rotateEulerFrom order arg TypeError'):
            rotateEulerFrom('a', self.angs90, self.vector_empty)
        #   invalid angles type
        with self.assertRaises(TypeError, msg='rotateEulerFrom angles arg TypeError'):
            rotateEulerFrom(XYZ, 'a', self.vector_empty)
        #   invalid vector type
        with self.assertRaises(TypeError, msg='rotateEulerFrom vector arg TypeError'):
            rotateEulerFrom(XYZ, self.angs90, 1)

    def test_rotation_matrix_to(self):
        # sentinel test to ensure tests also fail when incorrect
        mat = getMatrixEuler(XYZ, self.angs90)
        vec = rotateMatrixTo(mat, Vector.e1)
        wrongAns = rotation_euler_to_answers[XYZ][Y_AXIS]
        self.assertFalse(self.vector_equal(vec, wrongAns), msg='rotateMatrixTo fail sentinel')

        # test matrix rotate relying on the answers from euler rotate to
        for order in self.orders:
            mat = getMatrixEuler(order, self.angs90)
            for axis in self.axes:
                vec = rotateMatrixTo(mat, self.vectors[axis])
                msg = f'rotate matrix to, order: {order}, axis tested: {axis}'
                ans = rotation_euler_to_answers[order][axis]
                self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test type exceptions
        #   invalid matrix type
        with self.assertRaises(TypeError, msg='rotateMatrixTo matrix arg TypeError'):
            rotateMatrixTo(1, self.vector_empty)
        #   invalid vector type
        with self.assertRaises(TypeError, msg='rotateMatrixTo vector arg TypeError'):
            rotateMatrixTo(self.matrix_empty, 0)

    def test_rotation_matrix_from(self):
        # sentinel test to ensure tests also fail when incorrect
        mat = getMatrixEuler(XYZ, self.angs90)
        vec = rotateMatrixFrom(mat, Vector.e1)
        wrongAns = rotation_euler_from_answers[XYZ][Y_AXIS]
        self.assertFalse(self.vector_equal(vec, wrongAns), msg='rotateMatrixFrom fail sentinel')

        # test matrix rotate relying on the answer from euler rotate from
        for order in self.orders:
            mat = getMatrixEuler(order, self.angs90)
            for axis in self.axes:
                vec = rotateMatrixFrom(mat, self.vectors[axis])
                msg = f'rotate matrix from, order: {order}, axis tested: {axis}'
                ans = rotation_euler_from_answers[order][axis]
                self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test type exceptions
        #   invalid matrix type
        with self.assertRaises(TypeError, msg='rotateMatrixFrom matrix arg TypeError'):
            rotateMatrixFrom(1, self.vector_empty)
        #   invalid vector type
        with self.assertRaises(TypeError, msg='rotateMatrixFrom vector arg TypeError'):
            rotateMatrixFrom(self.matrix_empty, 0)

    def test_rotation_offset_to(self):
        # sentinel test to ensure tests also fail when incorrect
        mat = getMatrixEuler(XYZ, self.angs90)
        vec = rotateOffsetTo(mat, self.offset, Vector.e1)
        wrongAns = rotation_offset_to_answers[XYZ][Y_AXIS]
        self.assertFalse(self.vector_equal(vec, wrongAns), msg='rotateOffsetTo fail sentinel')

        # test rotating to offset reference frame by mapping basis vectors
        for order in self.orders:
            mat = getMatrixEuler(order, self.angs90)
            for axis in self.axes:
                vec = rotateOffsetTo(mat, self.offset, self.vectors[axis])
                msg = f'rotate offset to, order: {order}, axis tested: {axis}'
                ans = rotation_offset_to_answers[order][axis]
                self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test type exceptions
        #   invalid matrix type
        with self.assertRaises(TypeError, msg='rotateOffsetTo matrix arg TypeError'):
            rotateOffsetTo(1, Vector(), Vector())
        #   invalid offset vector type
        with self.assertRaises(TypeError, msg='rotateOffsetTo offset vector arg TypeError'):
            rotateOffsetTo(Matrix(), 1, Vector())
        #   invalid vector type
        with self.assertRaises(TypeError, msg='rotateOffsetTo vector arg TypeError'):
            rotateOffsetTo(Matrix(), Vector(), 0)

    def test_rotation_offset_from(self):
        # sentinel test to ensure tests also fail when incorrect
        mat = getMatrixEuler(XYZ, self.angs90)
        vec = rotateOffsetFrom(mat, self.offset, Vector.e1)
        wrongAns = rotation_offset_to_answers[XYZ][Y_AXIS]
        self.assertFalse(self.vector_equal(vec, wrongAns), msg='rotateOffsetFrom fail sentinel')

        # test rotating from offset reference frame by mapping basis vectors
        for order in self.orders:
            mat = getMatrixEuler(order, self.angs90)
            for axis in self.axes:
                vec = rotateOffsetFrom(mat, self.offset, self.vectors[axis])
                msg = f'rotate offset to, order: {order}, axis tested: {axis}'
                ans = rotation_offset_from_answers[order][axis]
                self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test type exceptions
        #   invalid matrix type
        with self.assertRaises(TypeError, msg='rotateOffsetFrom matrix arg TypeError'):
            rotateOffsetFrom(1, Vector(), Vector())
        #   invalid offset vector type
        with self.assertRaises(TypeError, msg='rotateOffsetFrom offset vector arg TypeError'):
            rotateOffsetFrom(Matrix(), 1, Vector())
        #   invalid vector type
        with self.assertRaises(TypeError, msg='rotateOffsetFrom vector arg TypeError'):
            rotateOffsetFrom(Matrix(), Vector(), 0)


if __name__ == '__main__':
    unittest.main()
