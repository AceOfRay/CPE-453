

from __future__ import annotations

from collections import deque


class FIFOReplacer:
    def __init__(self, frames: int):
        self.frames = frames
        self.ptr = 0

    def on_reference(self, i: int, page: int) -> None:
        pass

    def on_access(self, frame: int) -> None:
        pass

    def pick_victim(self, frame_to_page) -> int:
        victim = self.ptr
        self.ptr = (self.ptr + 1) % self.frames
        return victim


class LRUReplacer:
    def __init__(self, frames: int):
        self.frames = frames
        self.time = 0
        self.last_used = [0] * frames

    def on_reference(self, i: int, page: int) -> None:
        self.time += 1

    def on_access(self, frame: int) -> None:
        self.last_used[frame] = self.time

    def pick_victim(self, frame_to_page) -> int:
        victim = 0
        best = self.last_used[0]
        for f in range(1, self.frames):
            if self.last_used[f] < best:
                best = self.last_used[f]
                victim = f
        return victim


class OPTReplacer:
    def __init__(self, frames: int, pages_seq):
        self.frames = frames

        # future[page] = queue of future indices where that page is referenced.
        self.future = [deque() for _ in range(256)]
        for i, p in enumerate(pages_seq):
            self.future[p].append(i)

        self._current_index = 0

    def on_reference(self, i: int, page: int) -> None:
        self._current_index = i
        q = self.future[page]
        while q and q[0] <= i:
            q.popleft()

    def on_access(self, frame: int) -> None:
        pass

    def pick_victim(self, frame_to_page) -> int:
        victim = 0
        farthest = -1

        for f in range(self.frames):
            p = frame_to_page[f]
            if p == -1:
                return f

            q = self.future[p]
            nxt = q[0] if q else float("inf")

            if nxt > farthest:
                farthest = nxt
                victim = f

        return victim


def make_replacer(pra: str, frames: int, pages_seq):
    pra = pra.lower()
    if pra == "fifo":
        return FIFOReplacer(frames)
    if pra == "lru":
        return LRUReplacer(frames)
    if pra == "opt":
        return OPTReplacer(frames, pages_seq)
