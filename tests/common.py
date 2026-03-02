class DummyIndex:
    def __init__(self, idx: int) -> None:
        self.value = idx

    def __index__(self) -> int:
        return int(self.value)
