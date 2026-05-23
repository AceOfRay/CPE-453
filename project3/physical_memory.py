

class PhysicalMemory:
    def __init__(self, num_frames: int, frame_size: int = 256):
        self.num_frames = num_frames
        self.frame_size = frame_size

        # list of frames; each frame is a 256-byte bytearray.
        self.frames = [bytearray(frame_size) for _ in range(num_frames)]

    def load_frame(self, frame: int, data: bytes) -> None:
        self.frames[frame][:] = data

    def read_byte(self, frame: int, offset: int) -> int:
        return self.frames[frame][offset]

    def get_frame_bytes(self, frame: int) -> bytes:
        return bytes(self.frames[frame])
