import sys
# this is the location of the pyevspace.pyd module
sys.path.append(r'C:\dev\libs\pyevspace\pyevspace\bin\x64')
from pyevspace import * 
import unittest
import math

class Test_evector_test(unittest.TestCase):
    
    v0 = EVector()
    v1 = EVector(1, 2, 3)
    v2 = EVector(1, 2, 3)
    v3 = EVector(4, 5, 6)

    def iadd(self, y):
        try:
            self.v1 += y
            self.fail()
        except:
            pass

    def test_add(self):
        self.assertEqual(self.v1 + self.v2, EVector(2, 4, 6))
        iplus = self.v1 * 1.0
        iplus += self.v2
        self.assertEqual(iplus, EVector(2, 4, 6))
        self.assertRaises(TypeError, lambda x: self.v1 + x, 1)
        self.assertRaises(TypeError, lambda x: self.v1 + x, 1.1)
        self.assertRaises(TypeError, self.iadd(1))
        self.assertRaises(TypeError, self.iadd(1.1))

    def isub(self, x, y):
        try:
            x -= y
            self.fail()
        except:
            pass

    def test_sub(self):
        self.assertEqual(self.v1 - self.v2, EVector())
        isub = self.v1 * 1.0
        isub -= self.v2
        self.assertRaises(TypeError, lambda x: self.v1 - x, 1)
        self.assertRaises(TypeError, lambda x: self.v1 - x, 1.1)
        self.assertRaises(TypeError, self.isub(self.v1, 1))
        self.assertRaises(TypeError, self.isub(self.v1, 1.1))

    def imult(self, x, y):
        try:
            x *= y
            self.fail()
        except:
            pass

    def test_mult(self):
        self.assertEqual(self.v1 * 1.1, EVector(1.1, 2.2, 3.3))
        self.assertEqual(self.v1 * 2, EVector(2, 4, 6))
        imult = self.v1 * 1.0
        imult *= 1.1
        self.assertEqual(imult, EVector(1.1, 2.2, 3.3))
        imult = self.v1 * 1.0
        imult *= 2
        self.assertEqual(imult, EVector(2, 4, 6))
        self.assertRaises(TypeError, lambda x: self.v1 * x, self.v1)
        self.assertRaises(TypeError, self.imult(self.v1, self.v1))

    def idiv(self, x, y):
        try:
            x /= y
            self.fail()
        except:
            pass

    def test_div(self):
        self.assertEqual(self.v1 / 0.5, EVector(2, 4, 6))
        self.assertEqual(self.v1 / 2, EVector(0.5, 1, 1.5))
        idiv = self.v1 * 1.0
        idiv /= 0.5
        self.assertEqual(idiv, EVector(2, 4, 6))
        idiv = self.v1 * 1.0
        idiv /= 2
        self.assertEqual(idiv, EVector(0.5, 1, 1.5))
        self.assertRaises(TypeError, lambda x: self.v1 / x, self.v1)
        self.assertRaises(TypeError, self.idiv(self.v1, self.v1))

    def test_neg(self):
        self.assertEqual(-self.v1, EVector(-1, -2, -3))

    def test_abs(self):
        absval = abs(EVector(3, 4))
        self.assertEqual(absval, 5)
        absval = abs(EVector(0, 3, 4))
        self.assertEqual(absval, 5)

    def test_mag(self):
        self.assertEqual(EVector(3, 4).mag(), 5)
        self.assertEqual(EVector(0, 3, 4).mag(), 5)
        self.assertEqual(self.v1.mag2(), 14)
        self.assertEqual(self.v3.mag2(), 77)

    def test_norm(self):
        self.assertEqual(norm(EVector(3, 4)), EVector(.6, .8, 0))
        n = EVector(3, 4)
        n.normalize()
        self.assertEqual(n, EVector(.6, .8, 0))
        self.assertRaises(TypeError, lambda x, y: norm(x, y), self.v1, self.v2)
        self.assertRaises(TypeError, lambda x: norm(x), None)
        self.assertRaises(TypeError, lambda x: norm(x), 2.4)

    def test_dot(self):
        self.assertEqual(dot(self.v1, self.v1), 14)
        self.assertRaises(TypeError, lambda x: dot(x), self.v1)
        self.assertRaises(TypeError, lambda x, y, z: dot(x, y, z), self.v1, self.v2, self.v3)
        self.assertRaises(TypeError, lambda x, y: dot(x, y), self.v1, None)
        self.assertRaises(TypeError, lambda x, y: dot(x, y), None, None)
        self.assertRaises(TypeError, lambda x, y: dot(x, y), self.v1, 1.4)
        self.assertRaises(TypeError, lambda x, y: dot(x, y), 1.4, self.v1)

    def test_cross(self):
        self.assertEqual(cross(self.v1, self.v1), EVector(0, 0, 0))
        self.assertEqual(cross(self.v1, EVector(1, 1, 1)), EVector(-1, 2, -1))
        self.assertRaises(TypeError, lambda x: cross(x), self.v1)
        self.assertRaises(TypeError, lambda x, y, z: cross(x, y, z), self.v1, self.v2, self.v3)
        self.assertRaises(TypeError, lambda x, y: cross(x, y), self.v1, None)
        self.assertRaises(TypeError, lambda x, y: cross(x, y), None, None)
        self.assertRaises(TypeError, lambda x, y: cross(x, y), self.v1, 1.4)
        self.assertRaises(TypeError, lambda x, y: cross(x, y), 1.4, self.v1)

    def test_vang(self):
        self.assertAlmostEqual(vang(EVector(1, 1, 0), EVector(1, 0, 0)), 45)
        self.assertEqual(vang(EVector(1, 0, 0), EVector(0, 1, 0)), 90)
        self.assertRaises(TypeError, lambda x: vang(x), self.v1)
        self.assertRaises(TypeError, lambda x, y, z: vang(x, y, z), self.v1, self.v2, self.v3)
        self.assertRaises(TypeError, lambda x, y: vang(x, y), self.v1, None)
        self.assertRaises(TypeError, lambda x, y: vang(x, y), None, None)
        self.assertRaises(TypeError, lambda x, y: vang(x, y), self.v1, 1.4)
        self.assertRaises(TypeError, lambda x, y: vang(x, y), 1.4, self.v1)

    def test_vxcl(self):
        self.assertEqual(vxcl(EVector(1, 1, 1), EVector(1, 0, 1)), EVector(0, 1, 0))
        self.assertEqual(vxcl(EVector(1, 1, 0), EVector(1)), EVector(0, 1, 0))
        self.assertRaises(TypeError, lambda x: vxcl(x), self.v1)
        self.assertRaises(TypeError, lambda x, y, z: vxcl(x, y, z), self.v1, self.v2, self.v3)
        self.assertRaises(TypeError, lambda x, y: vxcl(x, y), self.v1, None)
        self.assertRaises(TypeError, lambda x, y: vxcl(x, y), None, None)
        self.assertRaises(TypeError, lambda x, y: vxcl(x, y), self.v1, 1.4)
        self.assertRaises(TypeError, lambda x, y: vxcl(x, y), 1.4, self.v1)

    def test_len(self):
        self.assertEqual(len(self.v1), 3)

    def test_get(self):
        self.assertEqual(self.v1[0], 1)
        self.assertEqual(self.v1[1], 2)
        self.assertEqual(self.v1[2], 3)
        self.assertEqual(self.v1[-1], 3)
        self.assertEqual(self.v1[-2], 2)
        self.assertEqual(self.v1[-3], 1)
        self.assertRaises(ValueError, lambda x: x[3], self.v1)

    def setv(self, x, y, z):
        try:
            x[y] = z
            self.fail()
        except:
            pass

    def test_set(self):
        vec = self.v1 * 1
        vec[0] = 8
        vec[1] = 7
        vec[2] = 6
        self.assertEqual(vec, EVector(8, 7, 6))
        vec[-1] = 4
        vec[-2] = 5
        vec[-3] = 6 
        self.assertEqual(vec, EVector(6, 5, 4))
        self.assertRaises(ValueError, self.v1.__setitem__, 3, 1)
        self.assertRaises(TypeError, self.setv(self.v1, 0, self.v1))

    def test_init(self):
        vec = EVector(3)
        self.assertEqual(vec, EVector(3, 0, 0))
        vec = EVector(2, 4)
        self.assertEqual(vec, EVector(2, 4, 0))
        vec = EVector(x = 3, y = 8, z = 5)
        self.assertEqual(vec, EVector(3, 8, 5))
        vec = EVector(y = 8, z = 3)
        self.assertEqual(vec, EVector(0, 8, 3))
        vec = EVector(5, y = 2)
        self.assertEqual(vec, EVector(5, 2, 0))

    def test_str(self):
        self.assertEqual(str(self.v1), "[ 1.000000, 2.000000, 3.000000 ]")
        
    def test_compare(self):
        self.assertTrue((self.v1 == self.v2))
        self.assertFalse((self.v1 != self.v2))
        self.assertFalse((self.v1 == self.v0))
        self.assertTrue((self.v1 != self.v3))
        self.assertRaises(TypeError, lambda x, y: (x < y), self.v1, self.v2)
        self.assertRaises(TypeError, lambda x, y: (x <= y), self.v1, self.v2)
        self.assertRaises(TypeError, lambda x, y: (x > y), self.v1, self.v2)
        self.assertRaises(TypeError, lambda x, y: (x >= y), self.v1, self.v2)
        self.assertRaises(TypeError, lambda x: (self.v1 == x), 2)
        self.assertRaises(TypeError, lambda x: (self.v1 == x), 2.3)

    def test_elementary(self):
        self.assertEqual(EVector.e1[0], 1)
        self.assertEqual(EVector.e2[1], 1)
        self.assertEqual(EVector.e3[2], 1)
        self.assertEqual(EVector.e1, EVector(1, 0, 0))
        self.assertEqual(EVector.e2, EVector(0, 1, 0))
        self.assertEqual(EVector.e3, EVector(0, 0, 1))

if __name__ == '__main__':
    unittest.main()
