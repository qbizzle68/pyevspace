from math import sqrt
from typing import Type

from pyevspace import Vector
import unittest


# class holding the vector tests
class Test_vector(unittest.TestCase):

    v111 = Vector((1, 1, 1))
    v123 = Vector((1, 2, 3))
    v123m = Vector((-1, -2, -3))

    def test_vector_add(self):
        # test if non iterable in constructor
        with self.assertRaises(TypeError) as cm:
            Vector(1, 2, 3)
        self.assertEqual(TypeError, type(cm.exception))

        # test iterable less than length 3
        with self.assertRaises(TypeError) as cm:
            Vector((1, 2))
        self.assertEqual(TypeError, type(cm.exception))

        # test iterable more than length 3
        with self.assertRaises(TypeError) as cm:
            Vector((1, 2, 3, 4))
        self.assertEqual(TypeError, type(cm.exception))

        # test non numeric value in each index
        with self.assertRaises(TypeError) as cm:
            Vector(('a', 1, 2))
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            Vector((0, 'a', 2))
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            Vector((0, 1, 'a'))
        self.assertEqual(TypeError, type(cm.exception))

        # test empty constructor
        self.assertEqual(Vector(), Vector((0, 0, 0)))

        # test non list or tuple sequence
        d = {0: 'a', 1: 'b', 2: 'c'}
        self.assertEqual(Vector((0, 1, 2)), Vector(d))

    def test_vector_str(self):
        # test int only vectors
        self.assertEqual(str(self.v123), '[1, 2, 3]')

        # test float integer only vectors
        self.assertEqual(str(Vector((1.0, 2.0, 3.0))), '[1, 2, 3]')

        # test float only vectors
        self.assertEqual(str(Vector((1.1, 2.2, 3.3))), '[1.1, 2.2, 3.3]')

        # test border of scientific notation
        self.assertEqual(str(Vector((123456.1234, 1234567.891, 0.123456))),
                         '[123456, 1.23457e+06, 0.123456]')
        self.assertEqual(str(Vector((0.0001234567, 0.00001234567, 0.000001234567))),
                         '[0.000123457, 1.23457e-05, 1.23457e-06]')

    def test_vector_repr(self):
        # test int only vectors
        self.assertEqual(repr(self.v123), 'Vector([1, 2, 3])')

        # test float integer only vectors
        self.assertEqual(repr(Vector((1.0, 2.0, 3.0))), 'Vector([1, 2, 3])')

        # test float only vectors
        self.assertEqual(repr(Vector((1.1, 2.2, 3.3))), 'Vector([1.1, 2.2, 3.3])')

        # test border of scientific notation
        self.assertEqual(repr(Vector((123456.1234, 1234567.891, 0.123456))),
                         'Vector([123456, 1.23457e+06, 0.123456])')
        self.assertEqual(repr(Vector((0.0001234567, 0.00001234567, 0.000001234567))),
                         'Vector([0.000123457, 1.23457e-05, 1.23457e-06])')

    def test_vector_iter(self):
        # test __iter__() by checking __next__() values
        itr = iter(self.v123)
        n = next(itr)
        self.assertEqual(n, 1.0)
        n = next(itr)
        self.assertEqual(n, 2.0)
        n = next(itr)
        self.assertEqual(n, 3.0)
        
        # test length of iteration is correct
        with self.assertRaises(StopIteration) as cm:
            n = next(itr)
        self.assertEqual(StopIteration, type(cm.exception))

        # test with generation
        ls = [i for i in self.v123]
        self.assertEqual(ls, [1.0, 2.0, 3.0])
        
        # test in operator
        self.assertIn(1, self.v123)
        self.assertNotIn(0, self.v123)

    def test_vector_add(self):
        self.assertEqual(self.v111 + self.v123, Vector((2, 3, 4)))
        self.assertEqual(self.v111 + self.v123m, Vector((0, -1, -2)))

        with self.assertRaises(TypeError) as cm:
            self.v111 + 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 + 1.0
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 + 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_iadd(self):
        v = Vector((1, 1, 1))
        v += self.v123
        self.assertEqual(v, Vector((2, 3, 4)))
        v = Vector((-1, -2, -3))
        v += self.v123
        self.assertEqual(v, Vector((0, 0, 0)))

        with self.assertRaises(TypeError) as cm:
            self.v111 += 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 += 1.0
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 += 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_subtract(self):
        self.assertEqual(self.v123 - self.v111, Vector((0, 1, 2)))
        self.assertEqual(self.v111 - self.v123, Vector((0, -1, -2)))

        with self.assertRaises(TypeError) as cm:
            self.v111 - 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 - 1.0
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 - 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_isubtract(self):
        v = Vector((1, 1, 1))
        v -= self.v123
        self.assertEqual(v, Vector((0, -1, -2)))
        v = Vector((1, 1, 1))
        v -= self.v123m
        self.assertEqual(v, Vector((2, 3, 4)))

        with self.assertRaises(TypeError) as cm:
            self.v111 -= 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 -= 1.0
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 -= 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_multiply(self):
        self.assertEqual(self.v123 * 2, Vector((2, 4, 6)))
        self.assertEqual(self.v123m * 0.5, Vector((-0.5, -1, -1.5)))

        with self.assertRaises(TypeError) as cm:
            self.v111 * 'a'
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 * Vector()
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_imultiply(self):
        v = Vector((1, 2, 3))
        v *= 2
        self.assertEqual(v, Vector((2, 4, 6)))
        v = Vector((-1, -1, -1))
        v *= 0.5
        self.assertEqual(v, Vector((-0.5, -0.5, -0.5)))

        with self.assertRaises(TypeError) as cm:
            self.v111 *= 'a'
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 *= Vector()
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_divide(self):
        self.assertEqual(self.v123 / 1, self.v123)
        self.assertEqual(self.v123m / 0.1, Vector((-10, -20, -30)))

        with self.assertRaises(TypeError) as cm:
            self.v111 / 'a'
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 / Vector()
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_idivide(self):
        v = Vector((1, 2, 3))
        v /= 2
        self.assertEqual(v, Vector((0.5, 1, 1.5)))
        v = Vector((-1, -1, -1))
        v /= 0.5
        self.assertEqual(v, Vector((-2, -2, -2)))

        with self.assertRaises(TypeError) as cm:
            self.v111 /= 'a'
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 /= Vector()
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_mag(self):
        v = Vector((3, 4, 0))
        self.assertEqual(v.mag(), 5)
        x = sqrt(1/3)
        v = Vector((x, x, x))
        self.assertEqual(v.mag(), 1)

        with self.assertRaises(TypeError) as cm:
            self.v111.mag(1)
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_mag2(self):
        self.assertEqual(self.v123.mag2(), 14)
        self.assertEqual(self.v123m.mag2(), 14)

        with self.assertRaises(TypeError) as cm:
            self.v111.mag2(1)
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_normalize(self):
        x = sqrt(1/3)
        v = Vector((x, x, x))
        v.normalize()
        self.assertEqual(v, Vector((x, x, x)))
        v = Vector((3, 4, 0))
        v.normalize()
        self.assertEqual(v, Vector((0.6, 0.8, 0)))

        with self.assertRaises(TypeError) as cm:
            self.v111.normalize(1)
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_sequence(self):
        self.assertEqual(len(self.v111), 3)
        self.assertEqual(self.v123[0], 1)
        self.assertEqual(self.v123[-1], 3)

        with self.assertRaises(IndexError) as cm:
            x = self.v111[3]
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            x = self.v111[-4]
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            x = self.v111['a']
        self.assertEqual(TypeError, type(cm.exception))

        v = Vector((1, 2, 3))
        v[0] = 5
        self.assertEqual(v[0], 5)
        v[-1] = 68
        self.assertEqual(v[2], 68)

        with self.assertRaises(IndexError) as cm:
            self.v111[3] = 5
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            self.v111[-4] = 5
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111[0] = 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_compare(self):
        v = Vector((1, 2, 3))
        self.assertEqual(v, self.v123)
        self.assertNotEqual(v, self.v111)
        self.assertTrue(v == self.v123)
        self.assertTrue(v != self.v111)
        self.assertFalse(v == self.v111)
        self.assertFalse(v != self.v123)

        # very small values test
        x = 0.1
        sumValue = 0
        for i in range(10):
            sumValue += x
        self.assertEqual(Vector((sumValue, 0, 0)), Vector((1, 0, 0)))

    def equality(self, vector, array):
        return (vector[0] == array[0] and 
                vector[1] == array[1] and 
                vector[2] == array[2])

    def test_buffer(self):
        v = Vector((1, 2, 3))
        view = memoryview(v)
        v[2] = 5
        self.assertEqual(v, Vector((1, 2, 5)))
        view[2] = 68
        self.assertEqual(v, Vector((1, 2, 68)))



if __name__ == '__main__':
    unittest.main()
