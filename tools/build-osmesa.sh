#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
DEPS_DIR="${ROOT_DIR}/dependencies"
MESA_SRC="${DEPS_DIR}/mesa"
MESA_BUILD="${DEPS_DIR}/mesa_build"
MESA_INSTALL="${DEPS_DIR}/mesa_install"
MESA_VERSION="${MESA_VERSION:-24.3.4}"

mkdir -p "${DEPS_DIR}"

# Ensure Homebrew is in PATH
if [ -d "/opt/homebrew/bin" ]; then
  export PATH="/opt/homebrew/bin:/opt/homebrew/sbin:${PATH}"
elif [ -d "/usr/local/bin" ]; then
  export PATH="/usr/local/bin:${PATH}"
fi

# Ensure Homebrew tools and libraries are found
if command -v brew >/dev/null 2>&1; then
  LLVM_PREFIX="$(brew --prefix llvm 2>/dev/null || true)"
  BISON_PREFIX="$(brew --prefix bison 2>/dev/null || true)"
  HOMEBREW_PREFIX="$(brew --prefix 2>/dev/null || true)"
  [ -n "${HOMEBREW_PREFIX}" ] && export PATH="${HOMEBREW_PREFIX}/bin:${PATH}"
  [ -n "${LLVM_PREFIX}" ] && export PATH="${LLVM_PREFIX}/bin:${PATH}"
  [ -n "${BISON_PREFIX}" ] && export PATH="${BISON_PREFIX}/bin:${PATH}"
  [ -n "${HOMEBREW_PREFIX}" ] && export LIBRARY_PATH="${HOMEBREW_PREFIX}/lib:${LLVM_PREFIX}/lib:${LIBRARY_PATH:-}"
  [ -n "${HOMEBREW_PREFIX}" ] && export LDFLAGS="-L${HOMEBREW_PREFIX}/lib -L${LLVM_PREFIX}/lib ${LDFLAGS:-}"
fi

# Set SDKROOT on macOS if available
if [ -z "${SDKROOT:-}" ] && command -v xcrun >/dev/null 2>&1; then
  export SDKROOT="$(xcrun --show-sdk-path)"
fi

# Ensure Python dependencies (mako, pyyaml, packaging) are available
VENV_DIR="${ROOT_DIR}/.venv"
if ! python3 -c "import mako, yaml, packaging" >/dev/null 2>&1; then
  echo "==> Installing Mesa build dependencies in Python virtual environment..."
  if [ ! -d "${VENV_DIR}" ]; then
    python3 -m venv "${VENV_DIR}" || true
  fi
  if [ -x "${VENV_DIR}/bin/pip" ]; then
    "${VENV_DIR}/bin/pip" install mako pyyaml packaging
    export PATH="${VENV_DIR}/bin:${PATH}"
  else
    python3 -m pip install --break-system-packages mako pyyaml packaging || true
  fi
elif [ -d "${VENV_DIR}" ]; then
  export PATH="${VENV_DIR}/bin:${PATH}"
fi

# Download or clone Mesa source if not present
if [ ! -d "${MESA_SRC}" ]; then
  echo "==> Fetching Mesa ${MESA_VERSION}..."
  TARBALL="${DEPS_DIR}/mesa-${MESA_VERSION}.tar.xz"
  if curl -f -L --retry 3 --connect-timeout 30 -o "${TARBALL}" "https://archive.mesa3d.org/mesa-${MESA_VERSION}.tar.xz"; then
    echo "==> Extracting ${TARBALL}..."
    tar -xf "${TARBALL}" -C "${DEPS_DIR}"
    rm -rf "${MESA_SRC}"
    mv "${DEPS_DIR}/mesa-${MESA_VERSION}" "${MESA_SRC}"
    rm -f "${TARBALL}"
  else
    echo "==> Downloading tarball failed, cloning from git repository..."
    git clone --recurse-submodules --depth 1 --branch "mesa-${MESA_VERSION}" \
      https://gitlab.freedesktop.org/mesa/mesa.git "${MESA_SRC}"
  fi
fi

echo "==> Configuring Mesa (OSMesa + LLVMpipe)..."
rm -rf "${MESA_BUILD}"

meson setup "${MESA_BUILD}" "${MESA_SRC}" \
  --prefix="${MESA_INSTALL}" \
  --buildtype=release \
  --libdir=lib \
  -Db_ndebug=true \
  -Dosmesa=true \
  -Dgallium-drivers=llvmpipe \
  -Dvulkan-drivers=[] \
  -Dopengl=true \
  -Dgles1=disabled \
  -Dgles2=disabled \
  -Degl=disabled \
  -Dglx=disabled \
  -Dxlib-lease=disabled \
  -Dxmlconfig=disabled \
  -Dexpat=disabled \
  -Dplatforms=[] \
  -Dshared-glapi=disabled \
  -Dllvm=enabled \
  -Dshared-llvm=disabled

echo "==> Building and installing Mesa to ${MESA_INSTALL}..."
ninja -C "${MESA_BUILD}" install

echo "==> OSMesa build complete!"
echo "Installed in: ${MESA_INSTALL}"
ls -la "${MESA_INSTALL}/lib"
ls -la "${MESA_INSTALL}/include/GL"


