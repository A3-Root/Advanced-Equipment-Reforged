"""Print {GUID}path for resources in a Workbench resourceDatabase.rdb matching a regex.
Usage: python tools/rdb_guid.py <rdb> <regex>"""
import re
import struct
import sys

data = open(sys.argv[1], "rb").read()
pat = re.compile(sys.argv[2])
for m in re.finditer(rb"([\x20-\x7e]{4,})\x00(.{6})(.{8})", data, re.S):
    path = m.group(1).decode("ascii")
    if not pat.search(path):
        continue
    guid = struct.unpack("<Q", m.group(3))[0]
    print("{%016X}%s" % (guid, path))
