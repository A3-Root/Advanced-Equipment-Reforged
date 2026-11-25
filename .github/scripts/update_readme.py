#!/usr/bin/env python3
"""Set the README version and build badges from the top version of releases/CHANGELOG.md and push the change.

Badges replaced:
    ![version](https://img.shields.io/badge/version-X.Y.Z-blue)
    ![build](https://img.shields.io/badge/build-passing-green) / build-failing-red

Usage:
    python3 .github/scripts/update_readme.py                    # success
    python3 .github/scripts/update_readme.py --status failure   # failed release
"""
import re
import subprocess
import sys
import time
from pathlib import Path

BRANCH = "main"
README = Path("README.md")
CHANGELOG = Path("releases/CHANGELOG.md")
VERSION_TOKEN = re.compile(r"v?(\d+\.\d+\.\d+)\b")


def top_version():
    if not CHANGELOG.exists():
        return "unknown"
    for line in CHANGELOG.read_text(encoding="utf-8").splitlines():
        if line.startswith("## "):
            m = VERSION_TOKEN.search(line[3:])
            if m:
                return m.group(1)
    return "unknown"


def push_with_retry(attempts=4):
    for attempt in range(1, attempts + 1):
        if subprocess.run(["git", "push"]).returncode == 0:
            return True
        print(f"git push failed (attempt {attempt}/{attempts}); re-syncing and retrying")
        time.sleep(5 * attempt)
        subprocess.run(["git", "pull", "--rebase", "origin", BRANCH])
    return False


def main():
    status = sys.argv[2] if len(sys.argv) > 2 and sys.argv[1] == "--status" else "success"
    if not README.exists():
        print("README.md not found")
        sys.exit(1)
    version = top_version()
    old = README.read_text(encoding="utf-8")
    new = re.sub(r"!\[version\]\(https://img\.shields\.io/badge/version-[^-)]+-blue\)",
                 f"![version](https://img.shields.io/badge/version-{version}-blue)", old)
    badge = ("![build](https://img.shields.io/badge/build-passing-green)" if status == "success"
             else "![build](https://img.shields.io/badge/build-failing-red)")
    new = re.sub(r"!\[build\]\(https://img\.shields\.io/badge/build-(passing|failing)-(green|red)\)", badge, new)
    if new == old:
        print("README already up to date")
        return
    README.write_text(new, encoding="utf-8")
    subprocess.run(["git", "config", "user.name", "github-actions"], check=True)
    subprocess.run(["git", "config", "user.email", "github-actions@github.com"], check=True)
    subprocess.run(["git", "add", "README.md"], check=True)
    msg = (f"docs: update README for version {version} [skip ci]" if status == "success"
           else "docs: update README with build failure status [skip ci]")
    subprocess.run(["git", "commit", "-m", msg], check=True)
    if not push_with_retry():
        print("Could not push the README badge update; release itself succeeded")


if __name__ == "__main__":
    main()
