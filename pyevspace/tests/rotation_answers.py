from math import cos, pi

ff = cos(pi / 4)    # cos and sin of forty-five

_rotation_matrix_answers = [
    [
        Matrix((1, 0, 0),       # X-rotation, 0.0 radians
               (0, 1, 0),
               (0, 0, 1)),
        Matrix((1, 0, 0),       # X-rotation, pi / 4 radians
               (0, ff, -ff),
               (0, ff, ff)),
        Matrix((1, 0, 0),       # X-rotation, pi / 2 radians
               (0, 0, -1),
               (0, 1, 0)),
        Matrix((1, 0, 0),       # X-rotation, 3 pi / 4 radians
               (0, -ff, -ff),
               (0, ff, -ff)),
        Matrix((1, 0, 0),       # X-rotation, pi radians
               (0, -1, 0),
               (0, 0, -1)),
        Matrix((1, 0, 0),       # X-rotation, 5 pi / 4 radians
               (0, -ff, ff),
               (0, -ff, -ff)),
        Matrix((1, 0, 0),       # X-rotation, 3 pi / 2 radians
               (0, 0, 1),
               (0, -1, 0)),
        Matrix((1, 0, 0),       # X-rotation, 7 pi / 4 radians
               (0, ff, ff),
               (0, -ff, ff))
    ],
    [
        Matrix((1, 0, 0),       # Y-rotation, 0.0 radians
               (0, 1, 0),
               (0, 0, 1)),
        Matrix((ff, 0, ff),     # Y-rotation, pi / 4 radians
               (0, 1, 0),
               (-ff, 0, ff)),
        Matrix((0, 0, 1),       # Y-rotation, pi / 2 radians
               (0, 1, 0),
               (-1, 0, 0)),
        Matrix((-ff, 0, ff),    # Y-rotation, 3 pi / 4 radians
               (0, 1, 0),
               (-ff, 0, -ff)),
        Matrix((-1, 0, 0),      # Y-rotation, pi radians
               (0, 1, 0),
               (0, 0, -1)),
        Matrix((-ff, 0, -ff),   # Y-rotation, 5 pi / 4 radians
               (0, 1, 0),
               (ff, 0, -ff)),
        Matrix((0, 0, -1),      # Y-rotation, 3 pi / 2 radians
               (0, 1, 0),
               (1, 0, 0)),
        Matrix((ff, 0, -ff),    # Y-rotation, 7 pi / 4 radians
               (0, 1, 0),
               (ff, 0, ff))
    ],
    [
        Matrix((1, 0, 0),       # Z-rotation, 0 radians
               (0, 1, 0),
               (0, 0, 1)),
        Matrix((ff, -ff, 0),    # Z-rotation, pi / 4 radians
               (ff, ff, 0),
               (0, 0, 1)),
        Matrix((0, -1, 0),      # Z-rotation, pi / 2 radians
               (1, 0, 0),
               (0, 0, 1)),
        Matrix((-ff, -ff, 0),   # Z-rotation, 3 pi / 4 radians
               (ff, -ff, 0),
               (0, 0, 1)),
        Matrix((-1, 0, 0),      # Z-rotation, pi radians
               (0, -1, 0),
               (0, 0, 1)),
        Matrix((-ff, ff, 0),    # Z-rotation, 5 pi / 4 radians
               (-ff, -ff, 0),
               (0, 0, 1)),
        Matrix((0, 1, 0),       # Z-rotation, 3 pi / 2 radians
               (-1, 0, 0),
               (0, 0, 1)),
        Matrix((ff, ff, 0),     # Z-rotation, 7 pi / 4 radians
               (-ff, ff, 0),
               (0, 0, 1))
    ]
]