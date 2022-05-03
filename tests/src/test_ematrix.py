import sys
sys.path.append(r'C:\dev\libs\pyevspace\pyevspace\bin\x64')
from pyevspace import *
import unittest

class Test_test_matrix(unittest.TestCase):
    v0 = EVector();
    v1 = EVector(1, 2, 3)
    v2 = EVector(1, 2, 3)
    v3 = EVector(4, 5, 6)
    m0 = EMatrix()
    m1 = EMatrix(v0, v1, v3)
    m2 = EMatrix(v1, v3, v2)

    def iadd(self, x):
        try:
            self.m1 += x
            self.fail()
        except:
            pass

    def test_add(self):
        ans = EMatrix(EVector(1, 2, 3), EVector(5, 7, 9), EVector(5, 7, 9))
        self.assertEqual(self.m1 + self.m2, ans)
        mat = self.m1 * 1
        mat += self.m2
        self.assertEqual(mat, ans)
        self.assertRaises(TypeError, lambda x: self.m1 + x, 1)
        self.assertRaises(TypeError, lambda x: self.m1 + x, 1.1)
        self.assertRaises(TypeError, self.iadd(1))
        self.assertRaises(TypeError, self.iadd(1.1))

    def isub(self, x):
        try:
            self.m1 -= x
            self.fail()
        except:
            pass

    def test_sub(self):
        ans = EMatrix(EVector(-1, -2, -3), EVector(-3, -3, -3), EVector(3, 3, 3))
        self.assertEqual(self.m1 - self.m2, ans)
        mat = self.m1 * 1
        mat -= self.m2
        self.assertEqual(mat, ans)
        self.assertRaises(TypeError, lambda x: self.m1 - x, 1)
        self.assertRaises(TypeError, lambda x: self.m1 - x, 1.1)
        self.assertRaises(TypeError, self.isub(1))
        self.assertRaises(TypeError, self.isub(1.1))

    def imult(self, x):
        try:
            self.m1 *= x
            self.fail()
        except:
            pass

    def test_mult(self):
        ans = EMatrix(EVector(0, 0, 0), EVector(1.1, 2.2, 3.3), EVector(4.4, 5.5, 6.6))
        self.assertEqual(self.m1 * 1.1, ans)
        mat = self.m1 * 1
        mat *= 1.1
        self.assertEqual(mat, ans)
        ans = EMatrix(EVector(0, 0, 0), EVector(2, 4, 6), EVector(8, 10, 12))
        self.assertEqual(self.m1 * 2, ans)
        mat = self.m1 *1
        mat *= 2
        self.assertEqual(mat, ans)
        self.assertRaises(TypeError, lambda x: self.m1 * x, self.v1)
        self.assertRaises(TypeError, lambda x: self.m1 * x, "asd")
        self.assertRaises(TypeError, self.imult(self.v1))
        self.assertRaises(TypeError, self.imult("asf"))

    def mimult(self, x):
        try:
            self.m1 @= x
            self.fail()
        except:
            pass

    def test_matrix_mult(self):
        ans = EMatrix(EVector(14, 19, 24), EVector(29, 40, 51), EVector(14, 19, 24))
        self.assertEqual(self.m1 @ self.m2, ans)
        mat = self.m1 * 1
        mat @= self.m2
        self.assertEqual(mat, ans)
        ans = EVector(14, 19, 24)
        self.assertEqual(self.m1 @ self.v1, ans)
        self.assertRaises(TypeError, lambda x: self.m1 @ x, 1)
        self.assertRaises(TypeError, lambda x: self.m1 @ x, 1.1)
        self.assertRaises(TypeError, self.mimult(1))
        self.assertRaises(TypeError, self.mimult(1.1))
        self.assertRaises(TypeError, self.mimult(self.v1))

    def idiv(self, x):
        try:
            self.m1 /= x
            self.fail()
        except:
            pass

    def test_div(self):
        ans = EMatrix(EVector(0,0,0), EVector(2,4,6), EVector(8,10,12))
        self.assertEqual(self.m1 / 0.5, ans)
        mat = self.m1 * 1
        mat /= 2.0
        ans = EMatrix(EVector(0,0,0), EVector(0.5, 1, 1.5), EVector(2, 2.5, 3))
        self.assertEqual(mat, ans)
        self.assertEqual(self.m1 / 2, ans)
        mat = self.m1 * 1
        mat /= 2
        self.assertEqual(mat, ans)
        self.assertRaises(TypeError, lambda x: self.m1 / x, self.v1)
        self.assertRaises(TypeError, lambda x: self.m1 / x, self.m1)
        self.assertRaises(TypeError, self.idiv(self.v1))
        self.assertRaises(TypeError, self.idiv(self.m1))

    def test_set(self):
        mat = self.m1 * 1
        mat.set(1, 1, 5)
        mat.set(2, 1, 8)
        mat.set(0, 2, 38)
        ans = EMatrix(EVector(0, 0, 0), EVector(1, 5, 8), EVector(38, 5, 6))
        self.assertEqual(mat, ans)
        self.assertRaises(TypeError, mat.set, "1", 1, 8)
        self.assertRaises(TypeError, mat.set, 2, 1.2, 8)
        self.assertRaises(ValueError, mat.set, -2, 1, 4)
        self.assertRaises(ValueError, mat.set, 0, -1, 4)
        self.assertRaises(ValueError, mat.set, 6, 1, 4)
        self.assertRaises(ValueError, mat.set, 2, 21, 4)

    def test_get(self):
        self.assertEqual(self.m1.get(0, 0), 0)
        self.assertEqual(self.m1.get(1, 0), 0)
        self.assertEqual(self.m1.get(2, 0), 0)
        self.assertEqual(self.m1.get(0, 1), 1)
        self.assertEqual(self.m1.get(1, 1), 2)
        self.assertEqual(self.m1.get(2, 1), 3)
        self.assertEqual(self.m1.get(0, 2), 4)
        self.assertEqual(self.m1.get(1, 2), 5)
        self.assertEqual(self.m1.get(2, 2), 6)
        self.assertRaises(TypeError, self.m1.get, "1", 1)
        self.assertRaises(TypeError, self.m1.get, 1, 1.2)
        self.assertRaises(ValueError, self.m1.get, -2, 0)
        self.assertRaises(ValueError, self.m1.get, 2, -4)
        self.assertRaises(ValueError, self.m1.get, 5, 0)
        self.assertRaises(ValueError, self.m1.get, 0, 10)

    def test_det(self):
        mat = EMatrix(EVector(3, 6, 4), EVector(1, 6, 3), EVector(8, 43, 1))
        self.assertEqual(det(mat), -251)
        self.assertRaises(TypeError, lambda: det(self.m1, self.m2))
        self.assertRaises(TypeError, lambda: det())
        self.assertRaises(TypeError, lambda: det(1))
        self.assertRaises(TypeError, lambda: det(self.v1))

    def test_trans(self):
        mat = EMatrix(EVector(0, 1, 4), EVector(0, 2, 5), EVector(0, 3, 6))
        self.assertEqual(transpose(self.m1), mat)
        self.assertRaises(TypeError, lambda: det(self.m1, self.m2))
        self.assertRaises(TypeError, lambda: det())
        self.assertRaises(TypeError, lambda: det(1))
        self.assertRaises(TypeError, lambda: det(self.v1))

    def test_init(self):
        mat = EMatrix(EVector(1, 2, 3))
        ans = EMatrix(self.v1, self.v0, self.v0)
        self.assertEqual(mat, ans)
        self.assertRaises(TypeError, EMatrix, 1, 2, 3)

    def test_compare(self):
        mat = self.m1 * 1
        self.assertTrue((mat == self.m1))
        self.assertFalse((mat == self.m2))
        self.assertTrue((mat != self.m2))
        self.assertFalse((mat != self.m1))
        self.assertRaises(TypeError, lambda: (self.m1 < self.m2))
        self.assertRaises(TypeError, lambda: (self.m1 <= self.m2))
        self.assertRaises(TypeError, lambda: (self.m1 > self.m2))
        self.assertRaises(TypeError, lambda: (self.m1 >= self.m2))
        self.assertRaises(TypeError, lambda: (self.m1 == 1))
        self.assertRaises(TypeError, lambda: (self.m1 == 1.1))
        self.assertRaises(TypeError, lambda: (self.m1 != 1))
        self.assertRaises(TypeError, lambda: (self.m1 != 1.1))

    def test_identity(self):
        self.assertEqual(EMatrix.I.get(0, 0), 1)
        self.assertEqual(EMatrix.I.get(1, 1), 1)
        self.assertEqual(EMatrix.I.get(2, 2), 1)
        mat = EMatrix(EVector(1, 0, 0), EVector(0, 1, 0), EVector(0, 0, 1))
        self.assertEqual(EMatrix.I, mat)
        
if __name__ == '__main__':
    unittest.main()
