"""Gives project files that override a dependency resource (same path) the GUID of that resource,
as Workbench "Override in addon" does. GUIDs are read from the dependency resourceDatabase.rdb files.
Usage: python tools/fix_overrides.py"""
import pathlib
import re
import struct

ROOT = pathlib.Path(__file__).resolve().parent.parent
RDBS = [
    "G:/Games/Steam/steamapps/common/Arma Reforger/addons/data/resourceDatabase.rdb",
    "G:/Games/Steam/steamapps/common/Arma Reforger/addons/core/resourceDatabase.rdb",
]
RDBS += [str(p) for p in pathlib.Path("E:/Documents/My Games/ArmaReforger/addons").glob("ACE*/resourceDatabase.rdb")]

deps = {}
for rdb in RDBS:
    try:
        data = open(rdb, "rb").read()
    except OSError:
        continue
    for m in re.finditer(rb"([\x20-\x7e]{4,})\x00(.{6})(.{8})", data, re.S):
        deps.setdefault(m.group(1).decode("ascii"), "%016X" % struct.unpack("<Q", m.group(3))[0])

for meta in ROOT.rglob("*.meta"):
    text = meta.read_text()
    m = re.search(r'Name "\{([0-9A-F]{16})\}([^"]+)"', text)
    if not m or m.group(2) not in deps or deps[m.group(2)] == m.group(1):
        continue
    meta.write_text(text.replace(m.group(1), deps[m.group(2)], 1), newline="\n")
    print("override", m.group(2), deps[m.group(2)])
