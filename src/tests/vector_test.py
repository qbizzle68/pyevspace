from copy import copy
from math import sqrt, pi

from pyevspace import Vector, Matrix, dot, cross, norm, vang, vxcl, proj
import unittest
import pickle


# class holding the vector tests
class TestVector(unittest.TestCase):

    v111 = Vector((1, 1, 1))
    v123 = Vector((1, 2, 3))
    v123m = Vector((-1, -2, -3))
    v34 = Vector((3, 4, 0))

    def test_vector_new(self):
        # test all vector constructors
        ans = Vector()
        ans = Vector(1, 2, 3)
        ans = Vector((1, 2, 3))

        # test iterable less than length 3
        msg = 'Vector() constructor iterable length < 3 ValueError'
        with self.assertRaises(ValueError, msg=msg):
            Vector((1, 2))

        # test iterable more than length 3
        msg = 'Vector() constructor iterable length > 3 ValueError'
        with self.assertRaises(ValueError, msg=msg):
            Vector((1, 2, 3, 4))

        # test non-numeric value in each index
        msg = 'Vector() constructor x-component non-numeric TypeError'
        with self.assertRaises(TypeError, msg=msg):
            Vector(('a', 1, 2))

        msg = 'Vector() constructor y-component non-numeric TypeError'
        with self.assertRaises(TypeError, msg=msg):
            Vector((0, 'a', 2))

        msg = 'Vector() constructor z-component non-numeric TypeError'
        with self.assertRaises(TypeError, msg=msg):
            Vector((0, 1, 'a'))

        # test empty constructor
        msg = 'Vector() empty constructor initialize to zero\'s'
        self.assertEqual(Vector(), Vector((0, 0, 0)), msg=msg)

        # test non list or tuple sequence
        d = {0: 'a', 1: 'b', 2: 'c'}
        msg = 'Vector() constructor from non list or tuple sequence'
        self.assertEqual(Vector((0, 1, 2)), Vector(d), msg=msg)

    def test_vector_str(self):
        # test int only vectors
        msg = 'vector str() check expected output with int\'s'
        self.assertEqual(str(self.v123), '[1, 2, 3]', msg=msg)

        # test float integer only vectors
        msg = 'vector str() check expected output with int float\'s'
        self.assertEqual(str(Vector((1.0, 2.0, 3.0))), '[1, 2, 3]', msg=msg)

        # test float only vectors
        msg = 'vector str() check expected output with float\'s'
        self.assertEqual(str(Vector((1.1, 2.2, 3.3))), '[1.1, 2.2, 3.3]',
                         msg=msg)

        # test border of scientific notation
        msg = 'vector str() sci-notation test'
        self.assertEqual(str(Vector((123456.1234, 1234567.891, 0.123456))),
                         '[123456, 1.23457e+06, 0.123456]', msg=msg)
        self.assertEqual(str(Vector((0.0001234567, 0.00001234567, 0.000001234567))),
                         '[0.000123457, 1.23457e-05, 1.23457e-06]', msg=msg)

    def test_vector_repr(self):
        # test int only vectors
        msg = 'vector repr() check expected out with int\'s'
        self.assertEqual(repr(self.v123), 'Vector([1, 2, 3])', msg=msg)

        # test float integer only vectors
        msg = 'vector repr() check expected output with int float\'s'
        self.assertEqual(repr(Vector((1.0, 2.0, 3.0))), 'Vector([1, 2, 3])',
                         msg=msg)

        # test float only vectors
        msg = 'vector repr() check expected output with float\'s'
        self.assertEqual(repr(Vector((1.1, 2.2, 3.3))), 'Vector([1.1, 2.2, 3.3])',
                         msg=msg)

        # test border of scientific notation
        msg = 'vector repr() sci-notation test'
        self.assertEqual(repr(Vector((123456.1234, 1234567.891, 0.123456))),
                         'Vector([123456, 1.23457e+06, 0.123456])', msg=msg)
        self.assertEqual(repr(Vector((0.0001234567, 0.00001234567, 0.000001234567))),
                         'Vector([0.000123457, 1.23457e-05, 1.23457e-06])', msg=msg)

    def test_vector_iter(self):
        # test __iter__() by checking next() values
        itr = iter(self.v123)
        n = next(itr)
        msg = 'vector iterator expected x value'
        self.assertEqual(n, 1.0, msg=msg)
        n = next(itr)
        msg = 'vector iterator expected y value'
        self.assertEqual(n, 2.0, msg=msg)
        n = next(itr)
        msg = 'vector iterator expected z value'
        self.assertEqual(n, 3.0, msg=msg)

        # test length of iteration is correct
        msg = 'vector iterator check expected StopIteration'
        with self.assertRaises(StopIteration, msg=msg):
            n = next(itr)

        # test with generation
        ls = [i for i in self.v123]
        msg = 'vector iterator compare with list'
        self.assertEqual(ls, [1.0, 2.0, 3.0], msg=msg)

        # test in operator
        msg = 'vector iterator check in operator'
        self.assertIn(1, self.v123, msg=msg)
        msg = 'vector iterator check not in operator'
        self.assertNotIn(0, self.v123, msg=msg)

    def test_vector_add(self):
        # test addition operator
        msg = 'vector add two vectors'
        self.assertEqual(self.v111 + self.v123, Vector((2, 3, 4)), msg=msg)
        self.assertEqual(self.v111 + self.v123m, Vector((0, -1, -2)), msg=msg)

        # test exceptions on types
        msg = 'vector add check rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 + 1

        msg = 'vector add rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 + 1.0

        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 + 'a'

    def test_vector_iadd(self):
        # test inplace addition
        v = Vector((1, 1, 1))
        v += self.v123
        msg = 'vector iadd two vectors'
        self.assertEqual(v, Vector((2, 3, 4)), msg=msg)
        v = Vector((-1, -2, -3))
        v += self.v123
        self.assertEqual(v, Vector((0, 0, 0)), msg=msg)

        # test exception on types
        msg = 'vector iadd rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.v111 += 1

        with self.assertRaises(TypeError, msg=msg):
            self.v111 += 1.0

        with self.assertRaises(TypeError, msg=msg):
            self.v111 += 'a'

    def test_vector_subtract(self):
        # test subtraction operator
        msg = 'vector subtract two vectors'
        self.assertEqual(self.v123 - self.v111, Vector((0, 1, 2)), msg=msg)
        self.assertEqual(self.v111 - self.v123, Vector((0, -1, -2)), msg=msg)

        # test exception on types
        msg = 'vector subtract rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 - 1

        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 - 1.0

        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 - 'a'

    def test_vector_isubtract(self):
        # test inplace subtraction operator
        v = Vector((1, 1, 1))
        v -= self.v123
        msg = 'vector isubtract two vectors'
        self.assertEqual(v, Vector((0, -1, -2)), msg=msg)
        v = Vector((1, 1, 1))
        v -= self.v123m
        self.assertEqual(v, Vector((2, 3, 4)), msg=msg)

        # test exception on types
        msg = 'vector isubtract rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.v111 -= 1

        with self.assertRaises(TypeError, msg=msg):
            self.v111 -= 1.0

        with self.assertRaises(TypeError, msg=msg):
            self.v111 -= 'a'

    def test_vector_multiply(self):
        # test scalar multiplication operator
        msg = 'vector multiply scalar'
        self.assertEqual(self.v123 * 2, Vector((2, 4, 6)), msg=msg)
        self.assertEqual(self.v123m * 0.5, Vector((-0.5, -1, -1.5)), msg=msg)

        # test vector multiplication operator
        msg = 'vector multiply matrix'
        mat = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        self.assertEqual(self.v123 @ mat, Vector(30, 36, 42), msg=msg)

        # test exception on types
        msg = 'vector multiply rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 * 'a'

        with self.assertRaises(TypeError, msg=msg):
            self.v111 * Vector()

        with self.assertRaises(TypeError, msg=msg):
            self.v123 @ 4

    def test_vector_imultiply(self):
        # test inplace scalar multiplication operator
        v = Vector((1, 2, 3))
        v *= 2
        msg = 'vector imultiply scalar'
        self.assertEqual(v, Vector((2, 4, 6)), msg=msg)
        v = Vector((-1, -1, -1))
        v *= 0.5
        self.assertEqual(v, Vector((-0.5, -0.5, -0.5)), msg=msg)

        # test exception on types
        msg = 'vector imultiply argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.v111 *= 'a'

        with self.assertRaises(TypeError, msg=msg):
            self.v111 *= Vector()

        with self.assertRaises(TypeError, msg=msg):
            self.v123 @= Matrix()

    def test_vector_divide(self):
        # test division operator
        msg = 'vector divide scalar'
        self.assertEqual(self.v123 / 1, self.v123, msg=msg)
        self.assertEqual(self.v123m / 0.1, Vector((-10, -20, -30)), msg=msg)

        # test exception on types
        msg = 'vector divide rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 / 'a'

        with self.assertRaises(TypeError, msg=msg):
            ans = self.v111 / Vector()

    def test_vector_idivide(self):
        # test inplace division operator
        v = Vector((1, 2, 3))
        v /= 2
        msg = 'vector idivide scalar'
        self.assertEqual(v, Vector((0.5, 1, 1.5)), msg=msg)
        v = Vector((-1, -1, -1))
        v /= 0.5
        self.assertEqual(v, Vector((-2, -2, -2)), msg=msg)

        # test exception on types
        msg = 'vector idivide rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.v111 /= 'a'

        with self.assertRaises(TypeError, msg=msg):
            self.v111 /= Vector()

    def test_vector_mag(self):
        # test magnitude computation is correct
        v = Vector((3, 4, 0))
        msg = 'vector magnitude class method'
        self.assertEqual(v.mag(), 5, msg=msg)
        x = sqrt(1/3)
        v = Vector((x, x, x))
        self.assertEqual(v.mag(), 1, msg=msg)

        # test exception on types
        msg = 'vector mag() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.v111.mag(1)

    def test_vector_mag2(self):
        # test mag2 computation is correct
        msg = 'vector magnitude square class method'
        self.assertEqual(self.v123.mag2(), 14, msg=msg)
        self.assertEqual(self.v123m.mag2(), 14, msg=msg)

        # test exception on types
        msg = 'vector mag2() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.v111.mag2(1)

    def test_vector_normalize(self):
        # test normalize computation
        x = sqrt(1/3)
        v = Vector((x, x, x))
        v.normalize()
        msg = 'vector normalize class method'
        self.assertEqual(v, Vector((x, x, x)), msg=msg)
        v = Vector((3, 4, 0))
        v.normalize()
        self.assertEqual(v, Vector((0.6, 0.8, 0)), msg=msg)

        # test exception on types
        msg = 'vector normalize arg TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.v111.normalize(1)

    def test_vector_sequence(self):
        # test sequence get and length operate correctly
        msg = 'vector sequence length'
        self.assertEqual(len(self.v111), 3, msg=msg)
        msg = 'vector sequence get item'
        self.assertEqual(self.v123[0], 1, msg=msg)
        self.assertEqual(self.v123[-1], 3, msg=msg)

        # test exception on index range
        msg = 'vector sequence get IndexError'
        with self.assertRaises(IndexError, msg=msg):
            x = self.v111[3]

        with self.assertRaises(IndexError, msg=msg):
            x = self.v111[-4]

        # test exception on index type
        msg = 'vector sequence get TypeError'
        with self.assertRaises(TypeError, msg=msg):
            x = self.v111['a']

        # test sequence setitem operates correctly
        v = Vector((1, 2, 3))
        v[0] = 5
        msg = 'vector sequence set item'
        self.assertEqual(v[0], 5, msg=msg)
        v[-1] = 68
        self.assertEqual(v[2], 68, msg=msg)

        # test exception on index range
        msg = 'vector sequence set IndexError'
        with self.assertRaises(IndexError, msg=msg):
            self.v111[3] = 5

        with self.assertRaises(IndexError, msg=msg):
            self.v111[-4] = 5

        # test exception on index type
        msg = 'vector sequence set TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.v111[0] = 'a'

    def test_vector_compare(self):
        # test equality and inequality operators
        v = Vector((1, 2, 3))
        self.assertEqual(v, self.v123, msg='vector compare equal')
        self.assertNotEqual(v, self.v111, msg='vector compare not equal')
        self.assertTrue(v == self.v123, msg='vector compare == true')
        self.assertTrue(v != self.v111, msg='vector compare != true')
        self.assertFalse(v == self.v111, msg='vector compare == false')
        self.assertFalse(v != self.v123, msg='vector compare != false')

        # check equality of very very small values
        x = 0.1
        sumValue = 0
        for i in range(10):
            sumValue += x
        msg = 'vector compare small value check'
        self.assertEqual(Vector((sumValue, 0, 0)), Vector((1, 0, 0)), msg=msg)

    @staticmethod
    def equality(vector, array):
        return (vector[0] == array[0] and 
                vector[1] == array[1] and 
                vector[2] == array[2])

    def test_vector_buffer(self):
        # test buffer correctly passes correct memory
        v = Vector((1, 2, 3))
        view = memoryview(v)
        v[2] = 5
        self.assertEqual(v, Vector((1, 2, 5)), msg='vector buffer set')
        view[2] = 68
        msg = 'vector buffer set from view'
        self.assertEqual(v, Vector((1, 2, 68)), msg=msg)
        
        # test correct size of memory buffer
        msg = 'vector buffer view IndexError'
        with self.assertRaises(IndexError, msg=msg):
            tmp = view[3]

        # test correct data type interpretation by memoryview
        msg = 'vector buffer view set TypeError'
        with self.assertRaises(TypeError, msg=msg):
            view[0] = 'a'

    def test_vector_pickle(self):
        # test creating bytes from pickle
        #   (test succeeds by not raises exception)
        buf = pickle.dumps(self.v123)

        # test validity of exported bytes and creation of object with pickle
        v = pickle.loads(buf)
        self.assertEqual(v, self.v123, msg='vector pickle')

        # test copy module's use of __reduce__()
        cpy = copy(self.v123)
        msg = 'vector copy from copy module'
        self.assertEqual(cpy, self.v123, msg=msg)
        msg = 'vector copy not same object'
        self.assertIsNot(cpy, self.v123, msg=msg)

    def test_vector_dot(self):
        # test dot operator computations
        msg = 'vector dot product'
        self.assertEqual(dot(self.v123, self.v34), 11, msg=msg)
        msg = 'vector dot equal to mag2()'
        self.assertEqual(dot(self.v123, self.v123), self.v123.mag2(), msg=msg)

        # test exceptions from types
        msg = 'dot() argument number < 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            dot(self.v123)

        msg = 'dot() argument number > 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            dot(self.v123, self.v123, self.v123)

        msg = 'dot() lhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            dot(self.v123, 0)

    def test_vector_cross(self):
        # test cross operator computations
        msg = 'vector cross product'
        self.assertEqual(cross(self.v123, self.v34), Vector((-12, 9, -2)), msg=msg)
        self.assertEqual(cross(self.v34, self.v123), Vector((12, -9, 2)), msg=msg)

        # test exception from types
        msg = 'cross() argument length < 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            cross(self.v123)

        msg = 'cross() argument length > 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            cross(self.v123, self.v123, self.v123)

        msg = 'cross() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            cross(self.v123, 1.0)

    def test_vector_norm(self):
        # test norm operator computations
        msg = 'vector norm'
        self.assertEqual(norm(self.v34), Vector((0.6, 0.8, 0)), msg=msg)
        self.assertEqual(norm(self.v34 * -1), Vector((-0.6, -0.8, 0)), msg=msg)
        v = Vector((1, 2, 3))
        v.normalize()
        msg = 'vector norm() same output as .normalize()'
        self.assertEqual(norm(self.v123), v, msg=msg)

        # test exception from types
        msg = 'norm() no argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            norm()

        msg = 'norm() argument length > 1 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            norm(self.v123, self.v123)

        msg = 'norm() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            norm(1)

    def test_vector_vang(self):
        # test vang computation values
        msg = 'vector angle'
        self.assertAlmostEqual(vang(Vector((1, 1, 0)), Vector((1, 0, 0))),
                               pi / 4, 6, msg=msg)
        self.assertAlmostEqual(vang(Vector((1, 0, 0)), Vector((0, 1, 0))),
                               pi / 2, 6, msg=msg)

        # test exception from types
        msg = 'vang() argument length < 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            vang(self.v123)

        msg = 'vang() argument length > 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            vang(self.v123, self.v123, self.v123)
        
        msg = 'vang() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            vang(2)

    def test_vector_exclude(self):
        # test vxcl computation values
        msg = 'vector exclude'
        self.assertEqual(vxcl(Vector((1, 1, 0)), Vector((0, 1, 0))),
                         Vector((1, 0, 0)), msg=msg)
        self.assertEqual(vxcl(Vector((1, 2, 0)), Vector((0, 1, 0))),
                         Vector((1, 0, 0)), msg=msg)

        # test exception from types
        msg = 'vxcl() argument length < 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            vang(self.v123)

        msg = 'vxcl() argument length > 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            vang(self.v123, self.v123, self.v123)

        msg = 'vxcl() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            vang(5)

    def test_vector_projection(self):
        # test proj computation values
        msg = 'vector projection'
        self.assertEqual(proj(Vector((1, 1, 7)), Vector((1, 1, 1))),
                         Vector((3, 3, 3)), msg=msg)
        self.assertEqual(proj(Vector.e1, Vector.e2), Vector(), msg=msg)

        # test exception from types
        msg = 'proj() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            proj(3, 'a')

        msg = 'proj() argument length < 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            proj(self.v123)

        msg = 'proj() argument length > 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            proj(self.v123, self.v123, self.v123)


if __name__ == '__main__':
    unittest.main()
