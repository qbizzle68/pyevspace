from pyevspace import *
import unittest
import pickle


class Test_matrix(unittest.TestCase):

    mi = Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1))
    m123 = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m147 = Matrix((1, 4, 7), (2, 5, 8), (3, 6, 9))
    v123 = Vector((1, 2, 3))
    m2 = Matrix((2, 4, 6), (8, 10, 12), (14, 16, 18))

    def test_matrix_new(self):
        # test if non iterable in constructor
        with self.assertRaises(TypeError) as cm:
            Matrix(1, (1, 2, 3), (2, 3, 4))
        self.assertEqual(TypeError, type(cm.exception))

        # test iterable less than length 3
        with self.assertRaises(ValueError) as cm:
            Matrix((1, 2), (1, 2, 3), (1, 2, 3))
        self.assertEqual(ValueError, type(cm.exception))

        # test iterable more than length 3
        with self.assertRaises(ValueError) as cm:
            Matrix((1, 2, 3), (4, 5, 6, 7), (8, 9, 10))
        self.assertEqual(ValueError, type(cm.exception))

        # test non numeric value
        with self.assertRaises(TypeError) as cm:
            Matrix((1, 2, 'a'), (4, 5, 6), (7, 8, 9))
        self.assertEqual(TypeError, type(cm.exception))

        # test empty constructor
        self.assertEqual(Matrix(), Matrix((0, 0, 0), (0, 0, 0), (0, 0, 0)))

    def test_matrix_str(self):
        # test int only matrix
        self.assertEqual(str(self.m123), '[[1, 2, 3]\n[4, 5, 6]\n[7, 8, 9]]')

        # test float 
        self.assertEqual(
            str(Matrix((1.1, 2.2, 3.3), (4.4, 5.5, 6.6), (7.7, 8.8, 9.9))),
            '[[1.1, 2.2, 3.3]\n[4.4, 5.5, 6.6]\n[7.7, 8.8, 9.9]]')

    def test_matrix_repr(self):
        # test int only matrix
        self.assertEqual(repr(self.m123), 'Matrix([[1, 2, 3]\n\t[4, 5, 6]\n\t[7, 8, 9]])')

        # test float 
        self.assertEqual(
            repr(Matrix((1.1, 2.2, 3.3), (4.4, 5.5, 6.6), (7.7, 8.8, 9.9))),
            'Matrix([[1.1, 2.2, 3.3]\n\t[4.4, 5.5, 6.6]\n\t[7.7, 8.8, 9.9]])')
        pass

    def test_add(self):
        # test matrix add computation
        ans = Matrix((2, 6, 10), (6, 10, 14), (10, 14, 18))
        self.assertEqual(self.m123 + self.m147, ans)

        # test exceptions from types
        with self.assertRaises(TypeError) as cm:
            self.m123 + 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123 + 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_iadd(self):
        # test inplace matrix add computation
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m += self.m147
        ans = Matrix((2, 6, 10), (6, 10, 14), (10, 14, 18))
        self.assertEqual(self.m123 + self.m147, ans)

        # test exceptions from types
        with self.assertRaises(TypeError) as cm:
            self.m123 += 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123 += 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_subtract(self):
        # test matrix subtraction computations
        ans = Matrix((0, -2, -4), (2, 0, -2), (4, 2, 0))
        self.assertEqual(self.m123 - self.m147, ans)

        # test exceptions from types
        with self.assertRaises(TypeError) as cm:
            self.m123 - 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123 - 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_isubtract(self):
        # test matrix inplace subtraction computations
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m -= self.m147
        ans = Matrix((0, -2, -4), (2, 0, -2), (4, 2, 0))
        self.assertEqual(m, ans)

        # test exceptions from types
        with self.assertRaises(TypeError) as cm:
            self.m123 -= 1
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123 -= 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_scalar_multiply(self):
        # test matrix scalar multiplication computations
        self.assertEqual(self.m123 * 2, self.m2)
        self.assertEqual(self.m123 * 2.0, self.m2)

        # test exceptions from types
        with self.assertRaises(TypeError) as cm:
            self.m123 * range(5)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123 * 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_vector_multiply(self):
        # test vector multiplication computations
        self.assertEqual(self.m123 * self.v123, Vector((14, 32, 50)))

    def test_matrix_multiply(self):
        # test matrix multiplication computations
        ans = Matrix((14, 32, 50), (32, 77, 122), (50, 122, 194))
        self.assertEqual(self.m123 * self.m147, ans)

    def test_imultiply(self):
        # test inplace scalar multiplication compuattinos
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m *= 2
        self.assertEqual(m, self.m2)
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m *= 2.0
        self.assertEqual(m, self.m2)

        # test exception from types
        with self.assertRaises(TypeError) as cm:
            self.m123 *= self.v123
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123 *= m
        self.assertEqual(TypeError, type(cm.exception))

    def test_divide(self):
        # test scalar division computations
        ans = Matrix((0.5, 1, 1.5), (2, 2.5, 3), (3.5, 4, 4.5))
        self.assertEqual(self.m123 / 2, ans)
        self.assertEqual(self.m123 / 2.0, ans)
        self.assertEqual(self.m123 / 0.5, self.m2)

        # test exceptions from types
        with self.assertRaises(TypeError) as cm:
            tmp = self.m123 / self.v123
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            tmp = self.m123 / 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_idivide(self):
        # test inplace scalar division computations
        ans = Matrix((0.5, 1, 1.5), (2, 2.5, 3), (3.5, 4, 4.5))
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 2
        self.assertEqual(m, ans)
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 2.0
        self.assertEqual(m, ans)
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 0.5
        self.assertEqual(m, self.m2)

        # test exceptions from types
        with self.assertRaises(TypeError) as cm:
            self.m123 /= self.v123
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123 /= 'a'
        self.assertEqual(TypeError, type(cm.exception))

    def test_get(self):
        # test mapping to a value
        self.assertEqual(self.m123[0, 0], 1)
        self.assertEqual(self.m123[1, 2], 6)

        # test exceptions of index types
        with self.assertRaises(TypeError) as cm:
            x = self.m123[1, 'a']
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            x = self.m123[1, 0, 2]
        self.assertEqual(TypeError, type(cm.exception))

        # test exceptions of index ranges
        with self.assertRaises(IndexError) as cm:
            x = self.m123[3, 1]
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            x = self.m123[2, 5]
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            x = self.m123[0, -5]
        self.assertEqual(IndexError, type(cm.exception))

        # test map to row
        row = self.m123[1]
        self.assertEqual(row[0], 4)
        self.assertEqual(row[1], 5)
        self.assertEqual(row[2], 6)

    def test_set(self):
        # test setting mapped to value
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m[0, 0] = 42
        self.assertEqual(m[0, 0], 42)

        # test exception on types
        with self.assertRaises(SystemError) as cm:
            self.m123[0] = 4
        self.assertEqual(SystemError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123[1, 'a'] = 2
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            self.m123[1, 0, 2] = 5
        self.assertEqual(TypeError, type(cm.exception))

        # test exception on index ranges
        with self.assertRaises(IndexError) as cm:
            self.m123[3, 1] = 3
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            self.m123[2, 5] = 8
        self.assertEqual(IndexError, type(cm.exception))

        with self.assertRaises(IndexError) as cm:
            self.m123[0, -5] = 7
        self.assertEqual(IndexError, type(cm.exception))

        # test setting views as rows
        m[1][1] = 4
        self.assertEqual(m[1, 1], 4)

    def test_compare(self):
        # test equality and inequality operators
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        self.assertEqual(m, self.m123)
        self.assertNotEqual(m, self.m147)
        self.assertTrue(m == self.m123)
        self.assertFalse(m == self.m147)
        self.assertTrue(m != self.m147)
        self.assertFalse(m != self.m123)

    def equality(self, matrix, array):
        return (matrix[0, 0] == array[0, 0]
                and matrix[0, 1] == array[0, 1]
                and matrix[0, 2] == array[0, 2]
                and matrix[1, 0] == array[1, 0]
                and matrix[1, 1] == array[1, 1]
                and matrix[1, 2] == array[1, 2]
                and matrix[2, 0] == array[2, 0]
                and matrix[2, 1] == array[2, 1]
                and matrix[2, 2] == array[2, 2])

    def test_buffer(self):
        # test buffer as a memoryview
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        view = memoryview(m)
        m[1, 1] = 10
        self.assertTrue(self.equality(m, view))
        m[1, 1] = 68
        self.assertTrue(self.equality(m, view))

    def test_pickle(self):
        # test ability to pickle matrix
        #   test passes by not raising exception
        buf = pickle.dumps(self.m123)

        # test ability to create matrix from bytes
        m = pickle.loads(buf)
        self.assertEqual(m, self.m123)

if __name__ == '__main__':
    unittest.main()
