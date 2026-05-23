

class PageTable:
    def __init__(self, size: int = 256, empty: int = -1):
        self.size = size
        self.empty = empty

        self.frames = [empty] * size
        self.loaded = [0] * size

    def is_loaded(self, page: int) -> bool:
        return self.loaded[page] == 1

    def lookup(self, page: int) -> int:
        """Return frame if loaded, else self.empty."""
        if self.loaded[page] == 1:
            return self.frames[page]
        return self.empty

    def set_mapping(self, page: int, frame: int) -> None:

        self.frames[page] = frame
        self.loaded[page] = 1

    def invalidate(self, page: int) -> None:

        self.frames[page] = self.empty
        self.loaded[page] = 0

    def clear(self) -> None:
        for p in range(self.size):
            self.frames[p] = self.empty
            self.loaded[p] = 0
