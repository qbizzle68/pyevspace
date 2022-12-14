from re import I
from typing import Type
from pyevspace import *
import unittest
import pickle


class Test_angles(unittest.TestCase):
    angs = Angles(1, 2, 3)
    o = Order(0, 1, 2)

    def angleEquality(self, ang1, ang2):
        return (ang1[0] == ang2[0] and
                ang1[1] == ang2[1] and
                ang1[2] == ang2[2])

    def test_angles_str(self):
        # test int values in string
        self.assertEqual(str(self.angs), '[1.000000, 2.000000, 3.000000]')

        # test float values in string
        a = Angles(1.1, 2.2, 3.3)
        self.assertEqual(str(a), '[1.100000, 2.200000, 3.300000]')

    def test_angles_repr(self):
        # test int values in repr
        self.assertEqual(repr(self.angs), 'angles([1.000000, 2.000000, 3.000000])')

        # test float values in repr
        a = Angles(1.1, 2.2, 3.3)
        self.assertEqual(repr(a), 'angles([1.100000, 2.200000, 3.300000])')

    def test_angles_pickle(self):
        # test if Angles can be pickled
        #   test passes if exception is not thrown
        b = pickle.dumps(self.angs)

        # test creating Angles from bytes
        a = pickle.loads(b)
        self.assertTrue(self.angleEquality(a, Angles(1, 2, 3)))

    def test_angle_get(self):
        # test __getitem__
        self.assertEqual(self.angs[0], 1)
        self.assertEqual(self.angs[1], 2)
        self.assertEqual(self.angs[2], 3)

        # test exceptions of index ranges
        with self.assertRaises(IndexError) as cm:
            self.angs[3]
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            self.angs[-4]
        self.assertEqual(IndexError, type(cm.exception))

        # test exception of index types
        with self.assertRaises(TypeError) as cm:
            self.angs['a']
        self.assertEqual(TypeError, type(cm.exception))

    def test_angle_set(self):
        # test __setitem__
        a = Angles(1, 2, 3)
        a[0] = 1.1
        self.assertTrue(self.angleEquality(a, Angles(1.1, 2, 3)))

        # test exceptions of index ranges
        with self.assertRaises(IndexError) as cm:
            self.angs[3] = 0
        self.assertEqual(IndexError, type(cm.exception))

        # test exceptions of index types
        with self.assertRaises(TypeError) as cm:
            self.angs['a']
        self.assertEqual(TypeError, type(cm.exception))

    def test_order_str(self):
        # test str with each axis in each index
        o = Order(0, 1, 2)
        self.assertEqual(str(o), '[X_Axis, Y_Axis, Z_Axis]')
        o = Order(1, 2, 0)
        self.assertEqual(str(o), '[Y_Axis, Z_Axis, X_Axis]')
        o = Order(2, 0, 1)
        self.assertEqual(str(o), '[Z_Axis, X_Axis, Y_Axis]')

    def test_order_repr(self):
        # test repr with each axis in each index
        o = Order(0, 1, 2)
        self.assertEqual(repr(o), 'order([X_Axis, Y_Axis, Z_Axis])')
        o = Order(1, 2, 0)
        self.assertEqual(repr(o), 'order([Y_Axis, Z_Axis, X_Axis])')
        o = Order(2, 0, 1)
        self.assertEqual(repr(o), 'order([Z_Axis, X_Axis, Y_Axis])')

    def test_order_get(self):
        # test __getitem__
        self.assertEqual(self.o[0], 0)
        self.assertEqual(self.o[1], 1)
        self.assertEqual(self.o[2], 2)

        # test exceptions of index range
        with self.assertRaises(IndexError) as cm:
            self.o[3]
        self.assertEqual(IndexError, type(cm.exception))

        # test exception of index type
        with self.assertRaises(TypeError) as cm:
            self.o['a']
        self.assertEqual(TypeError, type(cm.exception))

    def test_order_set(self):
        # test __setitem__
        o = Order(0, 1, 2)
        o[0] = 1
        self.assertEqual(o[0], 1)

        # test exception of index range
        with self.assertRaises(IndexError) as cm:
            o[3] = 0
        self.assertEqual(IndexError, type(cm.exception))

        # test exception of index type
        with self.assertRaises(TypeError) as cm:
            o['a'] = 0
        self.assertEqual(TypeError, type(cm.exception))

    def test_order_pickle(self):
        # test if Order can be pickled
        #   test passes if exception not raises
        b = pickle.dumps(self.o)

        # test building Order from bytes
        o = pickle.loads(b)
        self.assertTrue(o[0] == 0 and o[1] == 1 and o[2] == 2)


if __name__ == '__main__':
    unittest.main()
