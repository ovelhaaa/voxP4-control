#!/usr/bin/env python3
"""Verify that generated parameter files match the generator output exactly.

Run in CI:
    python scripts/check_contract_reproducibility.py
"""
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    print("Running generate_param_registry.py...")
    subprocess.check_call(
        [sys.executable, os.path.join(ROOT, "scripts", "generate_param_registry.py")]
    )

    print("Checking git diff on generated files...")
    targets = [
        os.path.join(ROOT, "src", "model", "ParameterRegistryData.inl"),
        os.path.join(ROOT, "schemas", "voxp4-parameters-v1.json"),
    ]
    try:
        subprocess.check_call(
            ["git", "diff", "--exit-code", "--no-patch"] + targets, cwd=ROOT
        )
        print("OK: Generated parameter contract files are in sync and reproducible.")
        return 0
    except subprocess.CalledProcessError:
        print("ERROR: Generated parameter contract files are out of date!")
        print("Run 'python scripts/generate_param_registry.py' and commit changes.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
