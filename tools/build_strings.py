"""Writes Language/ROOT_AER_localization.en_us.conf (StringTableRuntime) from Language/ROOT_AER_localization.st,
entries sorted by Id. Optional: --set Id=Text ... adds or replaces entries in the .st first.
Usage: python tools/build_strings.py [--set Id=Text ...]"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
ST = ROOT / "Language/ROOT_AER_localization.st"
RT = ROOT / "Language/ROOT_AER_localization.en_us.conf"

text = ST.read_text()
items = dict(re.findall(r'Id "([^"]+)"\s*Target_en_us "([^"]*)"', text))
args = sys.argv[1:]
if args and args[0] == "--set":
    for a in args[1:]:
        key, _, value = a.partition("=")
        items[key] = value

keys = sorted(items)
st = ['StringTable {', ' ItemClassName "CustomStringTableItem"', ' Items {']
for k in keys:
    st += ['  CustomStringTableItem {', f'   Id "{k}"', f'   Target_en_us "{items[k]}"', '  }']
st += [' }', '}']
ST.write_text("\n".join(st), newline="\n")
rt = ['StringTableRuntime {', ' Ids {'] + [f'  "{k}"' for k in keys] + [' }', ' Texts {'] + [f'  "{items[k]}"' for k in keys] + [' }', '}']
RT.write_text("\n".join(rt), newline="\n")
print(f"{len(keys)} strings")
