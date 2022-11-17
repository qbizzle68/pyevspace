from pyevspace import *
import unittest


class Test_ematrix(unittest.TestCase):

    mi = EMatrix((1, 0, 0), (0, 1, 0), (0, 0, 1))
    m123 = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m147 = EMatrix((1, 4, 7), (2, 5, 8), (3, 6, 9))
    v123 = EVector((1, 2, 3))
    m2 = EMatrix((2, 4, 6), (8, 10, 12), (14, 16, 18))

    def test_add(self):
        ans = EMatrix((2, 6, 10), (6, 10, 14), (10, 14, 18))
        self.assertEqual(self.m123 + self.m147, ans)

        with self.assertRaises(TypeError) as cm:
            self.v123 + 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v123 + 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_iadd(self):
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m += self.m147
        ans = EMatrix((2, 6, 10), (6, 10, 14), (10, 14, 18))
        self.assertEqual(self.m123 + self.m147, ans)

        with self.assertRaises(TypeError) as cm:
            self.v123 += 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v123 += 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_subtract(self):
        ans = EMatrix((0, -2, -4), (2, 0, -2), (4, 2, 0))
        self.assertEqual(self.m123 - self.m147, ans)

        with self.assertRaises(TypeError) as cm:
            self.v123 - 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v123 - 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_isubtract(self):
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m -= self.m147
        ans = EMatrix((0, -2, -4), (2, 0, -2), (4, 2, 0))
        self.assertEqual(m, ans)

        with self.assertRaises(TypeError) as cm:
            self.v123 += 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v123 += 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_scalar_multiply(self):
        # ans = EMatrix((2, 4, 6), (8, 10, 12), (14, 16, 18))
        self.assertEqual(self.m123 * 2, self.m2)
        self.assertEqual(self.m123 * 2.0, self.m2)

        with self.assertRaises(TypeError) as cm:
            self.m123 * range(5)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.v123 * 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_multiply(self):
        self.assertEqual(self.m123 * self.v123, EVector((14, 32, 50)))

    def test_matrix_multiply(self):
        ans = EMatrix((14, 32, 50), (32, 77, 122), (50, 122, 194))
        self.assertEqual(self.m123 * self.m147, ans)

    def test_imultiply(self):
        # ans = EMatrix((2, 4, 6), (8, 10, 12), (14, 16, 18))
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m *= 2
        self.assertEqual(m, self.m2)
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m *= 2.0
        self.assertEqual(m, self.m2)

    def test_divide(self):
        ans = EMatrix((0.5, 1, 1.5), (2, 2.5, 3), (3.5, 4, 4.5))
        self.assertEqual(self.m123 / 2, ans)
        self.assertEqual(self.m123 / 2.0, ans)
        self.assertEqual(self.m123 / 0.5, self.m2)

    def test_idivide(self):
        ans = EMatrix((0.5, 1, 1.5), (2, 2.5, 3), (3.5, 4, 4.5))
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 2
        self.assertEqual(m, ans)
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 2.0
        self.assertEqual(m, ans)
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 0.5
        self.assertEqual(m, self.m2)

    def test_get(self):
        self.assertEqual(self.m123[0, 0], 1)
        self.assertEqual(self.m123[1, 2], 6)

        with self.assertRaises(SystemError) as cm:
            x = self.m123[0]
        self.assertEqual(SystemError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            x = self.m123[1, 'a']
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            x = self.m123[1, 0, 2]
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            x = self.m123[3, 1]
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            x = self.m123[2, 5]
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            x = self.m123[0, -5]
        self.assertEqual(IndexError, type(cm.exception))

    def test_set(self):
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m[0, 0] = 42
        self.assertEqual(m[0, 0], 42)

        with self.assertRaises(SystemError) as cm:
            self.m123[0] = 4
        self.assertEqual(SystemError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123[1, 'a'] = 2
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123[1, 0, 2] = 5
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            self.m123[3, 1] = 3
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            self.m123[2, 5] = 8
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            self.m123[0, -5] = 7
        self.assertEqual(IndexError, type(cm.exception))

    def test_compare(self):
        m = EMatrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        self.assertEqual(m, self.m123)
        self.assertNotEqual(m, self.m147)
        self.assertTrue(m == self.m123)
        self.assertFalse(m == self.m147)
        self.assertTrue(m != self.m147)
        self.assertFalse(m != self.m123)


if __name__ == '__main__':
    unittest.main()
