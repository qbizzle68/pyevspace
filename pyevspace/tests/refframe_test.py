import sys
import unittest

from pyevspace import ReferenceFrame, XYZ, Angles, Vector, Matrix
from .rotation_answers import *
from math import pi

eps = 1e-6


class TestReferenceFrame(unittest.TestCase):

    ref = ReferenceFrame(XYZ, Angles(1, 2, 3))
    ref_offset = ReferenceFrame(XYZ, Angles(1, 2, 3), offset=Vector((1, 1, 1)))
    angs90 = Angles(pi/2, pi/2, pi/2)
    offset = Vector((1, 1, 1))
    vectors = [Vector.e1, Vector.e2, Vector.e3]

    @staticmethod
    def angles_equal(lhs, rhs):
        return lhs[0] == rhs[0] and lhs[1] == rhs[1] and lhs[2] == rhs[2]

    @staticmethod
    def matrix_equal(lhs, rhs, epsilon=eps):
        for row in range(3):
            for col in range(3):
                if abs(lhs[row, col] - rhs[row, col]) > epsilon:
                    return False
        return True

    def test_refframe_new(self):
        # test the refframe constructor
        #   test passes by not raising exception
        ref = ReferenceFrame(XYZ, Angles())
        angsArg = Angles(1, 2, 3)
        offsetArg = Vector(1, 1, 1)
        ref_offset = ReferenceFrame(XYZ, angsArg, offset=offsetArg)

        # test 'resource capturing' of attributes
        offset_vector = Vector((1, 1, 1))
        offRefCount0 = sys.getrefcount(offset_vector)
        orderRefCount0 = sys.getrefcount(XYZ)
        angleRefCount0 = sys.getrefcount(self.angs90)
        ref = ReferenceFrame(XYZ, self.angs90, offset=offset_vector)
        msg = 'refframe offset reference count increase'
        self.assertTrue(sys.getrefcount(offset_vector) > offRefCount0, msg=msg)
        msg = 'refframe offset same instance'
        self.assertIs(ref_offset.offset, offsetArg)

        msg = 'refframe order reference count increase'
        self.assertTrue(sys.getrefcount(XYZ) > orderRefCount0, msg=msg)
        msg = 'refframe order same instance'
        self.assertIs(ref_offset.order, XYZ)

        msg = 'refframe angles reference count increase'
        self.assertTrue(sys.getrefcount(self.angs90) > angleRefCount0, msg=msg)
        msg = 'refframe angles same instance'
        self.assertIs(ref_offset.angles, angsArg)

        # test position offset argument TypeError
        msg = 'reference frame positional offset argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = ReferenceFrame(XYZ, self.angs90, self.offset)

        # test constructor type exceptions
        msg = 'reference frame constructor order argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = ReferenceFrame(1, self.angs90)

        msg = 'reference frame constructor angles argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = ReferenceFrame(XYZ, 1)

        msg = 'reference frame constructor offset vector argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = ReferenceFrame(XYZ, self.angs90, offset=1)

    def test_refframe_angles_getter(self):
        # test angles property
        msg = 'reference frame angles property expected output'
        self.assertTrue(self.angles_equal(self.ref.angles, Angles(1, 2, 3)),
                        msg=msg)
        msg = 'reference frame angles property with offset'
        self.assertTrue(self.angles_equal(self.ref_offset.angles,
                                          Angles(1, 2, 3)), msg=msg)

        # ensure reference count increases
        ref = ReferenceFrame(XYZ, self.angs90)
        angRefCount0 = sys.getrefcount(self.angs90)
        angs = ref.angles
        msg = 'refframe angles ref count increase'
        self.assertTrue(sys.getrefcount(angs) > angRefCount0, msg=msg)
        msg = 'refframe angles same instance'
        self.assertIs(angs, self.angs90, msg=msg)

    def test_refframe_angles_setter(self):
        # test angles property
        ref = ReferenceFrame(XYZ, self.angs90)
        ref.angles = Angles()
        msg = 'reference frame angles setter test matrix change'
        self.assertEqual(ref.matrix, Matrix.id, msg=msg)

        ref = ReferenceFrame(XYZ, self.angs90, offset=self.offset)
        ref.angles = Angles()
        msg = 'reference frame angles setter test matrix change with offset'
        self.assertEqual(ref.matrix, Matrix.id, msg=msg)

        # test angles property exceptions
        # test deleting angles property
        msg = 'reference frame delete angles attribute'
        with self.assertRaises(ValueError, msg=msg):
            del ref.angles

        # test angles property TypeError
        msg = 'reference frame angles set TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ref.angles = 1

    def test_refframe_subangle_getter(self):
        # test getting sub-angles
        ref = ReferenceFrame(XYZ, self.angs90)
        msg = 'reference frame getting sub-angle'
        self.assertEqual(ref.alpha, pi/2, msg=msg)
        self.assertEqual(ref.beta, pi/2, msg=msg)
        self.assertEqual(ref.gamma, pi/2, msg=msg)

    def test_refframe_subangle_setter(self):
        # test setting sub-angle
        ref = ReferenceFrame(XYZ, Angles(pi/2, pi/2, pi/2))
        ref.alpha = 0
        msg = 'reference frame sub-angle set changing matrix'
        self.assertEqual(ref.matrix, Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
                         msg=msg)

        # test sub-angle exceptions
        # test deleting sub-angle ValueError
        msg = 'reference frame deleting sub-angle TypeError'
        with self.assertRaises(ValueError, msg=msg):
            del ref.alpha

        # test sub-angle TypeError
        msg = 'reference frame sub-angle TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ref.beta = 'a'

    def test_refframe_offset_getter(self):
        # test getting offset vector
        vecOffset = self.ref.offset
        msg = 'reference frame offset vector'
        self.assertEqual(vecOffset, None, msg=msg)
        vecOffset = self.ref_offset.offset
        self.assertEqual(vecOffset, self.offset, msg=msg)

    def test_refframe_offset_setter(self):
        # test setting offset vector
        ref = ReferenceFrame(XYZ, self.angs90)
        ref.offset = Vector(1, 1, 1)

        ref = ReferenceFrame(XYZ, self.angs90, offset=Vector(1, 1, 1))
        ref.offset = Vector(1, 2, 3)
        msg = 'refframe offset setter expected outcome'
        self.assertEqual(ref.offset, Vector(1, 2, 3), msg=msg)

        # test deleting offset attribute
        del ref.offset
        msg = 'refframe offset delete attribute'
        self.assertEqual(ref.offset, None, msg=msg)

        # test offset exceptions
        msg = 'refframe offset setter TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ref.offset = 1

    @staticmethod
    def vector_equal(lhs, rhs, epsilon=eps):
        for l, r in zip(lhs, rhs):
            if abs(l - r) > epsilon:
                return False
        return True

    def test_refframe_rotate_to(self):
        # test rotating to reference frame without offset vector
        for order, orderDict in rotation_euler_to_answers.items():
            ref = ReferenceFrame(order, self.angs90)
            for axis in orderDict:
                ans = ref.rotateTo(self.vectors[axis])
                msg = f'rotate refframe to, order: {order}, axis tested: {axis}'
                self.assertTrue(self.vector_equal(ans, orderDict[axis]), msg=msg)

        # test rotating to reference frame with offset vector
        for order, orderDict in rotation_offset_to_answers.items():
            ref = ReferenceFrame(order, self.angs90, offset=self.offset)
            for axis in orderDict:
                ans = ref.rotateTo(self.vectors[axis])
                msg = f'rotate refframe to, order: {order}, offset: {ref.offset}' \
                      f', axis tested: {axis}'
                self.assertTrue(self.vector_equal(ans, orderDict[axis]), msg=msg)

        # test rotate to exceptions
        msg = 'refframe rotate to vector argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.ref.rotateTo(1)

    def test_refframe_rotate_from(self):
        # test rotating to reference frame without offset vector
        for order, orderDict in rotation_euler_from_answers.items():
            ref = ReferenceFrame(order, self.angs90)
            for axis in orderDict:
                ans = ref.rotateFrom(self.vectors[axis])
                msg = f'rotate refframe to, order: {order}, axis tested: {axis}'
                self.assertTrue(self.vector_equal(ans, orderDict[axis]), msg=msg)

        # test rotating to reference frame with offset vector
        for order, orderDict in rotation_offset_from_answers.items():
            ref = ReferenceFrame(order, self.angs90, offset=self.offset)
            for axis in orderDict:
                ans = ref.rotateFrom(self.vectors[axis])
                msg = f'rotate refframe to, order: {order}, offset: {ref.offset}' \
                      f', axis tested: {axis}'
                self.assertTrue(self.vector_equal(ans, orderDict[axis]), msg=msg)

        # test rotate from exceptions
        msg = 'refframe rotate from vector argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.ref.rotateFrom(1)

    def test_refframe_between_frames(self):
        # test rotating to another reference frame
        for orderFrom, fromDict in rotation_refframe_from_to_answers.items():
            refFrom = ReferenceFrame(orderFrom, self.angs90)
            for orderTo, toDict in fromDict.items():
                refTo = ReferenceFrame(orderTo, self.angs90)
                for axis, ans in toDict.items():
                    vec = refFrom.rotateToFrame(refTo, self.vectors[axis])
                    msg = f'rotate refframe to frame, orderFrom: {orderFrom}, orderTo: ' \
                          f'{orderTo}, axis tested: {axis}'
                    self.assertTrue(self.vector_equal(vec, ans), msg=msg)
                    vec = refTo.rotateFromFrame(refFrom, self.vectors[axis])
                    msg = f'rotate refframe from frame, orderFrom: {orderFrom}, orderTo: ' \
                          f'{orderTo}, axis tested: {axis}'
                    self.assertTrue(self.vector_equal(vec, ans), msg=msg)

        # test exceptions
        msg = 'refframe rotate to reference frame argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.ref.rotateTo(1, self.offset)

        msg = 'refframe rotate from reference frame argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.ref.rotateFrom(1, self.offset)

        msg = 'refframe rotate to vector argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.ref.rotateTo(self.ref_offset, 1)

        msg = 'refframe rotate from vector argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.ref.rotateFrom(self.ref_offset, 2)

    def test_refframe_between_frames_offset(self):
        # test rotation to another reference frame with offset
        for orderFrom, fromDict in rotation_refframe_from_to_offset_answers.items():
            refFrom = ReferenceFrame(orderFrom, self.angs90)
            for orderTo, toDict in fromDict.items():
                refTo = ReferenceFrame(orderTo, self.angs90, offset=self.offset)
                for axis, ans in toDict.items():
                    vec = refFrom.rotateToFrame(refTo, self.vectors[axis])
                    msg = f'rotate refframe to offset frame, orderFrom: ' \
                          f'{orderFrom}, orderTo: {orderTo}, axis tested: {axis}'
                    self.assertTrue(self.vector_equal(vec, ans), msg=msg)


if __name__ == '__main__':
    unittest.main()
