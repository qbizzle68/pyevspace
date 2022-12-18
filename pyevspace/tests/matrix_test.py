from pyevspace import Matrix, Vector, det, transpose
import unittest
import pickle


class TestMatrix(unittest.TestCase):

    mi = Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1))
    m123 = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    m147 = Matrix((1, 4, 7), (2, 5, 8), (3, 6, 9))
    v123 = Vector((1, 2, 3))
    m2 = Matrix((2, 4, 6), (8, 10, 12), (14, 16, 18))

    def test_matrix_new(self):
        # test if non-iterable in constructor
        msg = 'Matrix() constructor non-iterable TypeError'
        with self.assertRaises(TypeError, msg=msg):
            Matrix(1, (1, 2, 3), (2, 3, 4))

        # test iterable less than length 3
        msg = 'Matrix() constructor iterable length < 3 ValueError'
        with self.assertRaises(ValueError, msg=msg):
            Matrix((1, 2), (1, 2, 3), (1, 2, 3))

        # test iterable more than length 3
        msg = 'Matrix() constructor iterable length > 3 ValueError'
        with self.assertRaises(ValueError, msg=msg):
            Matrix((1, 2, 3), (4, 5, 6, 7), (8, 9, 10))

        # test non numeric value
        msg = 'Matrix() constructor iterable non-numeric value TypeError'
        with self.assertRaises(TypeError, msg=msg):
            Matrix((1, 2, 'a'), (4, 5, 6), (7, 8, 9))

        # test empty constructor
        msg = 'Matrix() empty constructor initializing to zeros'
        self.assertEqual(Matrix(), Matrix((0, 0, 0), (0, 0, 0), (0, 0, 0)),
                         msg=msg)

    def test_matrix_str(self):
        # test int only matrix
        msg = 'matrix str() int'
        self.assertEqual(str(self.m123), '[[1, 2, 3]\n[4, 5, 6]\n[7, 8, 9]]',
                         msg=msg)

        # test float
        msg = 'matrix str() float'
        self.assertEqual(
            str(Matrix((1.1, 2.2, 3.3), (4.4, 5.5, 6.6), (7.7, 8.8, 9.9))),
            '[[1.1, 2.2, 3.3]\n[4.4, 5.5, 6.6]\n[7.7, 8.8, 9.9]]', msg=msg)

    def test_matrix_repr(self):
        # test int only matrix
        msg = 'matrix repr() int'
        self.assertEqual(repr(self.m123),
                         'Matrix([[1, 2, 3]\n\t[4, 5, 6]\n\t[7, 8, 9]])',
                         msg=msg)

        # test float
        msg = 'matrix repr() float'
        self.assertEqual(
            repr(Matrix((1.1, 2.2, 3.3), (4.4, 5.5, 6.6), (7.7, 8.8, 9.9))),
            'Matrix([[1.1, 2.2, 3.3]\n\t[4.4, 5.5, 6.6]\n\t[7.7, 8.8, 9.9]])',
            msg=msg)

    def test_matrix_add(self):
        # test matrix add computation
        ans = Matrix((2, 6, 10), (6, 10, 14), (10, 14, 18))
        msg = 'matrix add two matrices'
        self.assertEqual(self.m123 + self.m147, ans, msg=msg)

        # test exceptions from types
        msg = 'matrix add rhs arg TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.m123 + 1

        msg = 'matrix add rhs arg TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.m123 + 'a'

    def test_matrix_iadd(self):
        # test inplace matrix add computation
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m += self.m147
        ans = Matrix((2, 6, 10), (6, 10, 14), (10, 14, 18))
        msg = 'matrix iadd two matrices'
        self.assertEqual(self.m123 + self.m147, ans, msg=msg)

        # test exceptions from types
        msg = 'matrix iadd rhs arg TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.m123 += 1

        msg = 'matrix iadd rhs arg TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.m123 += 'a'

    def test_matrix_subtract(self):
        # test matrix subtraction computations
        ans = Matrix((0, -2, -4), (2, 0, -2), (4, 2, 0))
        msg = 'matrix subtract two matrices'
        self.assertEqual(self.m123 - self.m147, ans, msg=msg)

        # test exceptions from types
        msg = 'matrix subtract rhs arg TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.m123 - 1

        with self.assertRaises(TypeError, msg=msg):
            ans = self.m123 - 'a'

    def test_matrix_isubtract(self):
        # test matrix inplace subtraction computations
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m -= self.m147
        ans = Matrix((0, -2, -4), (2, 0, -2), (4, 2, 0))
        msg = 'matrix isubtract two matrices'
        self.assertEqual(m, ans, msg=msg)

        # test exceptions from types
        msg = 'matrix isubtract rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.m123 -= 1

        with self.assertRaises(TypeError, msg=msg):
            self.m123 -= 'a'

    def test_scalar_multiply(self):
        # test matrix scalar multiplication computations
        msg = 'matrix multiply scalar'
        self.assertEqual(self.m123 * 2, self.m2, msg=msg)
        self.assertEqual(self.m123 * 2.0, self.m2, msg=msg)

        # test exceptions from types
        msg = 'matrix multiply rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            ans = self.m123 * range(5)

        with self.assertRaises(TypeError, msg=msg):
            ans = self.m123 * 'a'

    def test_vector_multiply(self):
        # test vector multiplication computations
        msg = 'matrix multiply matrix and vector'
        self.assertEqual(self.m123 * self.v123, Vector((14, 32, 50)), msg=msg)

    def test_matrix_multiply(self):
        # test matrix multiplication computations
        ans = Matrix((14, 32, 50), (32, 77, 122), (50, 122, 194))
        msg = 'matrix multiply two matrices'
        self.assertEqual(self.m123 * self.m147, ans, msg=msg)

    def test_matrix_imultiply(self):
        # test inplace scalar multiplication computations
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m *= 2
        msg = 'matrix imultiply scalar'
        self.assertEqual(m, self.m2, msg=msg)
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m *= 2.0
        self.assertEqual(m, self.m2, msg=msg)

        # test exception from types
        msg = 'matrix imultiply rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.m123 *= self.v123

        with self.assertRaises(TypeError, msg=msg):
            self.m123 *= m

    def test_matrix_divide(self):
        # test scalar division computations
        ans = Matrix((0.5, 1, 1.5), (2, 2.5, 3), (3.5, 4, 4.5))
        msg = 'matrix divide scalar'
        self.assertEqual(self.m123 / 2, ans, msg=msg)
        self.assertEqual(self.m123 / 2.0, ans, msg=msg)
        self.assertEqual(self.m123 / 0.5, self.m2, msg=msg)

        # test exceptions from types
        msg = 'matrix divide rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            tmp = self.m123 / self.v123

        with self.assertRaises(TypeError, msg=msg):
            tmp = self.m123 / 'a'

    def test_matrix_idivide(self):
        # test inplace scalar division computations
        ans = Matrix((0.5, 1, 1.5), (2, 2.5, 3), (3.5, 4, 4.5))
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 2
        msg = 'matrix idivide scalar'
        self.assertEqual(m, ans, msg=msg)
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 2.0
        self.assertEqual(m, ans, msg=msg)
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m /= 0.5
        self.assertEqual(m, self.m2, msg=msg)

        # test exceptions from types
        msg = 'matrix idivide rhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.m123 /= self.v123

        with self.assertRaises(TypeError, msg=msg) as cm:
            self.m123 /= 'a'

    def test_matrix_get(self):
        # test mapping to a value
        msg = 'matrix getitem'
        self.assertEqual(self.m123[0, 0], 1, msg=msg)
        self.assertEqual(self.m123[1, 2], 6, msg=msg)

        # test exceptions of index types
        msg = 'matrix getitem index TypeError'
        with self.assertRaises(TypeError, msg=msg):
            x = self.m123[1, 'a']

        with self.assertRaises(TypeError, msg=msg):
            x = self.m123[1, 0, 2]

        # test exceptions of index ranges
        msg = 'matrix getitem IndexError'
        with self.assertRaises(IndexError, msg=msg):
            x = self.m123[3, 1]

        with self.assertRaises(IndexError, msg=msg):
            x = self.m123[2, 5]

        with self.assertRaises(IndexError, msg=msg):
            x = self.m123[0, -5]

        # test map to row
        row = self.m123[1]
        msg = 'matrix get row buffer'
        self.assertEqual(row[0], 4, msg=msg)
        self.assertEqual(row[1], 5, msg=msg)
        self.assertEqual(row[2], 6, msg=msg)

    def test_matrix_set(self):
        # test setting mapped to value
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        m[0, 0] = 42
        msg = 'matrix setitem'
        self.assertEqual(m[0, 0], 42, msg=msg)

        # test exception on types
        msg = 'matrix set row to bad type SystemError'
        with self.assertRaises(SystemError, msg=msg):
            self.m123[0] = 4

        msg = 'matrix setitem index TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.m123[1, 'a'] = 2

        msg = 'matrix setitem index too many args TypeError'
        with self.assertRaises(TypeError, msg=msg):
            self.m123[1, 0, 2] = 5

        # test exception on index ranges
        msg = 'matrix setitem index IndexError'
        with self.assertRaises(IndexError, msg=msg):
            self.m123[3, 1] = 3

        with self.assertRaises(IndexError, msg=msg):
            self.m123[2, 5] = 8

        with self.assertRaises(IndexError, msg=msg):
            self.m123[0, -5] = 7

        # test setting views as rows
        m[1][1] = 4
        msg = 'matrix set row item'
        self.assertEqual(m[1, 1], 4, msg=msg)

    def test_matrix_compare(self):
        # test equality and inequality operators
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        self.assertEqual(m, self.m123, msg='matrix compare equal')
        self.assertNotEqual(m, self.m147, msg='matrix compare not equal')
        self.assertTrue(m == self.m123, msg='matrix compare == true')
        self.assertFalse(m == self.m147, msg='matrix compare == false')
        self.assertTrue(m != self.m147, msg='matrix compare != true')
        self.assertFalse(m != self.m123, msg='matrix compare != false')

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

    def test_matrix_buffer(self):
        # test buffer as a memoryview
        m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
        view = memoryview(m)
        m[1, 1] = 10
        msg = 'matrix buffer set'
        self.assertTrue(self.equality(m, view), msg=msg)
        m[1, 1] = 68
        msg = 'matrix buffer set view'
        self.assertTrue(self.equality(m, view), msg=msg)

    def test_matrix_pickle(self):
        # test ability to pickle matrix
        #   test passes by not raising exception
        buf = pickle.dumps(self.m123)

        # test ability to create matrix from bytes
        m = pickle.loads(buf)
        msg = 'matrix pickle'
        self.assertEqual(m, self.m123, msg=msg)

    def test_matrix_determinate(self):
        # test det with non-invertible matrix
        msg = 'matrix determinate expected output'
        self.assertEqual(det(self.m123), 0, msg=msg)

        # test det with-invertible matrix
        m = Matrix((2, 6, 4), (7, 3, 1), (8, 0, 0))
        self.assertEqual(det(m), -48, msg=msg)

        # test exception from types
        msg = 'det() argument length < 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            det()

        msg = 'det() argument length > 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            det(self.v123, self.v123)

        msg = 'det() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            det(7)

    def test_matrix_transpose(self):
        # test transpose expected values
        # ans = Matrix((1, 4, 7), (2, 5, 8), (3, 6, 9))
        msg = 'matrix transpose expected output'
        self.assertEqual(transpose(self.m123), self.m147, msg=msg)

        # test exception from types
        msg = 'transpose() argument length < 1 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            transpose()

        msg = 'transpose() argument length > 1 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            transpose(self.m123, self.m123)

        msg = 'transpose() argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            transpose(5)


if __name__ == '__main__':
    unittest.main()
