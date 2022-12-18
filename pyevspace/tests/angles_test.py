from pyevspace import Angles, Order
import unittest
import pickle


class TestAngles(unittest.TestCase):
    angs = Angles(1, 2, 3)
    o = Order(0, 1, 2)

    def angleEquality(self, ang1, ang2):
        return (ang1[0] == ang2[0] and
                ang1[1] == ang2[1] and
                ang1[2] == ang2[2])

    def test_angles_str(self):
        # test int values in string
        msg = 'angles str() expected output'
        self.assertEqual(str(self.angs), '[1.000000, 2.000000, 3.000000]',
                         msg=msg)

        # test float values in string
        a = Angles(1.1, 2.2, 3.3)
        self.assertEqual(str(a), '[1.100000, 2.200000, 3.300000]', msg=msg)

    def test_angles_repr(self):
        # test int values in repr
        msg = 'angles repr() expected output'
        self.assertEqual(repr(self.angs), 'angles([1.000000, 2.000000, 3.000000])',
                         msg=msg)

        # test float values in repr
        a = Angles(1.1, 2.2, 3.3)
        self.assertEqual(repr(a), 'angles([1.100000, 2.200000, 3.300000])',msg=msg)

    def test_angles_pickle(self):
        # test if Angles can be pickled
        #   test passes if exception is not thrown
        b = pickle.dumps(self.angs)

        # test creating Angles from bytes
        a = pickle.loads(b)
        msg = 'angles pickle'
        self.assertTrue(self.angleEquality(a, Angles(1, 2, 3)), msg=msg)

    def test_angle_get(self):
        # test __getitem__
        msg = 'angles getitem'
        self.assertEqual(self.angs[0], 1, msg=msg)
        self.assertEqual(self.angs[1], 2, msg=msg)
        self.assertEqual(self.angs[2], 3, msg=msg)

        # test exceptions of index ranges
        msg = 'angles getitem index IndexError'
        with self.assertRaises(IndexError, msg=msg):
            ans = self.angs[3]

        with self.assertRaises(IndexError, msg=msg):
            ans = self.angs[-4]

        # test exception of index types
        msg = 'angles getitem index TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.angs['a']

    def test_angle_set(self):
        # test __setitem__
        a = Angles(1, 2, 3)
        a[0] = 1.1
        msg = 'angles setitem'
        self.assertTrue(self.angleEquality(a, Angles(1.1, 2, 3)), msg=msg)

        # test exceptions of index ranges
        msg = 'angles setitem index IndexError'
        with self.assertRaises(IndexError, msg=msg):
            self.angs[3] = 0

        # test exceptions of index types
        msg = 'angles setitem index TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.angs['a']

    def test_order_str(self):
        # test str with each axis in each index
        o = Order(0, 1, 2)
        msg = 'order str() expected output'
        self.assertEqual(str(o), '[X_Axis, Y_Axis, Z_Axis]', msg=msg)
        o = Order(1, 2, 0)
        self.assertEqual(str(o), '[Y_Axis, Z_Axis, X_Axis]', msg=msg)
        o = Order(2, 0, 1)
        self.assertEqual(str(o), '[Z_Axis, X_Axis, Y_Axis]', msg=msg)

    def test_order_repr(self):
        # test repr with each axis in each index
        msg = 'order repr() expected output'
        o = Order(0, 1, 2)
        self.assertEqual(repr(o), 'order([X_Axis, Y_Axis, Z_Axis])', msg=msg)
        o = Order(1, 2, 0)
        self.assertEqual(repr(o), 'order([Y_Axis, Z_Axis, X_Axis])', msg=msg)
        o = Order(2, 0, 1)
        self.assertEqual(repr(o), 'order([Z_Axis, X_Axis, Y_Axis])', msg=msg)

    def test_order_get(self):
        # test __getitem__
        msg = 'order getitem'
        self.assertEqual(self.o[0], 0, msg=msg)
        self.assertEqual(self.o[1], 1, msg=msg)
        self.assertEqual(self.o[2], 2, msg=msg)

        # test exceptions of index range
        msg = 'order getitem index IndexError'
        with self.assertRaises(IndexError, msg=msg):
            ans = self.o[3]

        # test exception of index type
        msg = 'order getitem index TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.o['a']

    def test_order_set(self):
        # test __setitem__
        o = Order(0, 1, 2)
        o[0] = 1
        msg = 'order setitem'
        self.assertEqual(o[0], 1, msg=msg)

        # test exception of index range
        msg = 'order setitem index IndexError'
        with self.assertRaises(IndexError, msg=msg):
            o[3] = 0

        # test exception of index type
        msg = 'order setitem index TypeError'
        with self.assertRaises(TypeError, msg=msg):
            o['a'] = 0

    def test_order_pickle(self):
        # test if Order can be pickled
        #   test passes if exception not raises
        b = pickle.dumps(self.o)

        # test building Order from bytes
        o = pickle.loads(b)
        msg = 'order pickle'
        self.assertTrue(o[0] == 0 and o[1] == 1 and o[2] == 2, msg=msg)


if __name__ == '__main__':
    unittest.main()
