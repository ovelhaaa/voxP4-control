```bash
#!/usr/bin/env bash

# VoxP4-control — Linux environment bootstrap
#
# Usage:
#   chmod +x setup.sh
#   ./setup.sh
#
# Optional:
#   ./setup.sh --no-build
#
# After setup:
#   source .venv/bin/activate
#   python -m platformio run
#
# Designed for:
#   - Linux development machines
#   - CI environments
#   - coding agents / containers
#
# The script is intentionally idempotent.

set -Eeuo pipefail

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
VENV_DIR="${REPO_ROOT}/.venv"
VENV_PYTHON="${VENV_DIR}/bin/python"
PLATFORMIO_INI="${REPO_ROOT}/platformio.ini"

RUN_BUILD=1

if [[ "${1:-}" == "--no-build" ]]; then
    RUN_BUILD=0
elif [[ -n "${1:-}" ]]; then
    echo "Unknown argument: $1"
    echo "Usage: ./setup.sh [--no-build]"
    exit 2
fi

cd "${REPO_ROOT}"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

info() {
    printf '\033[1;34m[INFO]\033[0m %s\n' "$*"
}

ok() {
    printf '\033[1;32m[ OK ]\033[0m %s\n' "$*"
}

warn() {
    printf '\033[1;33m[WARN]\033[0m %s\n' "$*"
}

error() {
    printf '\033[1;31m[ERROR]\033[0m %s\n' "$*" >&2
}

die() {
    error "$*"
    exit 1
}

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# ---------------------------------------------------------------------------
# Header
# ---------------------------------------------------------------------------

echo
echo "========================================"
echo " VoxP4-control Linux environment setup"
echo "========================================"
echo

info "Repository root: ${REPO_ROOT}"

# ---------------------------------------------------------------------------
# Check required base tools
# ---------------------------------------------------------------------------

info "Checking base tools..."

command_exists git || die "git was not found."
command_exists python3 || die "python3 was not found."

PYTHON_VERSION="$(python3 --version 2>&1)"
ok "${PYTHON_VERSION}"

# Python should be recent enough for current PlatformIO.
PYTHON_MAJOR="$(python3 -c 'import sys; print(sys.version_info.major)')"
PYTHON_MINOR="$(python3 -c 'import sys; print(sys.version_info.minor)')"

if (( PYTHON_MAJOR < 3 || (PYTHON_MAJOR == 3 && PYTHON_MINOR < 9) )); then
    die "Python 3.9 or newer is required. Found ${PYTHON_VERSION}."
fi

# ---------------------------------------------------------------------------
# Check Python venv support
# ---------------------------------------------------------------------------

info "Checking Python virtual environment support..."

if ! python3 -c 'import venv' >/dev/null 2>&1; then
    error "Python venv support is missing."
    echo
    echo "On Debian/Ubuntu, install it with:"
    echo
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y python3-venv"
    echo
    exit 1
fi

ok "Python venv module available."

# ---------------------------------------------------------------------------
# Create virtual environment
# ---------------------------------------------------------------------------

info "Preparing virtual environment..."

if [[ ! -x "${VENV_PYTHON}" ]]; then
    info "Creating ${VENV_DIR}..."
    python3 -m venv "${VENV_DIR}"
else
    info "Existing virtual environment found."
fi

[[ -x "${VENV_PYTHON}" ]] || die "Could not create Python virtual environment."

ok "Virtual environment ready."

# ---------------------------------------------------------------------------
# Upgrade Python packaging tools
# ---------------------------------------------------------------------------

info "Updating pip/setuptools/wheel..."

"${VENV_PYTHON}" -m pip install \
    --upgrade \
    pip \
    setuptools \
    wheel

ok "Python packaging tools ready."

# ---------------------------------------------------------------------------
# Install PlatformIO
# ---------------------------------------------------------------------------

info "Installing/updating PlatformIO..."

"${VENV_PYTHON}" -m pip install --upgrade platformio

PIO_VERSION="$("${VENV_PYTHON}" -m platformio --version)"
ok "${PIO_VERSION}"

# ---------------------------------------------------------------------------
# Validate repository
# ---------------------------------------------------------------------------

info "Checking project files..."

[[ -f "${PLATFORMIO_INI}" ]] || {
    error "platformio.ini was not found."
    echo
    echo "Expected:"
    echo "  ${PLATFORMIO_INI}"
    echo
    exit 1
}

ok "platformio.ini found."

# ---------------------------------------------------------------------------
# Install PlatformIO project dependencies
# ---------------------------------------------------------------------------

info "Installing PlatformIO project dependencies..."

"${VENV_PYTHON}" -m platformio pkg install

ok "PlatformIO dependencies installed."

# ---------------------------------------------------------------------------
# Optional git submodules
# ---------------------------------------------------------------------------

if [[ -f "${REPO_ROOT}/.gitmodules" ]]; then
    info "Initializing Git submodules..."
    git submodule update --init --recursive
    ok "Git submodules ready."
fi

# ---------------------------------------------------------------------------
# Validation build
# ---------------------------------------------------------------------------

if (( RUN_BUILD == 1 )); then
    info "Running validation build..."

    if "${VENV_PYTHON}" -m platformio run; then
        ok "Validation build succeeded."
    else
        echo
        error "Environment setup succeeded, but the firmware build failed."
        echo
        echo "The toolchain is installed correctly."
        echo "Review the compiler output above for source/configuration errors."
        exit 1
    fi
else
    warn "Validation build skipped (--no-build)."
fi

# ---------------------------------------------------------------------------
# Finished
# ---------------------------------------------------------------------------

echo
echo "========================================"
echo " VoxP4-control environment is ready"
echo "========================================"
echo

echo "Activate the environment:"
echo
echo "  source .venv/bin/activate"
echo

echo "Build:"
echo
echo "  python -m platformio run"
echo

echo "Clean build:"
echo
echo "  python -m platformio run --target clean"
echo

echo "Upload to connected CYD:"
echo
echo "  python -m platformio run --target upload"
echo

echo "Serial monitor:"
echo
echo "  python -m platformio device monitor"
echo

echo "List serial devices:"
echo
echo "  python -m platformio device list"
echo

echo "Run setup without compiling:"
echo
echo "  ./setup.sh --no-build"
echo
```
