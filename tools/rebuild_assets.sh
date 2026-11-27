#!/usr/bin/env bash
# Imports model and texture sources (.fbx -> .xob, .png -> .edds) through headless Workbench.
# Sources whose runtime file is missing are registered again; Workbench stays open until every runtime file
# exists (at most 20 minutes), then only the instance started here is ended and references are updated.
# Usage: tools/rebuild_assets.sh
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EXE="G:/Games/Steam/steamapps/common/Arma Reforger Tools/Workbench/ArmaReforgerWorkbenchSteamDiag.exe"
ADDONS="G:/Games/Steam/steamapps/common/Arma Reforger/addons"
cd "$ROOT"
SRC=()
for f in $(find Assets UI -name "*.fbx" -o -name "*.png" | sort); do
  base="${f%.*}"
  case "$f" in
    *.fbx) out="$base.xob" ;;
    *) out="$base.edds" ;;
  esac
  [ -f "$out" ] && continue
  rm -f "$out.meta"
  SRC+=("$(cygpath -m "$ROOT/$f")")
done
[ ${#SRC[@]} -eq 0 ] && { echo "nothing to import"; exit 0; }
L=$(printf '%s\n' "${SRC[@]}" | paste -sd'|')
LOG="$ROOT/logs/wbregister/import"
rm -rf "$LOG"
mkdir -p "$LOG"
BEFORE=$(powershell.exe -NoProfile -Command '(Get-Process ArmaReforgerWorkbenchSteamDiag -ErrorAction SilentlyContinue).Id -join ","' | tr -d '\r')
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(cygpath -w "$ROOT/tools/wb_autoanswer.ps1")" -Seconds 1200 &
W=$!
"$EXE" -gproj "$(cygpath -m "$ROOT/addon.gproj")" -addonsDir "$ADDONS" -wbmodule=ScriptEditor \
  -plugin=ROOT_AER_RegisterResourcesPlugin "-aerList=$L" -aerKeepOpen=1 -wbsilent -scriptAuthorizeAll \
  -logsDir "$(cygpath -m "$LOG")" > /dev/null 2>&1 &
missing=${#SRC[@]}
for i in $(seq 1 240); do
  sleep 5
  missing=0
  for s in "${SRC[@]}"; do
    s="$(cygpath -u "$s")"
    base="${s%.*}"
    case "$s" in
      *.fbx) [ -f "$base.xob" ] || missing=$((missing + 1)) ;;
      *) [ -f "$base.edds" ] || missing=$((missing + 1)) ;;
    esac
  done
  [ "$missing" -eq 0 ] && break
done
sleep 10
powershell.exe -NoProfile -Command "\$keep = '$BEFORE'.Split(','); Get-Process ArmaReforgerWorkbenchSteamDiag -ErrorAction SilentlyContinue | Where-Object { \$keep -notcontains [string]\$_.Id } | Stop-Process -Force"
kill "$W" 2>/dev/null
echo "import missing: $missing"
python "$ROOT/tools/resolve_refs.py"
