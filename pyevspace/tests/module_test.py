from math import pi

from pyevspace import *
import unittest


class Test_evspace(unittest.TestCase):

    v = Vector((1, 2, 3))
    v34 = Vector((3, 4, 0))
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))

    def test_dot(self):
        self.assertEqual(dot(self.v, self.v34), 11)
        self.assertEqual(dot(self.v, self.v), self.v.mag2())

        with self.assertRaises(TypeError) as cm:
            dot(self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            dot(self.v, self.v, self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            dot(self.v, 0)
        self.assertEqual(TypeError, type(cm.exception))

    def test_cross(self):
        self.assertEqual(cross(self.v, self.v34), Vector((-12, 9, -2)))
        self.assertEqual(cross(self.v34, self.v), Vector((12, -9, 2)))

        with self.assertRaises(TypeError) as cm:
            cross(self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            cross(self.v, self.v, self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            cross(self.v, 1.0)
        self.assertEqual(TypeError, type(cm.exception))

    def test_norm(self):
        self.assertEqual(norm(self.v34), Vector((0.6, 0.8, 0)))
        self.assertEqual(norm(self.v34 * -1), Vector((-0.6, -0.8, 0)))
        v = Vector((1, 2, 3))
        v.normalize()
        self.assertEqual(norm(self.v), v)

        with self.assertRaises(TypeError) as cm:
            norm()
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            norm(self.v, self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            norm(1)
        self.assertEqual(TypeError, type(cm.exception))

    def test_vang(self):
        self.assertAlmostEqual(vang(Vector((1, 1, 0)), Vector((1, 0, 0))), pi / 4, 6)
        self.assertAlmostEqual(vang(Vector((1, 0, 0)), Vector((0, 1, 0))), pi / 2, 6)

        with self.assertRaises(TypeError) as cm:
            vang(self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            vang(self.v, self.v, self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            vang(2)
        self.assertEqual(TypeError, type(cm.exception))

    def test_vxcl(self):
        self.assertEqual(vxcl(Vector((1, 1, 0)), Vector((0, 1, 0))), Vector((1, 0, 0)))
        self.assertEqual(vxcl(Vector((1, 2, 0)), Vector((0, 1, 0))), Vector((1, 0, 0)))

        with self.assertRaises(TypeError) as cm:
            vang(self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            vang(self.v, self.v, self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            vang(5)
        self.assertEqual(TypeError, type(cm.exception))

    def test_proj(self):
        self.assertEqual(proj(Vector((1, 1, 7)), Vector((1, 1, 1))), Vector((3, 3, 3)))
        self.assertEqual(proj(Vector.e1, Vector.e2), Vector())
        
        with self.assertRaises(TypeError) as cm:
            proj(3, 'a')
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            proj(Vector())
        self.assertEqual(TypeError, type(cm.exception))

    def test_det(self):
        self.assertEqual(det(self.m), 0)
        m = Matrix((2, 6, 4), (7, 3, 1), (8, 0, 0))
        self.assertEqual(det(m), -48)

        with self.assertRaises(TypeError) as cm:
            det()
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            det(self.v, self.v)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            det(7)
        self.assertEqual(TypeError, type(cm.exception))

    def test_transpose(self):
        ans = Matrix((1, 4, 7), (2, 5, 8), (3, 6, 9))
        self.assertEqual(transpose(self.m), ans)

        with self.assertRaises(TypeError) as cm:
            transpose()
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            transpose(self.m, self.m)
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            transpose(5)
        self.assertEqual(TypeError, type(cm.exception))


if __name__ == '__main__':
    unittest.main()
