import unittest

from pyevspace import *
from math import pi
from .rotation_answers import *

eps = 1e-6

class Test_rotation(unittest.TestCase):

    angles = [i * pi / 4 for i in range(8)]
    axes = [X_AXIS, Y_AXIS, Z_AXIS]
    orders = [XYZ, XZY, YXZ, YZX, ZXY, ZYX, XYX, XZX, YXY, YZY, ZXZ, XYX]
    vectors = [Vector.e1, Vector.e2, Vector.e3]

    def matrix_equal(self, lhs, rhs, epsilon=eps):
        for row in range(3):
            for col in range(3):
                if abs(lhs[row, col] - rhs[row, col]) > epsilon:
                    return False
        return True

    def test_rotation_matrix(self):
        # test rotation matrices for each axis and each quadrant
        for axis in self.axes:
            for ang, ans in zip(self.angles, rotation_matrix_answers[axis]):
                mat = getMatrixAxis(axis, ang)
                msg = f'rotation from axis: {axis}, angle: {ang}'
                self.assertTrue(self.matrix_equal(mat, ans), msg)

        # test invalid axis range
        #   axis > max axis value
        with self.assertRaises(ValueError, msg='getMatrixAxis axis arg ValueError') as cm:
            getMatrixAxis(3, 0)
        self.assertEqual(ValueError, type(cm.exception))
        #   negative axis value
        with self.assertRaises(ValueError, msg='getMatrixAxis axis arg ValueError') as cm:
            getMatrixAxis(-1, 0)
        self.assertEqual(ValueError, type(cm.exception))

        # test invalid argument types
        #   invalid type for angle argument
        with self.assertRaises(TypeError, msg='getMatrixAxis angle arg TypeError') as cm:
            getMatrixAxis(X_AXIS, 'a')
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid type for axis argument
        with self.assertRaises(TypeError, msg='getMatrixAxis axis arg TypeError') as cm:
            getMatrixAxis('a', 0)
        self.assertEqual(TypeError, type(cm.exception))

    def test_euler_matrix(self):
        # test euler rotation for each euler order and eqch quadrant
        for order in self.orders:
            for ang, ans in zip(self.angles, rotation_euler_answers[order]):
                angs = Angles(ang, ang, ang)
                mat = getMatrixEuler(order, angs)
                msg = f'rotation from order: {order}, angles: {angs}'
                self.assertTrue(self.matrix_equal(mat, ans), msg)

        # test invalid argument types
        #   invalid type for order argument
        with self.assertRaises(TypeError, msg='getMatrixEuler order arg TypeError') as cm:
            getMatrixEuler(3, Angles(1, 2, 3))
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid type for angles argument
        with self.assertRaises(TypeError, msg='getMatrixEuler angles arg TypeError') as cm:
            getMatrixEuler(XYZ, 7)
        self.assertEqual(TypeError, type(cm.exception))

    def vector_equal(self, lhs, rhs, epsilon=eps):
        for l, r in zip(lhs, rhs):
            if abs(l - r) > epsilon:
                return False
        return True

    def test_axis_to(self):
        # test rotating axis to by checking where basis vector map to in 90 degree rotations
        for axisRotate, axisDict in rotation_axis_to_answers.items():
            for axis in self.axes:
                vec = rotateAxisTo(axisRotate, pi/2, self.vectors[axis])
                msg = f'rotate axis to, axis of rotation: {axisRotate}, axis tested: {axis}'
                ans = axisDict[axis]
                self.assertTrue(self.vector_equal(vec, ans), msg)

        # test exception valid axis range
        #   axis larger than max value
        with self.assertRaises(ValueError) as cm:
            rotateAxisTo(3, 0, Vector())
        self.assertEqual(ValueError, type(cm.exception))
        #   negative axis value
        with self.assertRaises(ValueError) as cm:
            rotateAxisTo(-1, 0, Vector())
        self.assertEqual(ValueError, type(cm.exception))

        # test type exceptions
        #   invalid axis type
        with self.assertRaises(TypeError) as cm:
            rotateAxisTo('a', 0, Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid angle type
        with self.assertRaises(TypeError) as cm:
            rotateAxisTo(X_AXIS, 'a', Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid vector type
        with self.assertRaises(TypeError) as cm:
            rotateAxisTo(X_AXIS, 0, 1)
        self.assertEqual(TypeError, type(cm.exception))


    def test_axis_from(self):
        # test rotating axis from by checking where basis vectors map to in 90 degree rotations
        for axisRotate, axisDict in rotation_axis_from_answers.items():
            for axis in self.axes:
                vec = rotateAxisFrom(axisRotate, pi/2, self.vectors[axis])
                msg = f'rotate axis from, axis of rotation: {axisRotate}, axis tested: {axis}'
                ans = axisDict[axis]
                self.assertTrue(self.vector_equal(vec, ans), msg)

        # test exception valid axis range
        #   axis larger than max value
        with self.assertRaises(ValueError) as cm:
            rotateAxisFrom(3, 0, Vector())
        self.assertEqual(ValueError, type(cm.exception))
        #   negative axis value
        with self.assertRaises(ValueError) as cm:
            rotateAxisFrom(-1, 0, Vector())
        self.assertEqual(ValueError, type(cm.exception))

        # test type exceptions
        #   invalid axis type
        with self.assertRaises(TypeError) as cm:
            rotateAxisFrom('a', 0, Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid angle type
        with self.assertRaises(TypeError) as cm:
            rotateAxisFrom(X_AXIS, 'a', Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid vector type
        with self.assertRaises(TypeError) as cm:
            rotateAxisFrom(X_AXIS, 0, 1)
        self.assertEqual(TypeError, type(cm.exception))

    def test_euler_to(self):
        # test euler rotate by checking where basis vectors map to in 90 degree rotations
        angs90 = Angles(pi/2, pi/2, pi/2)
        for order, orderDict in rotation_euler_to_answers.items():
            for axis in self.axes:
                vec = rotateEulerTo(order, angs90, self.vectors[axis])
                msg = f'rotate euler to, order: {order}, axis tested: {axis}'
                ans = orderDict[axis]
                self.assertTrue(self.vector_equal(vec, ans), msg)

        # test type exceptions
        #   invalid order type
        with self.assertRaises(TypeError) as cm:
            rotateEulerTo('a', angs90, Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid angles type
        with self.assertRaises(TypeError) as cm:
            rotateEulerTo(XYZ, 'a', Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid vector type
        with self.assertRaises(TypeError) as cm:
            rotateEulerTo(XYZ, angs90, 1)
        self.assertEqual(TypeError, type(cm.exception))

    def test_euler_from(self):
        # test euler rotate by checking where basis vectors map to in 90 degree rotations
        angs90 = Angles(pi/2, pi/2, pi/2)
        for order, orderDict in rotation_euler_from_answers.items():
            for axis in self.axes:
                vec = rotateEulerFrom(order, angs90, self.vectors[axis])
                msg = f'rotate euler from, order: {order}, axis tested: {axis}'
                ans = orderDict[axis]
                self.assertTrue(self.vector_equal(vec, ans), msg)

        # test type exceptions
        #   invalid order type
        with self.assertRaises(TypeError) as cm:
            rotateEulerFrom('a', angs90, Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid angles type
        with self.assertRaises(TypeError) as cm:
            rotateEulerFrom(XYZ, 'a', Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid vector type
        with self.assertRaises(TypeError) as cm:
            rotateEulerFrom(XYZ, angs90, 1)
        self.assertEqual(TypeError, type(cm.exception))

    def test_matrix_to(self):
        # test matrix rotate relying on the answers from euler rotate to
        angs90 = Angles(pi/2, pi/2, pi/2)
        for order, orderDict in rotation_euler_to_answers.items():
            mat = getMatrixEuler(order, angs90)
            for axis in self.axes:
                vec = rotateMatrixTo(mat, self.vectors[axis])
                msg = f'rotate matrix to, order: {order}, axis tested: {axis}'
                ans = orderDict[axis]
                self.assertTrue(self.vector_equal(vec, ans), msg)

        # test type exceptions
        #   invalid matrix type
        with self.assertRaises(TypeError) as cm:
            rotateMatrixTo(1, Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid vector type
        with self.assertRaises(TypeError) as cm:
            rotateMatrixTo(Matrix(), 0)
        self.assertEqual(TypeError, type(cm.exception))

    def test_matrix_from(self):
        # test matrix rotate relying on the answer from euler rotate from
        angs90 = Angles(pi/2, pi/2, pi/2)
        for order, orderDict in rotation_euler_from_answers.items():
            mat = getMatrixEuler(order, angs90)
            for axis in self.axes:
                vec = rotateMatrixFrom(mat, self.vectors[axis])
                msg = f'rotate matrix from, order: {order}, axis tested: {axis}'
                ans = orderDict[axis]
                self.assertTrue(self.vector_equal(vec, ans), msg)

        # test type exceptions
        #   invalid matrix type
        with self.assertRaises(TypeError) as cm:
            rotateMatrixFrom(1, Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid vector type
        with self.assertRaises(TypeError) as cm:
            rotateMatrixFrom(Matrix(), 0)
        self.assertEqual(TypeError, type(cm.exception))

    def test_offset_to(self):
        # test rotating to offset reference frame by mapping basis vectors
        angs90 = Angles(pi/2, pi/2, pi/2)
        offset = Vector((1, 1, 1))
        for order, orderDict in rotation_offset_to_answers.items():
            mat = getMatrixEuler(order, angs90)
            for axis in self.axes:
                vec = rotateOffsetTo(mat, offset, self.vectors[axis])
                msg = f'rotate offset to, order: {order}, axis tested: {axis}'
                ans = orderDict[axis]
                self.assertTrue(self.vector_equal(vec, ans), msg)

        # test type exceptions
        #   invalid matrix type
        with self.assertRaises(TypeError) as cm:
            rotateOffsetTo(1, Vector(), Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid offset vector type
        with self.assertRaises(TypeError) as cm:
            rotateOffsetTo(Matrix(), 1, Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalif vector type
        with self.assertRaises(TypeError) as cm:
            rotateOffsetTo(Matrix(), Vector(), 0)
        self.assertEqual(TypeError, type(cm.exception))

    def test_offset_from(self):
        # test rotating from offset reference frame by mapping basis vectors
        angs90 = Angles(pi / 2, pi / 2, pi / 2)
        offset = Vector((1, 1, 1))
        for order, orderDict in rotation_offset_from_answers.items():
            mat = getMatrixEuler(order, angs90)
            for axis in self.axes:
                vec = rotateOffsetFrom(mat, offset, self.vectors[axis])
                msg = f'rotate offset to, order: {order}, axis tested: {axis}'
                ans = orderDict[axis]
                self.assertTrue(self.vector_equal(vec, ans), msg)

        # test type exceptions
        #   invalid matrix type
        with self.assertRaises(TypeError) as cm:
            rotateOffsetFrom(1, Vector(), Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid offset vector type
        with self.assertRaises(TypeError) as cm:
            rotateOffsetFrom(Matrix(), 1, Vector())
        self.assertEqual(TypeError, type(cm.exception))
        #   invalid vector type
        with self.assertRaises(TypeError) as cm:
            rotateOffsetFrom(Matrix(), Vector(), 0)
        self.assertEqual(TypeError, type(cm.exception))

if __name__ == '__main__':
    unittest.main()