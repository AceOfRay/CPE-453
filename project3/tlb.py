

class TLB:
    def __init__(self, size: int = 16, empty: int = -1):
        self.size = size
        self.empty = empty

        self.pages = [empty] * size
        self.frames = [empty] * size

        # rotating FIFO replacement pointer: next slot to overwrite.
        self.next_index = 0

    def lookup(self, page: int) -> int:
        for i in range(self.size):
            if self.pages[i] == page:
                return self.frames[i]
        return self.empty

    def replace(self, page: int, frame: int) -> int:

        # If the page is already in the TLB, just update its frame.
        for i in range(self.size):
            if self.pages[i] == page:
                self.frames[i] = frame
                return self.empty

        evicted_page = self.pages[self.next_index]
        self.pages[self.next_index] = page
        self.frames[self.next_index] = frame
        self.next_index = (self.next_index + 1) % self.size
        return evicted_page

    def invalidate(self, page: int) -> None:
        for i in range(self.size):
            if self.pages[i] == page:
                self.pages[i] = self.empty
                self.frames[i] = self.empty
                return

    def clear(self) -> None:
        for i in range(self.size):
            self.pages[i] = self.empty
            self.frames[i] = self.empty
        self.next_index = 0
