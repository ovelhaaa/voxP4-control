#!/usr/bin/env python3
"""Synchronize the versioned VoxLink contract artifacts from a voxP4 checkout.

Copies the generated/ABI artifacts into src/voxlink/generated/:

    integration/VoxP4ParamIds.h        -> src/voxlink/generated/VoxP4ParamIds.h
    integration/voxlink_v1_vectors.h   -> src/voxlink/generated/voxlink_v1_vectors.h

Usage:
    python scripts/sync_voxlink_schema.py --voxp4 /path/to/voxP4
    python scripts/sync_voxlink_schema.py --voxp4 /path/to/voxP4 --check

--check performs no writes and exits non-zero if the local copies differ from
the voxP4 source. The normal controller build never needs the voxP4 checkout.
"""
from __future__ import annotations

import argparse
import filecmp
import os
import shutil
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEST_DIR = os.path.join(ROOT, "src", "voxlink", "generated")
FILES = [
    "VoxP4ParamIds.h",
    "voxlink_v1_vectors.h",
]


def main() -> int:
    ap = argparse.ArgumentParser(description="Sync VoxLink contract artifacts")
    ap.add_argument("--voxp4", default=os.environ.get("VOXP4_PATH", ""),
                    help="path to a voxP4 checkout (or set VOXP4_PATH)")
    ap.add_argument("--check", action="store_true",
                    help="verify only; do not modify files")
    args = ap.parse_args()

    if not args.voxp4:
        print("error: --voxp4 PATH (or VOXP4_PATH) is required", file=sys.stderr)
        return 2
    src_dir = os.path.join(args.voxp4, "integration")
    if not os.path.isdir(src_dir):
        print("error: %s/integration not found" % args.voxp4, file=sys.stderr)
        return 2

    os.makedirs(DEST_DIR, exist_ok=True)
    drift = False
    for name in FILES:
        src = os.path.join(src_dir, name)
        dst = os.path.join(DEST_DIR, name)
        if not os.path.isfile(src):
            print("error: missing %s" % src, file=sys.stderr)
            return 2
        if args.check:
            if not os.path.isfile(dst) or not filecmp.cmp(src, dst, shallow=False):
                print("STALE: %s" % os.path.relpath(dst, ROOT))
                drift = True
            else:
                print("ok:    %s" % os.path.relpath(dst, ROOT))
        else:
            shutil.copyfile(src, dst)
            print("synced: %s" % os.path.relpath(dst, ROOT))

    if args.check and drift:
        print("VoxLink contract artifacts are stale. Run the sync script.",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
