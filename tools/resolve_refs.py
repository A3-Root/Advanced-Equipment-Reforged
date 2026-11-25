"""Replace "{?}path" placeholders with "{GUID}path" and update stale GUIDs of the project's own resources.
GUID source: the project's own .meta files, then the resourceDatabase.rdb of each dependency.
Usage: python tools/resolve_refs.py"""
import pathlib
import re
import struct
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
RDBS = [
    "G:/Games/Steam/steamapps/common/Arma Reforger/addons/data/resourceDatabase.rdb",
    "G:/Games/Steam/steamapps/common/Arma Reforger/addons/core/resourceDatabase.rdb",
]
RDBS += [str(p) for p in pathlib.Path("E:/Documents/My Games/ArmaReforger/addons").glob("ACE*/resourceDatabase.rdb")]
EXTS = {".c", ".et", ".conf", ".layout", ".emat", ".ent", ".layer", ".imageset", ".styles", ".ct", ".acp"}
PH = re.compile(r'\{\?\}([A-Za-z0-9_./ -]+\.[A-Za-z0-9]+)')

guids = {}
for rdb in RDBS:
    try:
        data = open(rdb, "rb").read()
    except OSError:
        continue
    for m in re.finditer(rb"([\x20-\x7e]{4,})\x00(.{6})(.{8})", data, re.S):
        guids.setdefault(m.group(1).decode("ascii").lower(), "%016X" % struct.unpack("<Q", m.group(3))[0])
own = {}
for meta in ROOT.rglob("*.meta"):
    m = re.search(r'Name "\{([0-9A-F]{16})\}([^"]+)"', meta.read_text(errors="replace"))
    if m:
        guids[m.group(2).lower()] = m.group(1)
        own[m.group(2).lower()] = m.group(1)
STALE = re.compile(r'\{([0-9A-F]{16})\}([A-Za-z0-9_./ -]+\.[A-Za-z0-9]+)')

missing = set()
changed = 0
for f in ROOT.rglob("*"):
    if f.suffix not in EXTS or not f.is_file() or "logs" in f.parts:
        continue
    text = f.read_text(encoding="utf-8")

    def sub(m):
        g = guids.get(m.group(1).lower())
        if not g:
            missing.add(m.group(1))
            return m.group(0)
        return "{%s}%s" % (g, m.group(1))

    new = PH.sub(sub, text)

    def resync(m):
        g = own.get(m.group(2).lower())
        if g and g != m.group(1):
            return "{%s}%s" % (g, m.group(2))
        return m.group(0)

    new = STALE.sub(resync, new)
    if new != text:
        f.write_text(new, encoding="utf-8", newline="\n")
        changed += 1
print("files changed:", changed)
for p in sorted(missing):
    print("MISSING", p)
sys.exit(1 if missing else 0)
