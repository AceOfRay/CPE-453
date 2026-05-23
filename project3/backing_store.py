

class BackingStore:
    def __init__(self, path: str = "BACKING_STORE.bin", page_size: int = 256):
        self.path = path
        self.page_size = page_size

        self.num_pages = 65536 // page_size

        self.file = open(self.path, "rb")

    def read_page(self, page: int) -> bytes:
        
        offset = page * self.page_size
        self.file.seek(offset)
        data = self.file.read(self.page_size)
        if len(data) != self.page_size:
            raise RuntimeError(f"Short read: got {len(data)} bytes, expected {self.page_size}")
        return data
