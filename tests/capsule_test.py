from capsule_consumer_c import (
    vector_capsule_c, matrix_capsule_c, angles_capsule_c, order_capsule_c,
    frame_capsule_c,
)
from capsule_consumer_cpp import (
    vector_capsule_cpp, matrix_capsule_cpp, angles_capsule_cpp,
    order_capsule_cpp, frame_capsule_cpp,
)
from pyevspace import (
    Vector, Matrix, EulerAngles, RotationOrder, XYZ, ReferenceFrame
)


def test_vector_capsule(subtests) -> None:
    v = Vector(1, 2, 3)
    vectors = vector_capsule_c(v)

    for i, vector in enumerate(vectors):
        with subtests.test(i=i):
            assert isinstance(vector, Vector)
            assert vector == v

    vectors = vector_capsule_cpp(v)

    for i, vector in enumerate(vectors):
        with subtests.test(i=i):
            assert isinstance(vector, Vector)
            assert vector == v


def test_matrix_capsule(subtests) -> None:
    m = Matrix((1, 2, 3), (4, 5, 6), (7, 8, 9))
    matrices = matrix_capsule_c(m)

    for i, matrix in enumerate(matrices):
        with subtests.test(i=i):
            assert isinstance(matrix, Matrix)
            assert matrix == m

    matrices = matrix_capsule_cpp(m)

    for i, matrix in enumerate(matrices):
        with subtests.test(i=i):
            assert isinstance(matrix, Matrix)
            assert matrix == m


def test_angles_capsule(subtests) -> None:
    a = EulerAngles(1, 2, 3)
    angles_list = angles_capsule_c(a)

    for i, angles in enumerate(angles_list):
        with subtests.test(i=i):
            assert isinstance(angles, EulerAngles)
            assert (angles[0] == a[0] and angles[1] == a[1] and
                    angles[2] == a[2])

    angles_list = angles_capsule_cpp(a)

    for i, angles in enumerate(angles_list):
        with subtests.test(i=i):
            assert isinstance(angles, EulerAngles)
            assert (angles[0] == a[0] and angles[1] == a[1]
                    and angles[2] == a[2])


def test_order_capsule(subtests) -> None:
    orders = order_capsule_c(XYZ)

    for i, order in enumerate(orders):
        with subtests.test(i=i):
            assert isinstance(order, RotationOrder)
            assert order == XYZ

    orders = order_capsule_cpp(XYZ)

    for i, order in enumerate(orders):
        with subtests.test(i=i):
            assert isinstance(order, RotationOrder)
            assert order == XYZ


def test_frame_capsule(subtests) -> None:
    angles = EulerAngles(1, 2, 3)
    offset = Vector(1, 2, 3)
    frame_arg = ReferenceFrame(XYZ, angles, offset=None, intrinsic=True)
    frame_offset_arg = ReferenceFrame(XYZ, angles, offset=offset,
                                      intrinsic=False)
    rotation_matrix_intrinsic = Matrix(
        (0.411982245665683, 0.05872664492762098, 0.9092974268256817),
        (-0.6812427202564033, -0.642872836134547, 0.35017548837401463),
        (0.6051272472413688, -0.7637183366502791, -0.2248450953661529)
    )
    rotation_matrix_extrinsic = Matrix(
        (0.411982245665683, -0.8337376517741567, -0.3676304629248992),
        (-0.05872664492762098, -0.42691762127620736, 0.9023815854833308),
        (-0.9092974268256817, -0.35017548837401463, -0.2248450953661529)
    )

    frames, offset_frames = frame_capsule_c(frame_arg, frame_offset_arg)
    for i, frame in enumerate(frames):
        with subtests.test(i=i):
            assert isinstance(frame, ReferenceFrame)
            assert frame.order == XYZ
            a = frame.get_angles()
            assert a[0] == 1. and a[1] == 2. and a[2] == 3.
            assert frame.offset is None
            assert frame.intrinsic is True
            assert frame.get_matrix() == rotation_matrix_intrinsic

    for i, frame in enumerate(offset_frames):
        with subtests.test(i=i):
            assert isinstance(frame, ReferenceFrame)
            assert frame.order == XYZ
            a = frame.get_angles()
            assert a[0] == 1. and a[1] == 2. and a[2] == 3.
            assert frame.offset == offset
            assert frame.intrinsic is False
            assert frame.get_matrix() == rotation_matrix_extrinsic

    frames, offset_frames = frame_capsule_cpp(frame_arg, frame_offset_arg)
    for i, frame in enumerate(frames):
        with subtests.test(i=i):
            assert isinstance(frame, ReferenceFrame)
            assert frame.order == XYZ
            a = frame.get_angles()
            assert a[0] == 1. and a[1] == 2. and a[2] == 3.
            assert frame.offset is None
            assert frame.intrinsic is True
            assert frame.get_matrix() == rotation_matrix_intrinsic

    for i, frame in enumerate(offset_frames):
        with subtests.test(i=i):
            assert isinstance(frame, ReferenceFrame)
            assert frame.order == XYZ
            a = frame.get_angles()
            assert a[0] == 1. and a[1] == 2. and a[2] == 3.
            assert frame.offset == offset
            assert frame.intrinsic is False
            assert frame.get_matrix() == rotation_matrix_extrinsic
