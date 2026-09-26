#!/usr/bin/env python3
"""
Synchronizes shared offscreen OpenGL rendering components from kintel/offscreen
into an OpenSCAD repository tree (src/glview/).
"""

import argparse
import difflib
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

DEFAULT_FILE_MAP = {
    "OffscreenContext.h": "src/glview/OffscreenContext.h",
    "OffscreenContextCGL.h": "src/glview/OffscreenContextCGL.h",
    "OffscreenContextCGL.cc": "src/glview/OffscreenContextCGL.cc",
    "OffscreenContextEGL.h": "src/glview/OffscreenContextEGL.h",
    "OffscreenContextEGL.cc": "src/glview/OffscreenContextEGL.cc",
    "OffscreenContextGLX.h": "src/glview/OffscreenContextGLX.h",
    "OffscreenContextGLX.cc": "src/glview/OffscreenContextGLX.cc",
    "OffscreenContextWGL.h": "src/glview/OffscreenContextWGL.h",
    "OffscreenContextWGL.cc": "src/glview/OffscreenContextWGL.cc",
    "OffscreenContextOSMesa.h": "src/glview/OffscreenContextOSMesa.h",
    "OffscreenContextOSMesa.cc": "src/glview/OffscreenContextOSMesa.cc",
    "OffscreenContextNULL.h": "src/glview/OffscreenContextNULL.h",
    "OffscreenContextNULL.cc": "src/glview/OffscreenContextNULL.cc",
    "OpenGLContext.h": "src/glview/OpenGLContext.h",
    "OpenGLContext.cc": "src/glview/OpenGLContext.cc",
    "fbo.h": "src/glview/fbo.h",
    "fbo.cc": "src/glview/fbo.cc",
}

INCLUDE_REPLACEMENTS = [
    (re.compile(r'#include "OffscreenContext\.h"'), '#include "glview/OffscreenContext.h"'),
    (re.compile(r'#include "OffscreenContextCGL\.h"'), '#include "glview/OffscreenContextCGL.h"'),
    (re.compile(r'#include "OffscreenContextEGL\.h"'), '#include "glview/OffscreenContextEGL.h"'),
    (re.compile(r'#include "OffscreenContextGLX\.h"'), '#include "glview/OffscreenContextGLX.h"'),
    (re.compile(r'#include "OffscreenContextWGL\.h"'), '#include "glview/OffscreenContextWGL.h"'),
    (re.compile(r'#include "OffscreenContextOSMesa\.h"'), '#include "glview/OffscreenContextOSMesa.h"'),
    (re.compile(r'#include "OffscreenContextNULL\.h"'), '#include "glview/OffscreenContextNULL.h"'),
    (re.compile(r'#include "OffscreenContextFactory\.h"'), '#include "glview/OffscreenContextFactory.h"'),
    (re.compile(r'#include "OpenGLContext\.h"'), '#include "glview/OpenGLContext.h"'),
    (re.compile(r'#include "system-gl\.h"'), '#include "glview/system-gl.h"'),
    (re.compile(r'#include "fbo\.h"'), '#include "glview/fbo.h"'),
    (re.compile(r'#include "scope_guard\.hpp"'), '#include "utils/scope_guard.hpp"'),
]

def find_openscad_dir(given_path=None):
    candidates = []
    if given_path:
        candidates.append(Path(given_path))
    else:
        script_dir = Path(__file__).resolve().parent
        repo_root = script_dir.parent
        candidates.extend([
            repo_root.parent / "OpenSCAD" / "openscad",
            repo_root.parent / "openscad",
            repo_root.parent.parent / "OpenSCAD" / "openscad",
            repo_root.parent.parent / "openscad",
            Path.cwd().parent / "openscad",
            Path.cwd().parent.parent / "openscad",
            Path.cwd().parent.parent / "OpenSCAD" / "openscad",
        ])

    for c in candidates:
        if c.exists() and (c / "src" / "glview").is_dir():
            return c.resolve()
    return None


def find_clang_format():
    found = shutil.which("clang-format")
    if found:
        return found
    for candidate in ["/opt/homebrew/bin/clang-format", "/usr/local/bin/clang-format"]:
        if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
            return candidate
    return None

def format_code(clang_format, content: str, filename: str, cwd: Path) -> str:
    if not clang_format:
        return content
    proc = subprocess.run(
        [clang_format, f"--assume-filename={filename}"],
        input=content,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        cwd=str(cwd),
        check=False,
    )
    if proc.returncode == 0:
        return proc.stdout
    return content


def transform_content(content: str) -> str:
    for pattern, replacement in INCLUDE_REPLACEMENTS:
        content = pattern.sub(replacement, content)
    return content

def main():
    parser = argparse.ArgumentParser(description="Sync offscreen components to OpenSCAD")
    parser.add_argument("openscad_dir", nargs="?", help="Path to OpenSCAD repository root")
    parser.add_argument("--dry-run", "-n", action="store_true", help="Print diffs without modifying files")
    parser.add_argument("--diff", "-d", action="store_true", help="Show unified diff of changes")
    parser.add_argument("--files", nargs="+", help="Specific files to sync")
    parser.add_argument("--format", action="store_true", help="Run clang-format on synced files")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parent.parent
    src_dir = repo_root / "src"

    openscad_root = find_openscad_dir(args.openscad_dir)
    if not openscad_root:
        print("Error: Could not locate OpenSCAD repository root.", file=sys.stderr)
        sys.exit(1)

    clang_format = None
    if args.format:
        clang_format = find_clang_format()
        if not clang_format:
            print("Error: --format requested, but 'clang-format' could not be found.", file=sys.stderr)
            sys.exit(1)

    print(f"Source repository: {repo_root}")
    print(f"Target OpenSCAD:   {openscad_root}")
    if clang_format:
        print(f"clang-format:      {clang_format}")

    files_to_sync = DEFAULT_FILE_MAP
    if args.files:
        files_to_sync = {k: v for k, v in DEFAULT_FILE_MAP.items() if k in args.files}

    synced, identical = 0, 0
    for src_rel, dst_rel in files_to_sync.items():
        src_path = src_dir / src_rel
        dst_path = openscad_root / dst_rel
        if not src_path.exists():
            continue

        with open(src_path, "r", encoding="utf-8") as f:
            new_content = transform_content(f.read())

        if clang_format and dst_path.suffix in [".cc", ".h", ".mm"]:
            new_content = format_code(clang_format, new_content, dst_rel, openscad_root)

        current_dst = ""
        if dst_path.exists():
            with open(dst_path, "r", encoding="utf-8") as f:
                current_dst = f.read()

        if current_dst == new_content:
            identical += 1
            continue

        synced += 1
        status = "[NEW]" if not dst_path.exists() else "[UPDATE]"
        print(f"{status} {src_rel} -> {dst_rel}")

        if args.diff or args.dry_run:
            diff = difflib.unified_diff(
                current_dst.splitlines(keepends=True),
                new_content.splitlines(keepends=True),
                fromfile=f"a/{dst_rel}",
                tofile=f"b/{dst_rel}",
            )
            sys.stdout.writelines(diff)

        if not args.dry_run:
            dst_path.parent.mkdir(parents=True, exist_ok=True)
            with open(dst_path, "w", encoding="utf-8") as f:
                f.write(new_content)

    print(f"Sync complete: {synced} updated, {identical} identical.")


if __name__ == "__main__":
    main()
