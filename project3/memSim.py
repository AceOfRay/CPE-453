import sys

from backing_store import BackingStore
from page_replacement import make_replacer
from page_table import PageTable
from physical_memory import PhysicalMemory
from tlb import TLB


def to_signed_byte(b: int) -> int:
    return b - 256 if b >= 128 else b


def read_reference_sequence(path: str):
    addrs = []
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            addrs.append(int(line))
    return addrs


def run(reference_path: str, frames: int = 256, pra: str = "fifo"):
    addresses = read_reference_sequence(reference_path)
    pra = pra.lower()

    if frames <= 0 or frames > 256:
        raise ValueError("FRAMES must be in 1..256")
    if pra not in ("fifo", "lru", "opt"):
        raise ValueError("PRA must be one of: fifo, lru, opt")

    pages_seq = [((logical >> 8) & 0xFF) for logical in addresses]
    replacer = make_replacer(pra, frames, pages_seq)

    tlb = TLB()
    pt = PageTable(size=256, empty=-1)
    bs = BackingStore()
    pm = PhysicalMemory(num_frames=frames, frame_size=256)

    # Track which page currently resides in each frame.
    frame_to_page = [-1] * frames

    tlb_hits = 0
    tlb_misses = 0
    page_faults = 0

    next_free_frame = 0

    for i, logical in enumerate(addresses):
        page = pages_seq[i]
        offset = logical & 0xFF

        replacer.on_reference(i, page)

        frame = tlb.lookup(page)
        if frame != tlb.empty:
            tlb_hits += 1
            replacer.on_access(frame)
        else:
            tlb_misses += 1
            frame = pt.lookup(page)

            if frame == pt.empty:
                page_faults += 1
                data = bs.read_page(page)

                if next_free_frame < frames:
                    frame = next_free_frame
                    next_free_frame += 1
                else:
                    victim_frame = replacer.pick_victim(frame_to_page)
                    old_page = frame_to_page[victim_frame]
                    if old_page != -1:
                        pt.invalidate(old_page)
                        tlb.invalidate(old_page)
                    frame = victim_frame

                pm.load_frame(frame, data)
                pt.set_mapping(page, frame)
                frame_to_page[frame] = page

            tlb.replace(page, frame)
            replacer.on_access(frame)

        raw = pm.read_byte(frame, offset)
        value = to_signed_byte(raw)
        frame_hex = pm.get_frame_bytes(frame).hex().upper()

        print(f"{logical}, {value}, {frame}, {frame_hex}")

    total = len(addresses)
    pf_rate = (page_faults / total) if total else 0.0
    tlb_total = tlb_hits + tlb_misses
    tlb_hit_rate = (tlb_hits / tlb_total) if tlb_total else 0.0
    tlb_miss_rate = (tlb_misses / tlb_total) if tlb_total else 0.0

    print(f"Number of Translated Addresses = {total}")
    print(f"Page Faults = {page_faults}")
    print(f"Page Fault Rate = {pf_rate:.3f}")
    print(f"TLB Hits = {tlb_hits}")
    print(f"TLB Misses = {tlb_misses}")
    print(f"TLB Hit Rate = {tlb_hit_rate:.3f}")

    return {
        "addresses": total,
        "frames": frames,
        "pra": pra,
        "page_faults": page_faults,
        "page_fault_rate": pf_rate,
        "tlb_hits": tlb_hits,
        "tlb_misses": tlb_misses,
        "tlb_hit_rate": tlb_hit_rate,
        "tlb_miss_rate": tlb_miss_rate,
    }


def main(argv=None):
    if argv is None:
        argv = sys.argv

    if len(argv) < 2:
        raise SystemExit("usage: memSim.py <reference-sequence-file.txt> [FRAMES] [PRA]")

    ref_path = argv[1]
    frames = 256
    pra = "fifo"

    if len(argv) >= 3:
        try:
            frames = int(argv[2])
        except ValueError:
            pra = argv[2]

    if len(argv) >= 4:
        pra = argv[3]

    run(ref_path, frames=frames, pra=pra)


if __name__ == "__main__":
    main()
