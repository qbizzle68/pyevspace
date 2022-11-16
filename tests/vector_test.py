from math import sqrt

from pyevspace import EVector
import unittest


# class holding the vector tests
class Test_evector(unittest.TestCase):

    v111 = EVector((1, 1, 1))
    v123 = EVector((1, 2, 3))
    v123m = EVector((-1, -2, -3))

    def test_vector_add(self):
        self.assertEqual(self.v111 + self.v123, EVector((2, 3, 4)))
        self.assertEqual(self.v111 + self.v123m, EVector((0, -1, -2)))

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
        v = EVector((1, 1, 1))
        v += self.v123
        self.assertEqual(v, EVector((2, 3, 4)))
        v = EVector((-1, -2, -3))
        v += self.v123
        self.assertEqual(v, EVector((0, 0, 0)))

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
        self.assertEqual(self.v123 - self.v111, EVector((0, 1, 2)))
        self.assertEqual(self.v111 - self.v123, EVector((0, -1, -2)))

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
        v = EVector((1, 1, 1))
        v -= self.v123
        self.assertEqual(v, EVector((0, -1, -2)))
        v = EVector((1, 1, 1))
        v -= self.v123m
        self.assertEqual(v, EVector((2, 3, 4)))

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
        self.assertEqual(self.v123 * 2, EVector((2, 4, 6)))
        self.assertEqual(self.v123m * 0.5, EVector((-0.5, -1, -1.5)))

        with self.assertRaises(TypeError) as cm:
            self.v111 * 'a'
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 * EVector()
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_imultiply(self):
        v = EVector((1, 2, 3))
        v *= 2
        self.assertEqual(v, EVector((2, 4, 6)))
        v = EVector((-1, -1, -1))
        v *= 0.5
        self.assertEqual(v, EVector((-0.5, -0.5, -0.5)))

        with self.assertRaises(TypeError) as cm:
            self.v111 *= 'a'
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 *= EVector()
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_divide(self):
        self.assertEqual(self.v123 / 1, self.v123)
        self.assertEqual(self.v123m / 0.1, EVector((-10, -20, -30)))

        with self.assertRaises(TypeError) as cm:
            self.v111 / 'a'
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 / EVector()
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_idivide(self):
        v = EVector((1, 2, 3))
        v /= 2
        self.assertEqual(v, EVector((0.5, 1, 1.5)))
        v = EVector((-1, -1, -1))
        v /= 0.5
        self.assertEqual(v, EVector((-2, -2, -2)))

        with self.assertRaises(TypeError) as cm:
            self.v111 /= 'a'
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v111 /= EVector()
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_mag(self):
        v = EVector((3, 4, 0))
        self.assertEqual(v.mag(), 5)
        x = sqrt(1/3)
        v = EVector((x, x, x))
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
        v = EVector((x, x, x))
        v.normalize()
        self.assertEqual(v, EVector((x, x, x)))
        v = EVector((3, 4, 0))
        v.normalize()
        self.assertEqual(v, EVector((0.6, 0.8, 0)))

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

        v = EVector((1, 2, 3))
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


if __name__ == '__main__':
    unittest.main()
