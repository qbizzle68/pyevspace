from math import pi

from pyevspace import Vector, Matrix, dot, cross, norm, vang, vxcl, proj, det, transpose
import unittest


class Test_evspace(unittest.TestCase):

    v = Vector((1, 2, 3))
    v34 = Vector((3, 4, 0))
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))

    def test_module_dot(self):
        # test dot operator computations
        msg = 'vector dot product'
        self.assertEqual(dot(self.v, self.v34), 11, msg=msg)
        msg = 'vector dot equal to mag2()'
        self.assertEqual(dot(self.v, self.v), self.v.mag2(), msg=msg)

        # test exceptions from types
        msg = 'dot() argument number < 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            dot(self.v)

        msg = 'dot() argument number > 2 TypeError'
        with self.assertRaises(TypeError, msg=msg):
            dot(self.v, self.v, self.v)

        msg = 'dot() lhs argument TypeError'
        with self.assertRaises(TypeError, msg=msg):
            dot(self.v, 0)

    def test_module_cross(self):
        # test cross operator computations
        msg = 'vector cross '
        self.assertEqual(cross(self.v, self.v34), Vector((-12, 9, -2)))
        self.assertEqual(cross(self.v34, self.v), Vector((12, -9, 2)))

        # test exception from types
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
        # test norm operator computations
        self.assertEqual(norm(self.v34), Vector((0.6, 0.8, 0)))
        self.assertEqual(norm(self.v34 * -1), Vector((-0.6, -0.8, 0)))
        v = Vector((1, 2, 3))
        v.normalize()
        self.assertEqual(norm(self.v), v)

        # test exception from types
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
        # test vang computation values
        self.assertAlmostEqual(vang(Vector((1, 1, 0)), Vector((1, 0, 0))), pi / 4, 6)
        self.assertAlmostEqual(vang(Vector((1, 0, 0)), Vector((0, 1, 0))), pi / 2, 6)

        # test exception from types
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
        # test vxcl computation values
        self.assertEqual(vxcl(Vector((1, 1, 0)), Vector((0, 1, 0))), Vector((1, 0, 0)))
        self.assertEqual(vxcl(Vector((1, 2, 0)), Vector((0, 1, 0))), Vector((1, 0, 0)))

        # test exception from types
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
        # test proj computation values
        self.assertEqual(proj(Vector((1, 1, 7)), Vector((1, 1, 1))), Vector((3, 3, 3)))
        self.assertEqual(proj(Vector.e1, Vector.e2), Vector())
        
        # test exception from types
        with self.assertRaises(TypeError) as cm:
            proj(3, 'a')
        self.assertEqual(TypeError, type(cm.exception))

        with self.assertRaises(TypeError) as cm:
            proj(Vector())
        self.assertEqual(TypeError, type(cm.exception))

    def test_det(self):
        # test det with non invertable matrix
        self.assertEqual(det(self.m), 0)

        # test det with invertable matrix
        m = Matrix((2, 6, 4), (7, 3, 1), (8, 0, 0))
        self.assertEqual(det(m), -48)

        # test exception from types
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
        # test transpose expected values
        ans = Matrix((1, 4, 7), (2, 5, 8), (3, 6, 9))
        self.assertEqual(transpose(self.m), ans)

        # test exception from types
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
