from math import sin, cos, pi
from pyevspace import Matrix, Vector, X_AXIS, Y_AXIS, Z_AXIS, XYZ, XZY, YXZ, \
    YZX, ZXY, ZYX, XYX, XZX, YXY, YZY, ZXZ, ZYZ, Order, rotateOffsetFrom, \
    rotateOffsetTo, Angles, getMatrixEuler, rotateMatrixFrom

so = cos(pi / 4)  # cos and sin of forty-five

rotation_xaxis_answers = [
    Matrix((1, 0, 0),  # X-rotation, 0.0 radians
           (0, 1, 0),
           (0, 0, 1)),
    Matrix((1, 0, 0),  # X-rotation, pi / 4 radians
           (0, so, -so),
           (0, so, so)),
    Matrix((1, 0, 0),  # X-rotation, pi / 2 radians
           (0, 0, -1),
           (0, 1, 0)),
    Matrix((1, 0, 0),  # X-rotation, 3 pi / 4 radians
           (0, -so, -so),
           (0, so, -so)),
    Matrix((1, 0, 0),  # X-rotation, pi radians
           (0, -1, 0),
           (0, 0, -1)),
    Matrix((1, 0, 0),  # X-rotation, 5 pi / 4 radians
           (0, -so, so),
           (0, -so, -so)),
    Matrix((1, 0, 0),  # X-rotation, 3 pi / 2 radians
           (0, 0, 1),
           (0, -1, 0)),
    Matrix((1, 0, 0),  # X-rotation, 7 pi / 4 radians
           (0, so, so),
           (0, -so, so))
]

rotation_yaxis_answers = [
    Matrix((1, 0, 0),  # Y-rotation, 0.0 radians
           (0, 1, 0),
           (0, 0, 1)),
    Matrix((so, 0, so),  # Y-rotation, pi / 4 radians
           (0, 1, 0),
           (-so, 0, so)),
    Matrix((0, 0, 1),  # Y-rotation, pi / 2 radians
           (0, 1, 0),
           (-1, 0, 0)),
    Matrix((-so, 0, so),  # Y-rotation, 3 pi / 4 radians
           (0, 1, 0),
           (-so, 0, -so)),
    Matrix((-1, 0, 0),  # Y-rotation, pi radians
           (0, 1, 0),
           (0, 0, -1)),
    Matrix((-so, 0, -so),  # Y-rotation, 5 pi / 4 radians
           (0, 1, 0),
           (so, 0, -so)),
    Matrix((0, 0, -1),  # Y-rotation, 3 pi / 2 radians
           (0, 1, 0),
           (1, 0, 0)),
    Matrix((so, 0, -so),  # Y-rotation, 7 pi / 4 radians
           (0, 1, 0),
           (so, 0, so))
]

rotation_zaxis_answers = [
    Matrix((1, 0, 0),  # Z-rotation, 0 radians
           (0, 1, 0),
           (0, 0, 1)),
    Matrix((so, -so, 0),  # Z-rotation, pi / 4 radians
           (so, so, 0),
           (0, 0, 1)),
    Matrix((0, -1, 0),  # Z-rotation, pi / 2 radians
           (1, 0, 0),
           (0, 0, 1)),
    Matrix((-so, -so, 0),  # Z-rotation, 3 pi / 4 radians
           (so, -so, 0),
           (0, 0, 1)),
    Matrix((-1, 0, 0),  # Z-rotation, pi radians
           (0, -1, 0),
           (0, 0, 1)),
    Matrix((-so, so, 0),  # Z-rotation, 5 pi / 4 radians
           (-so, -so, 0),
           (0, 0, 1)),
    Matrix((0, 1, 0),  # Z-rotation, 3 pi / 2 radians
           (-1, 0, 0),
           (0, 0, 1)),
    Matrix((so, so, 0),  # Z-rotation, 7 pi / 4 radians
           (-so, so, 0),
           (0, 0, 1))
]

rotation_matrix_answers = {
    X_AXIS: rotation_xaxis_answers,
    Y_AXIS: rotation_yaxis_answers,
    Z_AXIS: rotation_zaxis_answers
}

half = 0.5
ef = cos(pi / 4) * sin(pi / 4) * (1 + sin(pi / 4))  # 0.8535534
of = cos(pi / 4) * cos(pi / 4) * (1 - cos(pi / 4))  # 0.1464466

rotation_XYZ_answers = [
    Matrix.id,  # 0 radians
    Matrix((half, -half, so),  # pi / 4 radians
           (ef, of, -half),
           (of, ef, half)),
    Matrix((0, 0, 1),  # pi / 2 radians
           (0, -1, 0),
           (1, 0, 0)),
    Matrix((half, half, so),  # 3 pi / 2 radians
           (-ef, of, half),
           (of, -ef, half)),
    Matrix((1, 0, 0),  # pi radians
           (0, 1, 0),
           (0, 0, 1)),
    Matrix((half, -half, -so),  # 5 pi / 4 radians
           (of, ef, -half),
           (ef, of, half)),
    Matrix((0, 0, -1),  # 3 pi / 2 radians
           (0, 1, 0),
           (1, 0, 0)),
    Matrix((half, half, -so),  # 7 pi / 4 radians
           (-of, ef, half),
           (ef, -of, half))
]

rotation_XZY_answers = [
    Matrix.id,  # 0 radians
    Matrix((half, -so, half),  # pi / 4 radians
           (ef, half, -of),
           (-of, half, ef)),
    Matrix((0, -1, 0),  # pi / 2 radians
           (1, 0, 0),
           (0, 0, 1)),
    Matrix((half, -so, -half),  # 3 pi / 4 radians
           (ef, half, of),
           (of, -half, ef)),
    Matrix.id,  # pi radians
    Matrix((half, so, half),  # 5 pi / 4 radians
           (of, half, -ef),
           (-ef, half, of)),
    Matrix((0, 1, 0),  # 3 pi / 2 radians
           (1, 0, 0),
           (0, 0, -1)),
    Matrix((half, so, -half),  # 7 pi / 4 radians
           (of, half, ef),
           (ef, -half, of))
]

rotation_YXZ_answers = [
    Matrix.id,  # 0 radians
    Matrix((ef, -of, half),  # pi / 4 radians
           (half, half, -so),
           (-of, ef, half)),
    Matrix((1, 0, 0),  # pi / 2 radians
           (0, 0, -1),
           (0, 1, 0)),
    Matrix((ef, of, -half),  # 3 pi / 4 radians
           (-half, half, -so),
           (of, ef, half)),
    Matrix.id,  # pi radians
    Matrix((of, -ef, half),  # 5 pi / 4 radians
           (half, half, so),
           (-ef, of, half)),
    Matrix((-1, 0, 0),  # 3 pi / 2 radians
           (0, 0, 1),
           (0, 1, 0)),
    Matrix((of, ef, -half),  # 7 pi / 4 radians
           (-half, half, so),
           (ef, of, half))
]

rotation_YZX_answers = [
    Matrix.id,  # 0 radians
    Matrix((half, of, ef),  # pi / 4 radians
           (so, half, -half),
           (-half, ef, of)),
    Matrix((0, 1, 0),  # pi / 2 radians
           (1, 0, 0),
           (0, 0, -1)),
    Matrix((half, of, -ef),  # 3 pi / 4 radians
           (so, half, half),
           (half, -ef, of)),
    Matrix.id,  # pi radians
    Matrix((half, ef, of),  # 5 pi / 4 radians
           (-so, half, -half),
           (-half, of, ef)),
    Matrix((0, 1, 0),  # 3 pi / 2 radians
           (-1, 0, 0),
           (0, 0, 1)),
    Matrix((half, ef, -of),  # 7 pi / 4 radians
           (-so, half, half),
           (half, -of, ef))
]

rotation_ZXY_answers = [
    Matrix.id,  # 0 radians
    Matrix((of, -half, ef),  # pi / 4 radians
           (ef, half, of),
           (-half, so, half)),
    Matrix((-1, 0, 0),  # pi / 2 radians
           (0, 0, 1),
           (0, 1, 0)),
    Matrix((of, half, -ef),  # 3 pi / 4 radians
           (-ef, half, of),
           (half, so, half)),
    Matrix.id,  # pi radians
    Matrix((ef, -half, of),  # 5 pi / 4 radians
           (of, half, ef),
           (-half, -so, half)),
    Matrix((1, 0, 0),  # 3 pi / 2 radians
           (0, 0, 1),
           (0, -1, 0)),
    Matrix((ef, half, -of),  # 7 pi / 4 radians
           (-of, half, ef),
           (half, -so, half))
]

rotation_ZYX_answers = [
    Matrix.id,  # 0 radians
    Matrix((half, -of, ef),  # pi / 4 radians
           (half, ef, -of),
           (-so, half, half)),
    Matrix((0, 0, 1),  # pi / 2 radians
           (0, 1, 0),
           (-1, 0, 0)),
    Matrix((half, of, ef),  # 3 pi / 4 radians
           (-half, ef, of),
           (-so, -half, half)),
    Matrix.id,  # pi radians
    Matrix((half, -ef, of),  # 5 pi / 4 radians
           (half, of, -ef),
           (so, half, half)),
    Matrix((0, 0, 1),  # 3 pi / 2 radians
           (0, -1, 0),
           (1, 0, 0)),
    Matrix((half, ef, of),  # 7 pi / 4 radians
           (-half, of, ef),
           (so, -half, half))
]

rotation_XYX_answers = [
    Matrix.id,  # 0 radians
    Matrix((so, half, half),  # pi / 4 radians
           (half, of, -ef),
           (-half, ef, -of)),
    Matrix((0, 1, 0),  # pi / 2 radians
           (1, 0, 0),
           (0, 0, -1)),
    Matrix((-so, half, -half),  # 3 pi / 4 radians
           (half, ef, of),
           (half, -of, -ef)),
    Matrix((-1, 0, 0),  # pi radians
           (0, 1, 0),
           (0, 0, -1)),
    Matrix((-so, half, half),  # 5 pi / 4 radians
           (half, ef, -of),
           (-half, of, -ef)),
    Matrix((0, 1, 0),  # 3 pi / 2 radians
           (1, 0, 0),
           (0, 0, -1)),
    Matrix((so, half, -half),  # 7 pi / 4 radians
           (half, of, ef),
           (half, -ef, -of))
]

rotation_XZX_answers = [
    Matrix.id,  # 0 radians
    Matrix((so, -half, half),  # pi / 4 radians
           (half, -of, -ef),
           (half, ef, of)),
    Matrix((0, 0, 1),  # pi / 2 radians
           (0, -1, 0),
           (1, 0, 0)),
    Matrix((-so, half, half),  # 3 pi / 4 radians
           (-half, -ef, of),
           (half, -of, ef)),
    Matrix((-1, 0, 0),  # pi radians
           (0, -1, 0),
           (0, 0, 1)),
    Matrix((-so, -half, half),  # 5 pi / 4 radians
           (half, -ef, -of),
           (half, of, ef)),
    Matrix((0, 0, 1),  # 3 pi / 2 radians
           (0, -1, 0),
           (1, 0, 0)),
    Matrix((so, half, half),  # 7 pi / 4 radians
           (-half, -of, ef),
           (half, -ef, of))
]

rotation_YXY_answers = [
    Matrix.id,  # 0 radians
    Matrix((of, half, ef),  # pi / 4 radians
           (half, so, -half),
           (-ef, half, -of)),
    Matrix((0, 1, 0),  # pi / 2 radians
           (1, 0, 0),
           (0, 0, -1)),
    Matrix((ef, half, -of),  # 3 pi / 4 radians
           (half, -so, half),
           (of, -half, -ef)),
    Matrix((1, 0, 0),  # pi radians
           (0, -1, 0),
           (0, 0, -1)),
    Matrix((ef, half, of),  # 5 pi / 4 radians
           (half, -so, -half),
           (-of, half, -ef)),
    Matrix((0, 1, 0),  # 3 pi / 2 radians
           (1, 0, 0),
           (0, 0, -1)),
    Matrix((of, half, -ef),  # 7 pi / 4 radians
           (half, so, half),
           (ef, -half, -of))
]

rotation_YZY_answers = [
    Matrix.id,  # 0 radians
    Matrix((-of, -half, ef),  # pi / 4 radians
           (half, so, half),
           (-ef, half, of)),
    Matrix((-1, 0, 0),  # pi / 2 radians
           (0, 0, 1),
           (0, 1, 0)),
    Matrix((-ef, half, -of),  # 3 pi / 4 radians
           (-half, -so, half),
           (of, half, ef)),
    Matrix((-1, 0, 0),  # pi radians
           (0, -1, 0),
           (0, 0, 1)),
    Matrix((-ef, -half, of),  # 5 pi / 4 radians
           (half, -so, half),
           (-of, half, ef)),
    Matrix((-1, 0, 0),  # 3 pi / 2 radians
           (0, 0, 1),
           (0, 1, 0)),
    Matrix((-of, half, -ef),  # 7 pi / 4 radians
           (-half, so, half),
           (ef, half, of))
]

rotation_ZXZ_answers = [
    Matrix.id,  # 0 radians
    Matrix((of, -ef, half),  # pi / 4 radians
           (ef, -of, -half),
           (half, half, so)),
    Matrix((0, 0, 1),  # pi / 2 radians
           (0, -1, 0),
           (1, 0, 0)),
    Matrix((ef, of, half),  # 3 pi / 4 radians
           (-of, -ef, half),
           (half, -half, -so)),
    Matrix((1, 0, 0),  # pi radians
           (0, -1, 0),
           (0, 0, -1)),
    Matrix((ef, -of, half),  # 5 pi / 4 radians
           (of, -ef, -half),
           (half, half, -so)),
    Matrix((0, 0, 1),  # 3 pi / 2 radians
           (0, -1, 0),
           (1, 0, 0)),
    Matrix((of, ef, half),  # 7 pi / 4 radians
           (-ef, -of, half),
           (half, -half, so))
]

rotation_ZYZ_answers = [
    Matrix.id,  # 0 radians
    Matrix((-of, -ef, half),  # pi / 4 radians
           (ef, of, half),
           (-half, half, so)),
    Matrix((-1, 0, 0),  # pi / 2 radians
           (0, 0, 1),
           (0, 1, 0)),
    Matrix((-ef, of, -half),  # 3 pi / 4 radians
           (-of, ef, half),
           (half, half, -so)),
    Matrix((-1, 0, 0),  # pi radians
           (0, 1, 0),
           (0, 0, -1)),
    Matrix((-ef, -of, half),  # 5 pi / 4 radians
           (of, ef, half),
           (-half, half, -so)),
    Matrix((-1, 0, 0),  # 3 pi / 2 radians
           (0, 0, 1),
           (0, 1, 0)),
    Matrix((-of, ef, -half),  # 7 pi / 4 radians
           (-ef, of, half),
           (half, half, so))
]

rotation_euler_answers = {
    XYZ: rotation_XYZ_answers,
    XZY: rotation_XZY_answers,
    YXZ: rotation_YXZ_answers,
    YZX: rotation_YZX_answers,
    ZXY: rotation_ZXY_answers,
    ZYX: rotation_ZYX_answers,
    XYX: rotation_XYX_answers,
    XZX: rotation_XZX_answers,
    YXY: rotation_YXY_answers,
    YZY: rotation_YZY_answers,
    ZXZ: rotation_ZXZ_answers,
    ZYZ: rotation_ZYZ_answers
}

rotation_axis_to_answers = {
    X_AXIS:
        {
            X_AXIS: Vector.e1,
            Y_AXIS: -Vector.e3,
            Z_AXIS: Vector.e2
        },
    Y_AXIS:
        {
            X_AXIS: Vector.e3,
            Y_AXIS: Vector.e2,
            Z_AXIS: -Vector.e1
        },
    Z_AXIS:
        {
            X_AXIS: -Vector.e2,
            Y_AXIS: Vector.e1,
            Z_AXIS: Vector.e3
        }
}

rotation_axis_from_answers = {
    X_AXIS:
        {
            X_AXIS: Vector.e1,
            Y_AXIS: Vector.e3,
            Z_AXIS: -Vector.e2
        },
    Y_AXIS:
        {
            X_AXIS: -Vector.e3,
            Y_AXIS: Vector.e2,
            Z_AXIS: Vector.e1
        },
    Z_AXIS:
        {
            X_AXIS: Vector.e2,
            Y_AXIS: -Vector.e1,
            Z_AXIS: Vector.e3
        }
}

rotation_euler_to_answers = {
    XYZ:
        {
            X_AXIS: Vector.e3,
            Y_AXIS: -Vector.e2,
            Z_AXIS: Vector.e1
        },
    XZY:
        {
            X_AXIS: -Vector.e2,
            Y_AXIS: Vector.e1,
            Z_AXIS: Vector.e3
        },
    YXZ:
        {
            X_AXIS: Vector.e1,
            Y_AXIS: -Vector.e3,
            Z_AXIS: Vector.e2
        },
    YZX:
        {
            X_AXIS: Vector.e2,
            Y_AXIS: Vector.e1,
            Z_AXIS: -Vector.e3
        },
    ZXY:
        {
            X_AXIS: -Vector.e1,
            Y_AXIS: Vector.e3,
            Z_AXIS: Vector.e2
        },
    ZYX:
        {
            X_AXIS: Vector.e3,
            Y_AXIS: Vector.e2,
            Z_AXIS: -Vector.e1
        },
    XYX:
        {
            X_AXIS: Vector.e2,
            Y_AXIS: Vector.e1,
            Z_AXIS: -Vector.e3
        },
    XZX:
        {
            X_AXIS: Vector.e3,
            Y_AXIS: -Vector.e2,
            Z_AXIS: Vector.e1
        },
    YXY:
        {
            X_AXIS: Vector.e2,
            Y_AXIS: Vector.e1,
            Z_AXIS: -Vector.e3
        },
    YZY:
        {
            X_AXIS: -Vector.e1,
            Y_AXIS: Vector.e3,
            Z_AXIS: Vector.e2
        },
    ZXZ:
        {
            X_AXIS: Vector.e3,
            Y_AXIS: -Vector.e2,
            Z_AXIS: Vector.e1
        },
    ZYZ:
        {
            X_AXIS: -Vector.e1,
            Y_AXIS: Vector.e3,
            Z_AXIS: Vector.e2
        }
}

rotation_euler_from_answers = {
    XYZ:
        {
            X_AXIS: Vector.e3,
            Y_AXIS: -Vector.e2,
            Z_AXIS: Vector.e1
        },
    XZY:
        {
            X_AXIS: Vector.e2,
            Y_AXIS: -Vector.e1,
            Z_AXIS: Vector.e3
        },
    YXZ:
        {
            X_AXIS: Vector.e1,
            Y_AXIS: Vector.e3,
            Z_AXIS: -Vector.e2
        },
    YZX:
        {
            X_AXIS: Vector.e2,
            Y_AXIS: Vector.e1,
            Z_AXIS: -Vector.e3
        },
    ZXY:
        {
            X_AXIS: -Vector.e1,
            Y_AXIS: Vector.e3,
            Z_AXIS: Vector.e2
        },
    ZYX:
        {
            X_AXIS: -Vector.e3,
            Y_AXIS: Vector.e2,
            Z_AXIS: Vector.e1
        },
    XYX:
        {
            X_AXIS: Vector.e2,
            Y_AXIS: Vector.e1,
            Z_AXIS: -Vector.e3
        },
    XZX:
        {
            X_AXIS: Vector.e3,
            Y_AXIS: -Vector.e2,
            Z_AXIS: Vector.e1
        },
    YXY:
        {
            X_AXIS: Vector.e2,
            Y_AXIS: Vector.e1,
            Z_AXIS: -Vector.e3
        },
    YZY:
        {
            X_AXIS: -Vector.e1,
            Y_AXIS: Vector.e3,
            Z_AXIS: Vector.e2
        },
    ZXZ:
        {
            X_AXIS: Vector.e3,
            Y_AXIS: -Vector.e2,
            Z_AXIS: Vector.e1
        },
    ZYZ:
        {
            X_AXIS: -Vector.e1,
            Y_AXIS: Vector.e3,
            Z_AXIS: Vector.e2
        }
}

rotation_offset_to_answers = {
    XYZ:
        {
            X_AXIS: Vector((-1, 1, 0)),
            Y_AXIS: Vector((-1, 0, -1)),
            Z_AXIS: Vector((0, 1, -1))
        },
    XZY:
        {
            X_AXIS: Vector((-1, 0, -1)),
            Y_AXIS: Vector((0, 1, -1)),
            Z_AXIS: Vector((-1, 1, 0))
        },
    YXZ:
        {
            X_AXIS: Vector((0, -1, 1)),
            Y_AXIS: Vector((-1, -1, 0)),
            Z_AXIS: Vector((-1, 0, 1))
        },
    YZX:
        {
            X_AXIS: Vector((-1, 0, 1)),
            Y_AXIS: Vector((0, -1, 1)),
            Z_AXIS: Vector((-1, -1, 0))
        },
    ZXY:
        {
            X_AXIS: Vector((0, -1, -1)),
            Y_AXIS: Vector((1, -1, 0)),
            Z_AXIS: Vector((1, 0, -1))
        },
    ZYX:
        {
            X_AXIS: Vector((1, -1, 0)),
            Y_AXIS: Vector((1, 0, -1)),
            Z_AXIS: Vector((0, -1, -1))
        },
    XYX:
        {
            X_AXIS: Vector((-1, 0, 1)),
            Y_AXIS: Vector((0, -1, 1)),
            Z_AXIS: Vector((-1, -1, 0))
        },
    XZX:
        {
            X_AXIS: Vector((-1, 1, 0)),
            Y_AXIS: Vector((-1, 0, -1)),
            Z_AXIS: Vector((0, 1, -1))
        },
    YXY:
        {
            X_AXIS: Vector((-1, 0, 1)),
            Y_AXIS: Vector((0, -1, 1)),
            Z_AXIS: Vector((-1, -1, 0))
        },
    YZY:
        {
            X_AXIS: Vector((0, -1, -1)),
            Y_AXIS: Vector((1, -1, 0)),
            Z_AXIS: Vector((1, 0, -1))
        },
    ZXZ:
        {
            X_AXIS: Vector((-1, 1, 0)),
            Y_AXIS: Vector((-1, 0, -1)),
            Z_AXIS: Vector((0, 1, -1))
        },
    ZYZ:
        {
            X_AXIS: Vector((0, -1, -1)),
            Y_AXIS: Vector((1, -1, 0)),
            Z_AXIS: Vector((1, 0, -1))
        }
}

rotation_offset_from_answers = {
    XYZ:
        {
            X_AXIS: Vector((1, 1, 2)),
            Y_AXIS: Vector((1, 0, 1)),
            Z_AXIS: Vector((2, 1, 1))
        },
    XZY:
        {
            X_AXIS: Vector((1, 2, 1)),
            Y_AXIS: Vector((0, 1, 1)),
            Z_AXIS: Vector((1, 1, 2))
        },
    YXZ:
        {
            X_AXIS: Vector((2, 1, 1)),
            Y_AXIS: Vector((1, 1, 2)),
            Z_AXIS: Vector((1, 0, 1))
        },
    YZX:
        {
            X_AXIS: Vector((1, 2, 1)),
            Y_AXIS: Vector((2, 1, 1)),
            Z_AXIS: Vector((1, 1, 0))
        },
    ZXY:
        {
            X_AXIS: Vector((0, 1, 1)),
            Y_AXIS: Vector((1, 1, 2)),
            Z_AXIS: Vector((1, 2, 1))
        },
    ZYX:
        {
            X_AXIS: Vector((1, 1, 0)),
            Y_AXIS: Vector((1, 2, 1)),
            Z_AXIS: Vector((2, 1, 1))
        },
    XYX:
        {
            X_AXIS: Vector((1, 2, 1)),
            Y_AXIS: Vector((2, 1, 1)),
            Z_AXIS: Vector((1, 1, 0))
        },
    XZX:
        {
            X_AXIS: Vector((1, 1, 2)),
            Y_AXIS: Vector((1, 0, 1)),
            Z_AXIS: Vector((2, 1, 1))
        },
    YXY:
        {
            X_AXIS: Vector((1, 2, 1)),
            Y_AXIS: Vector((2, 1, 1)),
            Z_AXIS: Vector((1, 1, 0))
        },
    YZY:
        {
            X_AXIS: Vector((0, 1, 1)),
            Y_AXIS: Vector((1, 1, 2)),
            Z_AXIS: Vector((1, 2, 1))
        },
    ZXZ:
        {
            X_AXIS: Vector((1, 1, 2)),
            Y_AXIS: Vector((1, 0, 1)),
            Z_AXIS: Vector((2, 1, 1))
        },
    ZYZ:
        {
            X_AXIS: Vector((0, 1, 1)),
            Y_AXIS: Vector((1, 1, 2)),
            Z_AXIS: Vector((1, 2, 1))
        }
}

rotation_from_XYZ_to = {
    XYZ: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    XZY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    YXZ: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    YZX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    ZXY: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    ZYX: Matrix((-1, 0, 0), (0, -1, 0), (0, 0, 1)),
    XYX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    XZX: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    YXY: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    YZY: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    ZXZ: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    ZYZ: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0))
}

rotation_from_XZY_to = {
    XYZ: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    XZY: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    YXZ: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    YZX: Matrix((1, 0, 0), (0, -1, 0), (0, 0, -1)),
    ZXY: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0)),
    ZYX: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    XYX: Matrix((1, 0, 0), (0, -1, 0), (0, 0, -1)),
    XZX: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    YXY: Matrix((1, 0, 0), (0, -1, 0), (0, 0, -1)),
    YZY: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0)),
    ZXZ: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    ZYZ: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0))
}

rotation_from_YXZ_to = {
    XYZ: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0)),
    XZY: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    YXZ: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    YZX: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    ZXY: Matrix((-1, 0, 0), (0, 1, 0), (0, 0, -1)),
    ZYX: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    XYX: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    XZX: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0)),
    YXY: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    YZY: Matrix((-1, 0, 0), (0, 1, 0), (0, 0, -1)),
    ZXZ: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0)),
    ZYZ: Matrix((-1, 0, 0), (0, 1, 0), (0, 0, -1))
}

rotation_from_YZX_to = {
    XYZ: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    XZY: Matrix((1, 0, 0), (0, -1, 0), (0, 0, -1)),
    YXZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    YZX: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    ZXY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    ZYX: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    XYX: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    XZX: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    YXY: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    YZY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    ZXZ: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    ZYZ: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0))
}

rotation_from_ZXY_to = {
    XYZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    XZY: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    YXZ: Matrix((-1, 0, 0), (0, 1, 0), (0, 0, -1)),
    YZX: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    ZXY: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    ZYX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    XYX: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    XZX: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    YXY: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    YZY: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    ZXZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    ZYZ: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1))
}

rotation_from_ZYX_to = {
    XYZ: Matrix((-1, 0, 0), (0, -1, 0), (0, 0, 1)),
    XZY: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    YXZ: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    YZX: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0)),
    ZXY: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    ZYX: Matrix.id,
    XYX: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0)),
    XZX: Matrix((-1, 0, 0), (0, -1, 0), (0, 0, 1)),
    YXY: Matrix((0, 1, 0), (0, 0, 1), (1, 0, 0)),
    YZY: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    ZXZ: Matrix((-1, 0, 0), (0, -1, 0), (0, 0, 1)),
    ZYZ: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0))
}

rotation_from_XYX_to = {
    XYZ: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    XZY: Matrix((1, 0, 0), (0, -1, 0), (0, 0, -1)),
    YXZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    YZX: Matrix.id,
    ZXY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    ZYX: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    XYX: Matrix.id,
    XZX: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    YXY: Matrix.id,
    YZY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    ZXZ: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    ZYZ: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0))
}

rotation_from_XZX_to = {
    XYZ: Matrix.id,
    XZY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    YXZ: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    YZX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    ZXY: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    ZYX: Matrix((-1, 0, 0), (0, -1, 0), (0, 0, 1)),
    XYX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    XZX: Matrix.id,
    YXY: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    YZY: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    ZXZ: Matrix.id,
    ZYZ: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0))
}

rotation_from_YXY_to = {
    XYZ: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    XZY: Matrix((1, 0, 0), (0, -1, 0), (0, 0, -1)),
    YXZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    YZX: Matrix((1, 0, 0), (0, 1, 0), (0, 0, 1)),
    ZXY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    ZYX: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    XYX: Matrix.id,
    XZX: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    YXY: Matrix.id,
    YZY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    ZXZ: Matrix((0, 0, -1), (-1, 0, 0), (0, 1, 0)),
    ZYZ: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0))
}

rotation_from_YZY_to = {
    XYZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    XZY: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    YXZ: Matrix((-1, 0, 0), (0, 1, 0), (0, 0, -1)),
    YZX: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    ZXY: Matrix.id,
    ZYX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    XYX: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    XZX: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    YXY: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    YZY: Matrix.id,
    ZXZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    ZYZ: Matrix.id
}

rotation_from_ZXZ_to = {
    XYZ: Matrix.id,
    XZY: Matrix((0, -1, 0), (0, 0, -1), (1, 0, 0)),
    YXZ: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    YZX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    ZXY: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    ZYX: Matrix((-1, 0, 0), (0, -1, 0), (0, 0, 1)),
    XYX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    XZX: Matrix.id,
    YXY: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    YZY: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0)),
    ZXZ: Matrix.id,
    ZYZ: Matrix((0, 0, -1), (1, 0, 0), (0, -1, 0))
}

rotation_from_ZYZ_to = {
    XYZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    XZY: Matrix((0, 0, 1), (1, 0, 0), (0, 1, 0)),
    YXZ: Matrix((-1, 0, 0), (0, 1, 0), (0, 0, -1)),
    YZX: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    ZXY: Matrix.id,
    ZYX: Matrix((0, -1, 0), (0, 0, 1), (-1, 0, 0)),
    XYX: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    XZX: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    YXY: Matrix((0, 0, 1), (-1, 0, 0), (0, -1, 0)),
    YZY: Matrix.id,
    ZXZ: Matrix((0, 1, 0), (0, 0, -1), (-1, 0, 0)),
    ZYZ: Matrix.id
}

rotation_from_to_answers = {
    XYZ: rotation_from_XYZ_to,
    XZY: rotation_from_XZY_to,
    YXZ: rotation_from_YXZ_to,
    YZX: rotation_from_YZX_to,
    ZXY: rotation_from_ZXY_to,
    ZYX: rotation_from_ZYX_to,
    XYX: rotation_from_XYX_to,
    XZX: rotation_from_XZX_to,
    YXY: rotation_from_YXY_to,
    YZY: rotation_from_YZY_to,
    ZXZ: rotation_from_ZXZ_to,
    ZYZ: rotation_from_ZYZ_to
}

_vectors = [Vector.e1, Vector.e2, Vector.e3]
_vectors_neg = [-Vector.e1, -Vector.e2, -Vector.e3]


def make_dict(d):
    rtn = {}
    for order, matrix in d.items():
        subDict = {}
        rows = [Vector(matrix[0]), Vector(matrix[1]), Vector(matrix[2])]
        for i in range(3):
            try:
                ind = rows.index(_vectors[i])
                ls = _vectors
            except ValueError:
                ind = rows.index(_vectors_neg[i])
                ls = _vectors_neg
            subDict[i] = ls[ind]
        rtn[order] = subDict
    return rtn


rotation_refframe_from_XYZ_to = make_dict(rotation_from_XYZ_to)
rotation_refframe_from_XZY_to = make_dict(rotation_from_XZY_to)
rotation_refframe_from_YXZ_to = make_dict(rotation_from_YXZ_to)
rotation_refframe_from_YZX_to = make_dict(rotation_from_YZX_to)
rotation_refframe_from_ZXY_to = make_dict(rotation_from_ZXY_to)
rotation_refframe_from_ZYX_to = make_dict(rotation_from_ZYX_to)
rotation_refframe_from_XYX_to = make_dict(rotation_from_XYX_to)
rotation_refframe_from_XZX_to = make_dict(rotation_from_XZX_to)
rotation_refframe_from_YXY_to = make_dict(rotation_from_YXY_to)
rotation_refframe_from_YZY_to = make_dict(rotation_from_YZY_to)
rotation_refframe_from_ZXZ_to = make_dict(rotation_from_ZXZ_to)
rotation_refframe_from_ZYZ_to = make_dict(rotation_from_ZYZ_to)

rotation_refframe_from_to_answers = {
    XYZ: rotation_refframe_from_XYZ_to,
    XZY: rotation_refframe_from_XZY_to,
    YXZ: rotation_refframe_from_YXZ_to,
    YZX: rotation_refframe_from_YZX_to,
    ZXY: rotation_refframe_from_ZXY_to,
    ZYX: rotation_refframe_from_ZYX_to,
    XYX: rotation_refframe_from_XYX_to,
    XZX: rotation_refframe_from_XZX_to,
    YXY: rotation_refframe_from_YXY_to,
    YZY: rotation_refframe_from_YZY_to,
    ZXZ: rotation_refframe_from_ZXZ_to,
    ZYZ: rotation_refframe_from_ZYZ_to
}

_orders = [XYZ, XZY, YXZ, YZX, ZXY, ZYX, XYX, XZX, YXY, YZY, ZXZ, ZYZ]


def make_dict2(orderFrom, offset):
    rtnDict = {}
    angs90 = Angles(pi/2, pi/2, pi/2)
    matFrom = getMatrixEuler(orderFrom, angs90)
    for orderTo in _orders:
        matTo = getMatrixEuler(orderTo, angs90)
        axisDict = {}
        for i in range(3):
            tmp = rotateMatrixFrom(matFrom, _vectors[i])
            axisDict[i] = rotateOffsetTo(matTo, offset, tmp)
        rtnDict[orderTo] = axisDict
    return rtnDict

offset = Vector(1, 1, 1)
rotation_refframe_from_XYZ_to_offset = make_dict2(XYZ, offset)
rotation_refframe_from_XZY_to_offset = make_dict2(XZY, offset)
rotation_refframe_from_YXZ_to_offset = make_dict2(YXZ, offset)
rotation_refframe_from_YZX_to_offset = make_dict2(YZX, offset)
rotation_refframe_from_ZXY_to_offset = make_dict2(ZXY, offset)
rotation_refframe_from_ZYX_to_offset = make_dict2(ZYX, offset)
rotation_refframe_from_XYX_to_offset = make_dict2(XYX, offset)
rotation_refframe_from_XZX_to_offset = make_dict2(XZX, offset)
rotation_refframe_from_YXY_to_offset = make_dict2(YXY, offset)
rotation_refframe_from_YZY_to_offset = make_dict2(YZY, offset)
rotation_refframe_from_ZXZ_to_offset = make_dict2(ZXZ, offset)
rotation_refframe_from_ZYZ_to_offset = make_dict2(ZYZ, offset)

rotation_refframe_from_to_offset_answers = {
    XYZ: rotation_refframe_from_XYZ_to_offset,
    XZY: rotation_refframe_from_XZY_to_offset,
    YXZ: rotation_refframe_from_YXZ_to_offset,
    YZX: rotation_refframe_from_YZX_to_offset,
    ZXY: rotation_refframe_from_ZXY_to_offset,
    ZYX: rotation_refframe_from_ZYX_to_offset,
    XYX: rotation_refframe_from_XYX_to_offset,
    XZX: rotation_refframe_from_XZX_to_offset,
    YXY: rotation_refframe_from_YXY_to_offset,
    YZY: rotation_refframe_from_YZY_to_offset,
    ZXZ: rotation_refframe_from_ZXZ_to_offset,
    ZYZ: rotation_refframe_from_ZYZ_to_offset
}
