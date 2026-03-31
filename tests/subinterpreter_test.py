from concurrent.interpreters import create

import pyevspace


def test_subinterpreter_support():
    interp = create()

    def import_module():
        import pyevspace
        return id(pyevspace)

    result = interp.call(import_module)
    assert id(pyevspace) != result


def test_heap_types(subtests):
    interp = create()

    def run():
        import pyevspace
        return {
            "vector": id(pyevspace.Vector),
            "matrix": id(pyevspace.Matrix),
            "angles": id(pyevspace.EulerAngles),
            "order": id(pyevspace.RotationOrder),
            "frame": id(pyevspace.ReferenceFrame)
        }

    types = {
        "vector": id(pyevspace.Vector),
        "matrix": id(pyevspace.Matrix),
        "angles": id(pyevspace.EulerAngles),
        "order": id(pyevspace.RotationOrder),
        "frame": id(pyevspace.ReferenceFrame)
    }

    result = interp.call(run)

    for name, id_ in types.items():
        with subtests.test(name=name):
            assert id_ != result[name]


def test_global_integer_constants():
    interp = create()

    def run():
        import pyevspace
        return (pyevspace.X_AXIS, pyevspace.Y_AXIS, pyevspace.Z_AXIS)

    pyevspace.X_AXIS = 100
    pyevspace.Y_AXIS = 101
    pyevspace.Z_AXIS = 102

    result = interp.call(run)

    assert result[0] == 0
    assert result[1] == 1
    assert result[2] == 2


def test_global_order_constants():
    interp = create()

    def run():
        import pyevspace
        return hash(pyevspace.XYZ)

    pyevspace.XYZ = 'abcd'

    result = interp.call(run)

    assert result != hash(pyevspace.XYZ)
