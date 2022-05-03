import sys
# this is the location of the pyevspace.pyd module
sys.path.append(r'C:\dev\libs\pyevspace\pyevspace\bin\x64')
from pyevspace import *

v0 = EVector();
v1 = EVector(1, 2, 3)
v2 = EVector(1, 2, 3)
v3 = EVector(4, 5, 6)
m0 = EMatrix()
m1 = EMatrix(v0, v1, v3)
m2 = EMatrix(v1, v3, v2)
    
print("get")
print(m1.get(0, 0)== 0)
print(m1.get(0, 1)== 1)
print(m1.get(0, 2)== 4)
print(m1.get(1, 0)== 0)
print(m1.get(1, 1)== 2)
print(m1.get(1, 2)== 5)
print(m1.get(2, 0)== 0)
print(m1.get(2, 1)== 3)
print(m1.get(2, 2)== 6)

print("set")
mat = EMatrix()
mat.set(0, 1, 34)
print(mat.get(0, 1)== 34)
mat.set(2, 2, -4)
print(mat.get(2, 2)== -4)

print("equality")
mat = EMatrix(v0, v1, v3)
print(m1== mat)
print(m2== mat)
        
print("add")
mplus = m1 + m2
ans = EMatrix(EVector(1, 2, 3), EVector(5, 7, 9), EVector(5, 7, 9))
print(mplus== ans)
miplus = EMatrix(v0, v1, v3)
miplus += m2
print(miplus== ans)
# bad argument types
try:
    m1 + 1
except:
    print("error adding 1")
try:
    m1 + 1.1
except:
    print("error adding 1.1")
try:
    m1 + v1
except:
    print("error adding v1")

print("sub")
msub = m1 - m2
ans = EMatrix(EVector(-1, -2, -3), EVector(-3, -3, -3), EVector(3, 3, 3))
print(msub== ans)
misub = EMatrix(v0, v1, v3)
misub -= m2
print(misub== ans)
# bad argument types
try:
    m1 - 1
except:
    print("error subtracting 1")
try:
    m1 - 1.1
except:
    print("error subtracting 1.1")
try:
    m1 - v1
except:
    print("error subtracting v1")

print("mult")
# matrix
mmultm = m1 * m2
ansm = EMatrix(EVector(14, 19, 24), EVector(29, 40, 51), EVector(14, 19, 24))
print(mmultm== ansm)
# vector
mmultv = m1 * v1
ansv = EVector(14, 19, 24)
print(mmultv== ansv)
# double
mmultd = m1 * 2.5
ansd = EMatrix(v0 * 2.5, v1 * 2.5, v3 * 2.5)
print(mmultd== ansd)
# inplace
# matrix
mimultm = m1 * 1
mimultm *= m2
print(mimultm== ansm)
# double
mimultd = m1 * 1
mimultd *= 2.5
print(mimultd== ansd)
try:
    m1 * "asdf"
except:
    print("error multiplying string")

print("div")
mat = m1 / 0.5
ans = EMatrix(v0, v1 * 2, v3 * 2)
print(mat== ans)
mat /= 2
print(mat== m1)
mat2 = mat * 2
mat3 = mat2 / 2
print(mat3== m1)
try:
    m1 / m2
except:
    print("error dividing m2")
try:
    m1 / v1
except:
    print("error dividing v1")

    '''def test_neg(self):
        mat = -self.m1
        ans = EMatrix(-self.v0, -self.v1, -self.v3)
        self.assertEqual(mat, ans, "negate operator")

    def test_det(self):
        mat = EMatrix(EVector(2, 6, 1), EVector(4, 7, 5), EVector(8, 4, 6))
        self.assertEqual(mat.det(), 100, "determinate")

    def test_transpose(self):
        mat = self.m1.transpose()
        ans = EMatrix(EVector(0, 1, 4), EVector(0, 2, 5), EVector(0, 3, 6))
        self.assertEqual(mat, ans, "transpose")'''
