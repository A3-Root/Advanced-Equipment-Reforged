#!/usr/bin/env bash
# Registers every resource file without a .meta, re-saves new .et / .conf / .layout so Workbench writes IDs,
# gives dependency overrides their original GUID and resolves {?}path references.
# Uses headless Workbench (-scriptAuthorizeAll), ROOT_AER_RegisterResourcesPlugin and wb_autoanswer.ps1 (FBX = model).
# Usage: tools/register_resources.sh [--resave-all]
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EXE="G:/Games/Steam/steamapps/common/Arma Reforger Tools/Workbench/ArmaReforgerWorkbenchSteamDiag.exe"
ADDONS="G:/Games/Steam/steamapps/common/Arma Reforger/addons"
LIST="$ROOT/logs/wbregister.txt"
RESAVE="$ROOT/logs/wbresave.txt"
mkdir -p "$ROOT/logs"
: > "$LIST"
: > "$RESAVE"
cd "$ROOT"
find Configs Prefabs PrefabsEditable UI Sounds Assets Worlds Missions Language -type f 2>/dev/null \
  | grep -vE '\.(meta|blend|blend1|md|txt|py|sh|ps1|txo)$' | sort | while read -r f; do
  abs="$(cygpath -m "$ROOT/$f")"
  base="${f%.*}"
  case "$f" in
    *.fbx) [ -f "$base.xob.meta" ] && continue ;;
    *.png|*.tga) [ -f "$base.edds.meta" ] && continue ;;
  esac
  if [ ! -f "$f.meta" ]; then
    echo "$abs" >> "$LIST"
    case "$f" in *.et|*.conf|*.layout) echo "$abs" >> "$RESAVE" ;; esac
  elif [ "${1:-}" = "--resave-all" ]; then
    case "$f" in *.et|*.conf|*.layout) echo "$abs" >> "$RESAVE" ;; esac
  fi
done

run() {
  local log="$ROOT/logs/wbregister/$1"
  rm -rf "$log"
  mkdir -p "$log"
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(cygpath -w "$ROOT/tools/wb_autoanswer.ps1")" -Seconds 900 &
  local watcher=$!
  timeout 900 "$EXE" -gproj "$(cygpath -m "$ROOT/addon.gproj")" -addonsDir "$ADDONS" -wbmodule=ScriptEditor \
    -plugin=ROOT_AER_RegisterResourcesPlugin "$2" -wbsilent -scriptAuthorizeAll -logsDir "$(cygpath -m "$log")" > /dev/null 2>&1
  local code=$?
  kill "$watcher" 2>/dev/null
  echo "$1 exit $code"
  grep -h "\[AER\] \(FAILED\|NO META\|RESAVE FAILED\|done\)" "$log"/console.log 2>/dev/null
}

echo "register: $(wc -l < "$LIST"), resave: $(wc -l < "$RESAVE")"
[ -s "$LIST" ] && run register "-aerList=$(paste -sd'|' "$LIST")"
python "$ROOT/tools/fix_overrides.py"
python "$ROOT/tools/resolve_refs.py"
[ -s "$RESAVE" ] && run resave "-aerResave=$(paste -sd'|' "$RESAVE")"
exit 0
