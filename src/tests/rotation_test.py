from math import pi

import pytest

from .rotation_answers import (
    reversed_rotation_order,
    TestData,
    rotation_matrix_answers,
    rotation_euler_answers,
    rotation_axis_to_answers,
    rotation_axis_from_answers,
    rotation_axis_to_offset_answers,
    rotation_axis_from_offset_answers,
    rotation_euler_to_answers,
    rotation_euler_from_answers,
    rotation_offset_to_answers,
    rotation_offset_from_answers,
    rotation_from_to_answers,
    rotate_from_to_answers,
    rotate_from_XZY_to_ZYX_offset,
)
from pyevspace import (
    X_AXIS, XYZ, XZY, YZX, ZYX, YZY, ZYZ, Matrix, compute_rotation_matrix,
    rotate_from, rotate_to, rotate_between, EulerAngles,
)


@pytest.fixture
def data() -> TestData:
    return TestData()


def test_rotation_matrix_axis(data: TestData) -> None:
    for axis in data.axes:
        for angle, answer in zip(data.angles, rotation_matrix_answers[axis]):
            matrix = compute_rotation_matrix(angle, axis)
            assert matrix == answer

    # vector as axis
    for axis, vector in zip(data.axes, data.vectors):
        for angle, answer in zip(data.angles, rotation_matrix_answers[axis]):
            matrix = compute_rotation_matrix(angle, vector)
            assert matrix == answer


def test_rotation_matrix_euler(data: TestData) -> None:
    for order in data.orders:
        for angle, answer in zip(data.angles, rotation_euler_answers[order]):
            angles = EulerAngles(angle, angle, angle)
            matrix = compute_rotation_matrix(order, angles)
            assert matrix == answer

    # extrinsic
    for order in data.orders:
        reversed_order_answers \
            = rotation_euler_answers[reversed_rotation_order(order)]
        for angle, answer in zip(data.angles, reversed_order_answers):
            angles = EulerAngles(angle, angle, angle)
            matrix = compute_rotation_matrix(order, angles, intrinsic=False)
            assert matrix == answer


def test_rotation_matrix_from_to(data: TestData) -> None:
    for order_from in data.orders:
        for order_to in data.orders:
            answer = rotation_from_to_answers[order_from][order_to]
            matrix = compute_rotation_matrix(order_from, data.angs90,
                                             order_to, data.angs90)
            assert matrix == answer

    # order_from being extrinsic
    for order_from in data.orders:
        reversed_order = reversed_rotation_order(order_from)
        for order_to in data.orders:
            answer = rotation_from_to_answers[reversed_order][order_to]
            matrix = compute_rotation_matrix(order_from, data.angs90,
                                             order_to, data.angs90,
                                             intrinsic_from=False)
            assert matrix == answer

    # order_to being extrinsic
    for order_to in data.orders:
        reversed_order = reversed_rotation_order(order_to)
        for order_from in data.orders:
            answer = rotation_from_to_answers[order_from][reversed_order]
            matrix = compute_rotation_matrix(order_from, data.angs90,
                                             order_to, data.angs90,
                                             intrinsic_to=False)
            assert matrix == answer

    # both extrinsic
    for order_from in data.orders:
        from_reversed = reversed_rotation_order(order_from)
        for order_to in data.orders:
            to_reversed = reversed_rotation_order(order_to)
            answer = rotation_from_to_answers[from_reversed][to_reversed]
            matrix = compute_rotation_matrix(order_from, data.angs90,
                                             order_to, data.angs90,
                                             intrinsic_from=False,
                                             intrinsic_to=False)
            assert matrix == answer


def test_compute_matrix_errors(data: TestData) -> None:
    # test invalid axis ranges
    with pytest.raises(ValueError):
        compute_rotation_matrix(0, 3)

    with pytest.raises(ValueError):
        compute_rotation_matrix(0, -1)

    # test invalid argument types
    # first arg as float
    with pytest.raises(TypeError):
        compute_rotation_matrix('a', X_AXIS)

    with pytest.raises(TypeError):
        compute_rotation_matrix(0, [])

    with pytest.raises(TypeError):
        compute_rotation_matrix(1.5)

    with pytest.raises(TypeError):
        compute_rotation_matrix(1.5, X_AXIS, 3)

    # first arg as RotationOrder (size = 2)
    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, 0)

    with pytest.raises(TypeError):
        compute_rotation_matrix((), data.angs90[0])

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, data.angs90[0], intrinsic=5)

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ)

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, data.angs90[0], 'a')

    # first arg as RotationOrder (size = 4)
    with pytest.raises(TypeError):
        compute_rotation_matrix(1, data.angs90[0], XYZ, data.angs90[0])

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, 'a', XYZ, data.angs90[0])

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, data.angs90[0], 5, data.angs90[0])

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, data.angs90[0], YZY, [])

    # keyword args
    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, data.angs90[0], YZY, data.angs90[1],
                                intrinsic_from=4)

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, data.angs90[0], YZY, data.angs90[1],
                                intrinsic_to=tuple)

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, data.angs90[0], YZY)

    with pytest.raises(TypeError):
        compute_rotation_matrix(XYZ, data.angs90[0], YZY, data.angs90[1], 5)


def test_rotation_rotate_axis(data: TestData) -> None:
    # Rotate elementary vectors "to" around elementary axes using an axis
    # constant, getting the answers from rotation_axis_to_answers.
    for axis_rotation in data.axes:
        for axis_mapped in data.axes:
            answer = rotation_axis_to_answers[axis_rotation][axis_mapped]
            vector = rotate_to(pi / 2.0, axis_rotation,
                               data.vectors[axis_mapped])
            assert vector == answer

    # Rotate elementary vectors "to" around elementary axes using a Vector
    # as axis, getting the answers from rotation_axis_to_answers.
    for axis_rotation, vector_rotation in zip(data.axes, data.vectors):
        for axis_mapped in data.axes:
            answer = rotation_axis_to_answers[axis_rotation][axis_mapped]
            vector = rotate_to(pi / 2.0, vector_rotation,
                               data.vectors[axis_mapped])
            assert vector == answer

    # Rotate elementary vectors "from" around elementary axes using an axis
    # constant, getting the answers from rotation_axis_from_answers.
    for axis_rotation in data.axes:
        for axis_mapped in data.axes:
            answer = rotation_axis_from_answers[axis_rotation][axis_mapped]
            vector = rotate_from(pi / 2.0, axis_rotation,
                                 data.vectors[axis_mapped])
            assert vector == answer

    # Rotate elementary vectors "from" around elementary axes using a Vector
    # as axis, getting the answers from rotation_axis_from_answers.
    for axis_rotation, vector_rotation in zip(data.axes, data.vectors):
        for axis_mapped in data.axes:
            answer = rotation_axis_from_answers[axis_rotation][axis_mapped]
            vector = rotate_from(pi / 2.0, vector_rotation,
                                 data.vectors[axis_mapped])
            assert vector == answer

    # offsets
    # Rotate elementary vectors "to" around elementary axes with an offset,
    # using an axis constant and getting the answers from
    # rotation_axis_to_answers.
    for axis_rotation in data.axes:
        for axis_mapped in data.axes:
            answer \
                = rotation_axis_to_offset_answers[axis_rotation][axis_mapped]
            vector = rotate_to(pi / 2.0, axis_rotation,
                               data.vectors[axis_mapped], offset=data.offset)
            assert vector == answer

    # Rotate elementary vectors "to" around elementary axes with an offset,
    # using a Vector as axis, getting the answers from
    # rotation_axis_to_answers.
    for axis_rotation, vector_rotation in zip(data.axes, data.vectors):
        for axis_mapped in data.axes:
            answer = rotation_axis_to_answers[axis_rotation][axis_mapped]
            vector = rotate_to(pi / 2.0, vector_rotation,
                               data.vectors[axis_mapped])
            assert vector == answer

    # Rotate elementary vectors "from" around elementary axes with an offset,
    # using an axis constant and getting the answers from
    # rotation_axis_from_answers.
    for axis_rotation in data.axes:
        for axis_mapped in data.axes:
            _from_answers = rotation_axis_from_offset_answers[axis_rotation]
            answer = _from_answers[axis_mapped]
            vector = rotate_from(pi / 2.0, axis_rotation,
                                 data.vectors[axis_mapped], offset=data.offset)
            assert vector == answer

    # Rotate elementary vectors "from" around elementary axes with an offset,
    # using a Vector as axis, getting the answers from
    # rotation_axis_from_answers.
    for axis_rotation, vector_rotation in zip(data.axes, data.vectors):
        for axis_mapped in data.axes:
            answer = rotation_axis_from_answers[axis_rotation][axis_mapped]
            vector = rotate_from(pi / 2.0, vector_rotation,
                                 data.vectors[axis_mapped])
            assert vector == answer


def test_rotation_rotate_euler(data: TestData) -> None:
    for order in data.orders:
        for axis in data.axes:
            vector = rotate_to(order, data.angs90, data.vectors[axis])
            answer = rotation_euler_to_answers[order][axis]
            assert vector == answer

    # extrinsic
    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        for axis in data.axes:
            answer = rotation_euler_to_answers[reversed_order][axis]
            vector = rotate_to(order, data.angs90, data.vectors[axis],
                               intrinsic=False)
            assert vector == answer

    for order in data.orders:
        for axis in data.axes:
            vector = rotate_from(order, data.angs90, data.vectors[axis])
            answer = rotation_euler_from_answers[order][axis]
            assert vector == answer

    # extrinsic
    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        for axis in data.axes:
            answer = rotation_euler_from_answers[reversed_order][axis]
            vector = rotate_from(order, data.angs90, data.vectors[axis],
                                 intrinsic=False)
            assert vector == answer

    # offsets
    for order in data.orders:
        for axis in data.axes:
            vector = rotate_to(order, data.angs90, data.vectors[axis],
                               offset=data.offset)
            answer = rotation_offset_to_answers[order][axis]
            assert vector == answer

    # extrinsic
    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        for axis in data.axes:
            answer = rotation_offset_to_answers[reversed_order][axis]
            vector = rotate_to(order, data.angs90, data.vectors[axis],
                               intrinsic=False, offset=data.offset)
            assert vector == answer

    for order in data.orders:
        for axis in data.axes:
            vector = rotate_from(order, data.angs90, data.vectors[axis],
                                 offset=data.offset)
            answer = rotation_offset_from_answers[order][axis]
            assert vector == answer

    # extrinsic
    for order in data.orders:
        reversed_order = reversed_rotation_order(order)
        for axis in data.axes:
            answer = rotation_offset_from_answers[reversed_order][axis]
            vector = rotate_from(order, data.angs90, data.vectors[axis],
                                 intrinsic=False, offset=data.offset)
            assert vector == answer


def test_rotation_rotate_matrix(data: TestData) -> None:
    for order in data.orders:
        matrix = compute_rotation_matrix(order, data.angs90)
        for axis in data.axes:
            vector = rotate_to(matrix, data.vectors[axis])
            answer = rotation_euler_to_answers[order][axis]
            assert vector == answer

    for order in data.orders:
        matrix = compute_rotation_matrix(order, data.angs90)
        for axis in data.axes:
            vector = rotate_from(matrix, data.vectors[axis])
            answer = rotation_euler_from_answers[order][axis]
            assert vector == answer

    # offsets
    for order in data.orders:
        matrix = compute_rotation_matrix(order, data.angs90)
        for axis in data.axes:
            vector = rotate_to(matrix, data.vectors[axis], offset=data.offset)
            answer = rotation_offset_to_answers[order][axis]
            assert vector == answer

    for order in data.orders:
        matrix = compute_rotation_matrix(order, data.angs90)
        for axis in data.axes:
            vector = rotate_from(matrix, data.vectors[axis],
                                 offset=data.offset)
            answer = rotation_offset_from_answers[order][axis]
            assert vector == answer


def test_rotation_rotate_between(data: TestData) -> None:
    for order_from in data.orders:
        for order_to in data.orders:
            answer = rotate_from_to_answers[order_from][order_to]
            for i, test_vector in enumerate(data.vectors):
                vector = rotate_between(order_from, data.angs90, order_to,
                                        data.angs90, test_vector)
                assert vector == answer[i]

    # order_from being extrinsic
    for order_from in data.orders:
        reversed_order_from = reversed_rotation_order(order_from)
        for order_to in data.orders:
            answer = rotate_from_to_answers[reversed_order_from][order_to]
            for i, test_vector in enumerate(data.vectors):
                vector = rotate_between(order_from, data.angs90, order_to,
                                        data.angs90, test_vector,
                                        intrinsic_from=False)
                assert vector == answer[i]

    # order_to being extrinsic
    for order_to in data.orders:
        reversed_order_to = reversed_rotation_order(order_to)
        for order_from in data.orders:
            answer = rotate_from_to_answers[order_from][reversed_order_to]
            for i, test_vector in enumerate(data.vectors):
                vector = rotate_between(order_from, data.angs90, order_to,
                                        data.angs90, test_vector,
                                        intrinsic_to=False)
                assert vector == answer[i]

    # both extrinsic
    for order_from in data.orders:
        reversed_order_from = reversed_rotation_order(order_from)
        for order_to in data.orders:
            reversed_order_to = reversed_rotation_order(order_to)
            _from_answer = rotate_from_to_answers[reversed_order_from]
            answer = _from_answer[reversed_order_to]
            for i, test_vector in enumerate(data.vectors):
                vector = rotate_between(order_from, data.angs90, order_to,
                                        data.angs90, test_vector,
                                        intrinsic_from=False,
                                        intrinsic_to=False)
                assert vector == answer[i]

    # offset
    # This would require an astronomical amount of time to visual inspect what
    # the answers should be for all combinations of intrinsic and offset
    # frames. We'll only generate data for going between two frames that do
    # not produce a symetric matrix for now and hope that's enough (it should
    # be).

    # from XZY to ZYX
    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_to"][axis]
        vector = rotate_between(XZY, data.angs90, ZYX, data.angs90,
                                test_vector, offset_to=data.offset)
        assert vector == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_from"][axis]
        vector = rotate_between(XZY, data.angs90, ZYX, data.angs90,
                                test_vector, offset_from=data.offset)
        assert vector == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_both"][axis]
        vector = rotate_between(XZY, data.angs90, ZYX, data.angs90,
                                test_vector, offset_from=data.offset,
                                offset_to=data.offset)
        assert vector == answer

    # extrinsic from (note we must use 'YZX' as from order)
    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_to"][axis]
        vector = rotate_between(YZX, data.angs90, ZYX, data.angs90,
                                test_vector, intrinsic_from=False,
                                offset_to=data.offset)
        assert vector == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_from"][axis]
        vector = rotate_between(YZX, data.angs90, ZYX, data.angs90,
                                test_vector, intrinsic_from=False,
                                offset_from=data.offset)
        assert vector == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_both"][axis]
        vector = rotate_between(YZX, data.angs90, ZYX, data.angs90,
                                test_vector, intrinsic_from=False,
                                offset_from=data.offset,
                                offset_to=data.offset)
        assert vector == answer

    # extrinsic to (note we must use 'XYZ' as to order)
    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_to"][axis]
        vector = rotate_between(XZY, data.angs90, XYZ, data.angs90,
                                test_vector, intrinsic_to=False,
                                offset_to=data.offset)
        assert vector == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_from"][axis]
        vector = rotate_between(XZY, data.angs90, XYZ, data.angs90,
                                test_vector, intrinsic_to=False,
                                offset_from=data.offset)
        assert vector == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_both"][axis]
        vector = rotate_between(XZY, data.angs90, XYZ, data.angs90,
                                test_vector, intrinsic_to=False,
                                offset_from=data.offset,
                                offset_to=data.offset)
        assert vector == answer

    # both extrinsic (note we use YZX -> XYZ)
    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_to"][axis]
        vector = rotate_between(YZX, data.angs90, XYZ, data.angs90,
                                test_vector, intrinsic_from=False,
                                intrinsic_to=False,
                                offset_to=data.offset)
        assert vector == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_from"][axis]
        vector = rotate_between(YZX, data.angs90, XYZ, data.angs90,
                                test_vector, intrinsic_from=False,
                                intrinsic_to=False,
                                offset_from=data.offset)
        assert vector == answer

    for axis, test_vector in zip(data.axes, data.vectors):
        answer = rotate_from_XZY_to_ZYX_offset["offset_both"][axis]
        vector = rotate_between(YZX, data.angs90, XYZ, data.angs90,
                                test_vector, intrinsic_from=False,
                                intrinsic_to=False, offset_from=data.offset,
                                offset_to=data.offset)
        assert vector == answer


def test_rotate_to_errors(data: TestData) -> None:
    v = data.vectors[0]

    # invalid axis values
    with pytest.raises(ValueError):
        rotate_to(1.5, 3, v)

    with pytest.raises(ValueError):
        rotate_to(1.5, -1, v)

    # signature: (m: Matrix, v: Vector)
    m = Matrix()
    with pytest.raises(TypeError):
        rotate_to(m, 5)

    with pytest.raises(TypeError):
        rotate_to('a', v)

    with pytest.raises(TypeError):
        rotate_to(m)

    with pytest.raises(TypeError):
        rotate_to(m, v, True)

    with pytest.raises(TypeError):
        rotate_to(m, v, offset='a')

    # first arg being float
    with pytest.raises(TypeError):
        rotate_to(1.5, [], v)

    with pytest.raises(TypeError):
        rotate_to(1.5, v, 'a')

    with pytest.raises(TypeError):
        rotate_to('a', X_AXIS, v)

    with pytest.raises(TypeError):
        rotate_to(1.5, X_AXIS, v, offset=[])

    # first arg being RotationOrder
    a = EulerAngles()
    with pytest.raises(TypeError):
        rotate_to(XYZ, a, True)

    with pytest.raises(TypeError):
        rotate_to(XYZ, [], v)

    with pytest.raises(TypeError):
        rotate_to('a', a, v)

    with pytest.raises(TypeError):
        rotate_to(XYZ, a, v, offset=False)

    with pytest.raises(TypeError):
        rotate_to(XYZ, a)

    with pytest.raises(TypeError):
        rotate_to(XYZ, a, v, 5)


def test_rotate_from_errors(data: TestData) -> None:
    v = data.vectors[0]

    # invalid axis values
    with pytest.raises(ValueError):
        rotate_from(1.5, 3, v)

    with pytest.raises(ValueError):
        rotate_from(1.5, -1, v)

    # signature: (m: Matrix, v: Vector)
    m = Matrix()
    with pytest.raises(TypeError):
        rotate_from(m, 5)

    with pytest.raises(TypeError):
        rotate_from('a', v)

    with pytest.raises(TypeError):
        rotate_from(m)

    with pytest.raises(TypeError):
        rotate_from(m, v, True)

    with pytest.raises(TypeError):
        rotate_from(m, v, offset='a')

    # first arg being float
    with pytest.raises(TypeError):
        rotate_from(1.5, [], v)

    with pytest.raises(TypeError):
        rotate_from(1.5, v, 'a')

    with pytest.raises(TypeError):
        rotate_from('a', X_AXIS, v)

    with pytest.raises(TypeError):
        rotate_from(1.5, X_AXIS, v, offset=[])

    # first arg being RotationOrder
    a = EulerAngles()
    with pytest.raises(TypeError):
        rotate_from(XYZ, a, True)

    with pytest.raises(TypeError):
        rotate_from(XYZ, [], v)

    with pytest.raises(TypeError):
        rotate_from('a', a, v)

    with pytest.raises(TypeError):
        rotate_from(XYZ, a, v, offset=False)

    with pytest.raises(TypeError):
        rotate_from(XYZ, a)

    with pytest.raises(TypeError):
        rotate_from(XYZ, a, v, 5)


def test_rotate_between_errors(data: TestData) -> None:
    a = EulerAngles()
    with pytest.raises(TypeError):
        rotate_between('a', a, XYZ, a)

    with pytest.raises(TypeError):
        rotate_between(XYZ, 5, XYZ, a)

    with pytest.raises(TypeError):
        rotate_between(XYZ, a, str, a)

    with pytest.raises(TypeError):
        rotate_between(ZYZ, a, XYZ, data.vectors[0])

    with pytest.raises(TypeError):
        rotate_between(XYZ, a, ZYZ, a, offset_from=5)

    with pytest.raises(TypeError):
        rotate_between(XYZ, a, ZYZ, a, offset_to='a')
