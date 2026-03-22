from pyevspace import (
    Vector, Matrix, EulerAngles, RotationOrder, ReferenceFrame
)


class DerivedVector(Vector):
    def __new__(cls, *args, **kwargs):
        return Vector.__new__(cls, *args, **kwargs)

    def __init__(self, *args, **kwargs):
        self.foo = 1
        super().__init__(*args, **kwargs)


class DerivedMatrix(Matrix):
    def __new__(cls, *args, **kwargs):
        return Matrix.__new__(cls, *args, **kwargs)

    def __init__(self, *args, **kwargs):
        self.foo = 1
        super().__init__(*args, **kwargs)


class DerivedEulerAngles(EulerAngles):
    def __new__(cls, *args, **kwargs):
        return EulerAngles.__new__(cls, *args, **kwargs)

    def __init__(self, *args, **kwargs):
        self.foo = 1
        super().__init__(*args, **kwargs)


class DerivedRotationOrder(RotationOrder):
    def __new__(cls, *args, **kwargs):
        return RotationOrder.__new__(cls, *args, **kwargs)

    def __init__(self, *args, **kwargs):
        self.foo = 1
        super().__init__(*args, **kwargs)


class DerivedReferenceFrame(ReferenceFrame):
    def __new__(cls, *args, **kwargs):
        obj = ReferenceFrame.__new__(cls, *args, **kwargs)
        obj.foo = 1
        return obj
