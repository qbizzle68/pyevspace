from copy import copy
import pickle

import pytest

from pyevspace import (
    XYZ, EulerAngles, ReferenceFrame, Vector, compute_rotation_matrix,
    rotate_between, rotate_from, rotate_to
)
from .inherited_types import DerivedEulerAngles
from .common import DummyIndex


@pytest.fixture
def angles():
    return EulerAngles(1, 2, 3)


def test_angles_construction(subtests, angles):
    for i in range(3):
        with subtests.test(index=i):
            assert angles[i] == i + 1


def test_angles_str(angles):
    s = str(angles)
    assert s == '[1, 2, 3]'


def test_angles_repr(angles):
    s = repr(angles)
    assert s == 'pyevspace.EulerAngles(1, 2, 3)'


def test_angles_persistance(subtests, angles):
    buffer = pickle.dumps(angles)
    a = pickle.loads(buffer)

    for i in range(3):
        with subtests.test(index=i):
            assert angles[i] == a[i]

    a = copy(angles)
    for i in range(3):
        with subtests.test(index=i):
            assert angles[i] == a[i]


def test_angles_get(angles):
    assert angles[0] == 1
    assert angles[1] == 2
    assert angles[2] == 3
    assert angles[-1] == 3
    assert angles[-2] == 2
    assert angles[-3] == 1

    with pytest.raises(IndexError):
        angles[3]

    with pytest.raises(IndexError):
        angles[-4]

    with pytest.raises(TypeError):
        angles['a']

    # __index__() support
    idx = DummyIndex(0)
    assert angles[idx] == 1
    idx.value = -1
    assert angles[idx] == 3
    idx.value = 3
    with pytest.raises(IndexError):
        angles[idx]
    idx.value = -4
    with pytest.raises(IndexError):
        angles[idx]


def test_angles_set(angles):
    angles[0] = 11
    angles[1] = 12
    angles[2] = 13

    assert angles[0] == 11
    assert angles[1] == 12
    assert angles[2] == 13

    angles[-1] = 3
    angles[-2] = 2
    angles[-3] = 1

    assert angles[0] == 1
    assert angles[1] == 2
    assert angles[2] == 3

    with pytest.raises(IndexError):
        angles[3] = 5

    with pytest.raises(IndexError):
        angles[-4] = 5

    with pytest.raises(TypeError):
        angles['a'] = 5

    with pytest.raises(TypeError):
        angles[0] = 'a'


def test_angles_inheritance() -> None:
    derived = DerivedEulerAngles(1, 2, 3)

    assert type(derived) is DerivedEulerAngles
    assert isinstance(derived, EulerAngles)
    assert issubclass(DerivedEulerAngles, EulerAngles)

    assert derived[0] == 1
    assert derived[1] == 2
    assert derived[2] == 3

    # Test global functions
    assert compute_rotation_matrix(XYZ, derived)
    assert compute_rotation_matrix(XYZ, derived, XYZ, derived)
    assert rotate_to(XYZ, derived, Vector.E1)
    assert rotate_from(XYZ, derived, Vector.E1)
    assert rotate_between(XYZ, derived, XYZ, derived, Vector.E1)
    frame = ReferenceFrame(XYZ, derived)
    frame.set_angles(derived)
    assert frame

    # Test repr
    assert repr(derived).startswith('tests.inherited_types')

    # Test __new__
    obj = EulerAngles.__new__(DerivedEulerAngles)
    obj.__init__(1, 2, 3)

    assert type(obj) is DerivedEulerAngles
    assert obj[0] == 1 and obj[1] == 2 and obj[2] == 3

    # Test __dict__ support
    assert derived.foo
