#!/usr/bin/env python3
"""Publish the top version of releases/CHANGELOG.md as a GitHub release (Arma Reforger projects).

Version: first `## ` heading in releases/CHANGELOG.md that carries MAJOR.MINOR.PATCH (e.g. `## 1.2.0`).
Creates tag vX.Y.Z at HEAD and a release titled vX.Y.Z, notes = that section, asset = <PREFIX>-X.Y.Z.zip
(git archive of HEAD). An existing release only gets its notes refreshed. Marks the release as latest.

Usage:
    python3 .github/scripts/auto_release.py            # publish (needs GH_TOKEN for gh)
    python3 .github/scripts/auto_release.py --dry-run  # preview only, no gh/git writes
"""
import re
import subprocess
import sys
from pathlib import Path

from packaging.version import Version  # type: ignore

PREFIX = "ROOT_AER"
CHANGELOG = Path("releases/CHANGELOG.md")
VERSION_TOKEN = re.compile(r"v?(\d+\.\d+\.\d+)\b")
DRY_RUN = "--dry-run" in sys.argv


def run(cmd, check=True):
    if DRY_RUN:
        print(f"   [dry-run] {' '.join(cmd)}")
        return subprocess.CompletedProcess(cmd, 0, b"", b"")
    return subprocess.run(cmd, check=check)


def top_section():
    """(version, section text) of the first versioned `## ` heading, or (None, None)."""
    if not CHANGELOG.exists():
        return None, None
    version, body = None, []
    for line in CHANGELOG.read_text(encoding="utf-8").splitlines():
        if line.startswith("## "):
            if version:
                break
            m = VERSION_TOKEN.search(line[3:])
            if m:
                version, body = m.group(1), [line.strip()]
            continue
        if version:
            body.append(line.rstrip())
    return version, ("\n".join(body).strip() if version else None)


def release_exists(tag):
    if DRY_RUN:
        return False
    return subprocess.run(["gh", "release", "view", tag], stdout=subprocess.PIPE, stderr=subprocess.PIPE).returncode == 0


def main():
    if DRY_RUN:
        print("Dry run: nothing will be published")
    version, notes = top_section()
    if not version:
        print(f"No versioned '## X.Y.Z' section in {CHANGELOG}; nothing to release")
        return
    Version(version)
    tag = f"v{version}"
    if release_exists(tag):
        print(f"Release {tag} exists; refreshing notes")
        run(["gh", "release", "edit", tag, "--notes", notes, "--latest"])
        return
    archive = Path(f"{PREFIX}-{version}.zip")
    run(["git", "archive", "--format=zip", f"--prefix={PREFIX}/", "-o", str(archive), "HEAD"])
    sha = subprocess.run(["git", "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
    print(f"Creating release {tag} at {sha}")
    if DRY_RUN:
        print("   --- release notes ---")
        print("   " + notes.replace("\n", "\n   "))
    run(["gh", "release", "create", tag, str(archive), "--target", sha, "--title", tag, "--notes", notes, "--latest"])
    print(f"Created release {tag}")


if __name__ == "__main__":
    main()
