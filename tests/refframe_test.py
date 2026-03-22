import pytest

from pyevspace import (
    ReferenceFrame, XYZ, XZY, ZYX, YZX, EulerAngles, Vector, Matrix
)
from tests.inherited_types import DerivedReferenceFrame
from .rotation_answers import (
    reversed_rotation_order,
    TestData,
    rotation_euler_answers,
    rotation_euler_to_answers,
    rotation_euler_from_answers,
    rotation_offset_to_answers,
    rotation_offset_from_answers,
    rotate_from_to_answers,
    rotate_from_XZY_to_ZYX_offset,
)


@pytest.fixture
def data() -> TestData:
    return TestData()


def test_refframe_new(subtests) -> None:
    angles = EulerAngles(0, 0, 0)
    frame = ReferenceFrame(XYZ, angles)

    assert frame.get_matrix() == Matrix.IDENTITY
    assert frame.offset is None
    assert frame.intrinsic is True

    for i in range(3):
        with subtests.test(i=i):
            assert frame.get_angles()[i] == angles[i]
    assert frame.get_angles() is not angles

    frame = ReferenceFrame(XYZ, angles, intrinsic=False,
                           offset=Vector(1, 1, 1))
    assert frame.intrinsic is False
    assert frame.offset == Vector(1, 1, 1)

    frame = ReferenceFrame(XYZ, angles, offset=None)
    assert frame.offset is None

    frame = ReferenceFrame(XYZ, angles, intrinsic=(1 > 2))
    assert frame.intrinsic is False

    with pytest.raises(TypeError):
        ReferenceFrame(3, angles)

    with pytest.raises(TypeError):
        ReferenceFrame(XYZ, [])

    with pytest.raises(TypeError):
        ReferenceFrame(XYZ)

    with pytest.raises(TypeError):
        ReferenceFrame(XYZ, angles, 0)

    with pytest.raises(TypeError):
        ReferenceFrame(XYZ, angles, offset=5)


def test_refframe_angles(subtests) -> None:
    frame = ReferenceFrame(XYZ, EulerAngles())

    angles = frame.get_angles()
    for i in range(3):
        with subtests.test(i=i):
            assert angles[i] == 0.0

    frame.set_angles(alpha=1)
    assert frame.get_angles()[0] == 1.0
    assert frame.get_angles()[1] == 0.0
    assert frame.get_angles()[2] == 0.0

    frame.set_angles(beta=2)
    assert frame.get_angles()[0] == 1.0
    assert frame.get_angles()[1] == 2.0
    assert frame.get_angles()[2] == 0.0

    frame.set_angles(gamma=3)
    assert frame.get_angles()[0] == 1.0
    assert frame.get_angles()[1] == 2.0
    assert frame.get_angles()[2] == 3.0

    frame.set_angles(alpha=0.0, beta=None, gamma=None)
    assert frame.get_angles()[0] == 0.0
    assert frame.get_angles()[1] == 2.0
    assert frame.get_angles()[2] == 3.0

    frame.set_angles(alpha=None, beta=0.0, gamma=None)
    assert frame.get_angles()[0] == 0.0
    assert frame.get_angles()[1] == 0.0
    assert frame.get_angles()[2] == 3.0

    frame.set_angles(alpha=None, beta=None, gamma=0.0)
    assert frame.get_angles()[0] == 0.0
    assert frame.get_angles()[1] == 0.0
    assert frame.get_angles()[2] == 0.0

    angles = EulerAngles(7, 8, 9)
    frame.set_angles(angles)
    assert frame.get_angles()[0] == 7.0
    assert frame.get_angles()[1] == 8.0
    assert frame.get_angles()[2] == 9.0

    with pytest.raises(TypeError):
        frame.get_angles(5)

    with pytest.raises(TypeError):
        frame.set_angles('abc')

    with pytest.raises(TypeError):
        frame.set_angles(alpha='a')

    with pytest.raises(TypeError):
        frame.set_angles(beta=[])

    with pytest.raises(TypeError):
        frame.set_angles(gamma=list)


def test_refframe_matrix(data: TestData) -> None:
    for order in data.orders:
        for angle, answer in zip(data.angles, rotation_euler_answers[order]):
            angles = EulerAngles(angle, angle, angle)
            frame = ReferenceFrame(order, angles)
            assert frame.get_matrix() == answer

    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        reversed_order_answers = rotation_euler_answers[reversed_order]
        for angle, answer in zip(data.angles, reversed_order_answers):
            angles = EulerAngles(angle, angle, angle)
            frame = ReferenceFrame(order, angles, intrinsic=False)
            assert frame.get_matrix() == answer


def test_refframe_rotate_to(data: TestData) -> None:
    for order in data.orders:
        frame = ReferenceFrame(order, data.angs90)
        for axis in data.axes:
            vector = frame.rotate_to(data.vectors[axis])
            answer = rotation_euler_to_answers[order][axis]
            assert vector == answer

    # extrinsic
    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        frame = ReferenceFrame(order, data.angs90, intrinsic=False)
        for axis in data.axes:
            vector = frame.rotate_to(data.vectors[axis])
            answer = rotation_euler_to_answers[reversed_order][axis]
            assert vector == answer

    # offsets
    for order in data.orders:
        frame = ReferenceFrame(order, data.angs90, offset=data.offset)
        for axis in data.axes:
            vector = frame.rotate_to(data.vectors[axis])
            answer = rotation_offset_to_answers[order][axis]
            assert vector == answer

    # extrinsic
    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        frame = ReferenceFrame(order, data.angs90, intrinsic=False,
                               offset=data.offset)
        for axis in data.axes:
            vector = frame.rotate_to(data.vectors[axis])
            answer = rotation_offset_to_answers[reversed_order][axis]
            assert vector == answer

    frame = ReferenceFrame(XYZ, EulerAngles())
    with pytest.raises(TypeError):
        frame.rotate_to(5)

    with pytest.raises(TypeError):
        frame.rotate_to(frame, 'a')

    with pytest.raises(TypeError):
        frame.rotate_to()

    with pytest.raises(TypeError):
        frame.rotate_to(Vector.E1, 0)

    with pytest.raises(TypeError):
        frame.rotate_to(frame, Vector.E1, 'a')


def test_refframe_rotate_from(data: TestData) -> None:
    for order in data.orders:
        frame = ReferenceFrame(order, data.angs90)
        for axis in data.axes:
            vector = frame.rotate_from(data.vectors[axis])
            answer = rotation_euler_from_answers[order][axis]
            assert vector == answer

    # extrinsic
    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        frame = ReferenceFrame(order, data.angs90, intrinsic=False)
        for axis in data.axes:
            vector = frame.rotate_from(data.vectors[axis])
            answer = rotation_euler_from_answers[reversed_order][axis]
            assert vector == answer

    # offsets
    for order in data.orders:
        frame = ReferenceFrame(order, data.angs90, offset=data.offset)
        for axis in data.axes:
            vector = frame.rotate_from(data.vectors[axis])
            answer = rotation_offset_from_answers[order][axis]
            assert vector == answer

    # extrinsic
    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        frame = ReferenceFrame(order, data.angs90, intrinsic=False,
                               offset=data.offset)
        for axis in data.axes:
            vector = frame.rotate_from(data.vectors[axis])
            answer = rotation_offset_from_answers[reversed_order][axis]
            assert vector == answer

    frame = ReferenceFrame(XYZ, EulerAngles())
    with pytest.raises(TypeError):
        frame.rotate_from(5)

    with pytest.raises(TypeError):
        frame.rotate_from(frame, 'a')

    with pytest.raises(TypeError):
        frame.rotate_from()

    with pytest.raises(TypeError):
        frame.rotate_from(Vector.E1, 0)

    with pytest.raises(TypeError):
        frame.rotate_from(frame, Vector.E1, 'a')


def test_refframe_rotate_between(data: TestData) -> None:
    for order_from in data.orders:
        frame_from = ReferenceFrame(order_from, data.angs90)
        for order_to in data.orders:
            frame_to = ReferenceFrame(order_to, data.angs90)
            answer = rotate_from_to_answers[order_from][order_to]
            for i, test_vector in enumerate(data.vectors):
                vector_to = frame_to.rotate_to(frame_from, test_vector)
                vector_from = frame_from.rotate_from(frame_to, test_vector)
                assert vector_to == vector_from
                assert vector_to == answer[i]

    # order_from being extrinsic
    for order_from in data.orders:
        frame_from = ReferenceFrame(order_from, data.angs90, intrinsic=False)
        reversed_order_from = reversed_rotation_order(order_from)
        for order_to in data.orders:
            frame_to = ReferenceFrame(order_to, data.angs90)
            answer = rotate_from_to_answers[reversed_order_from][order_to]
            for i, test_vector in enumerate(data.vectors):
                vector_to = frame_to.rotate_to(frame_from, test_vector)
                vector_from = frame_from.rotate_from(frame_to, test_vector)
                assert vector_to == vector_from
                assert vector_to == answer[i]

    # order_to being extrinsic
    for order_to in data.orders:
        reversed_order_to = reversed_rotation_order(order_to)
        frame_to = ReferenceFrame(order_to, data.angs90, intrinsic=False)
        for order_from in data.orders:
            frame_from = ReferenceFrame(order_from, data.angs90)
            answer = rotate_from_to_answers[order_from][reversed_order_to]
            for i, test_vector in enumerate(data.vectors):
                vector_to = frame_to.rotate_to(frame_from, test_vector)
                vector_from = frame_from.rotate_from(frame_to, test_vector)
                assert vector_to == vector_from
                assert vector_to == answer[i]

    # both extrinsic
    for order_from in data.orders:
        reversed_order_from = reversed_rotation_order(order_from)
        frame_from = ReferenceFrame(order_from, data.angs90, intrinsic=False)
        for order_to in data.orders:
            reversed_order_to = reversed_rotation_order(order_to)
            frame_to = ReferenceFrame(order_to, data.angs90, intrinsic=False)
            _from_answers = rotate_from_to_answers[reversed_order_from]
            answer = _from_answers[reversed_order_to]
            for i, test_vector in enumerate(data.vectors):
                vector_to = frame_to.rotate_to(frame_from, test_vector)
                vector_from = frame_from.rotate_from(frame_to, test_vector)
                assert vector_to == vector_from
                assert vector_to == answer[i]

    # offset
    # This would require an astronomical amount of time to visual inspect what
    # the answers should be for all combinations of intrinsic and offset
    # frames. We'll only generate data for going between two frames that do
    # not produce a symetric matrix for now and hope that's enough (it should
    # be).

    # from XZY to ZYX
    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_to"][axis]
        frame_from = ReferenceFrame(XZY, data.angs90)
        frame_to = ReferenceFrame(ZYX, data.angs90, offset=data.offset)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_from"][axis]
        frame_from = ReferenceFrame(XZY, data.angs90, offset=data.offset)
        frame_to = ReferenceFrame(ZYX, data.angs90)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_both"][axis]
        frame_from = ReferenceFrame(XZY, data.angs90, offset=data.offset)
        frame_to = ReferenceFrame(ZYX, data.angs90, offset=data.offset)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    # extrinsic from (not we must use 'YZX' as from order)
    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_to"][axis]
        frame_from = ReferenceFrame(YZX, data.angs90, intrinsic=False)
        frame_to = ReferenceFrame(ZYX, data.angs90, offset=data.offset)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_from"][axis]
        frame_from = ReferenceFrame(YZX, data.angs90, offset=data.offset,
                                    intrinsic=False)
        frame_to = ReferenceFrame(ZYX, data.angs90)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_both"][axis]
        frame_from = ReferenceFrame(YZX, data.angs90, offset=data.offset,
                                    intrinsic=False)
        frame_to = ReferenceFrame(ZYX, data.angs90, offset=data.offset)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    # extrinsic to (note we must use 'XYZ' as to order)
    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_to"][axis]
        frame_from = ReferenceFrame(XZY, data.angs90)
        frame_to = ReferenceFrame(XYZ, data.angs90, offset=data.offset,
                                  intrinsic=False)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_from"][axis]
        frame_from = ReferenceFrame(XZY, data.angs90, offset=data.offset)
        frame_to = ReferenceFrame(XYZ, data.angs90, intrinsic=False)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_both"][axis]
        frame_from = ReferenceFrame(XZY, data.angs90, offset=data.offset)
        frame_to = ReferenceFrame(XYZ, data.angs90, offset=data.offset,
                                  intrinsic=False)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    # both extrinsic (note we use YZX -> XYZ)
    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_to"][axis]
        frame_from = ReferenceFrame(YZX, data.angs90, intrinsic=False)
        frame_to = ReferenceFrame(XYZ, data.angs90, offset=data.offset,
                                  intrinsic=False)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_from"][axis]
        frame_from = ReferenceFrame(YZX, data.angs90, offset=data.offset,
                                    intrinsic=False)
        frame_to = ReferenceFrame(XYZ, data.angs90, intrinsic=False)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_both"][axis]
        frame_from = ReferenceFrame(YZX, data.angs90, offset=data.offset,
                                    intrinsic=False)
        frame_to = ReferenceFrame(XYZ, data.angs90, offset=data.offset,
                                  intrinsic=False)
        vector_to = frame_to.rotate_to(frame_from, test_vector)
        vector_from = frame_from.rotate_from(frame_to, test_vector)
        assert vector_to == vector_from
        assert vector_to == answer


def test_refframe_inheritance() -> None:
    derived = DerivedReferenceFrame(XYZ, EulerAngles(1, 2, 3))

    assert type(derived) is DerivedReferenceFrame
    assert isinstance(derived, ReferenceFrame)
    assert issubclass(DerivedReferenceFrame, ReferenceFrame)

    frame = ReferenceFrame(XYZ, EulerAngles(1, 2, 3))
    assert derived.get_matrix() == frame.get_matrix()

    # Test global methods
    assert derived.rotate_to(Vector.E1)
    assert derived.rotate_to(frame, Vector.E1)
    assert frame.rotate_to(derived, Vector.E1)
    assert derived.rotate_from(Vector.E1)
    assert derived.rotate_from(frame, Vector.E1)
    assert frame.rotate_from(derived, Vector.E1)

    # Test __new__
    obj = ReferenceFrame.__new__(DerivedReferenceFrame, XYZ,
                                 EulerAngles(1, 2, 3))
    assert obj.order == derived.order
    assert obj.get_matrix() == derived.get_matrix()

    # Test __dict__ support
    assert derived.foo
