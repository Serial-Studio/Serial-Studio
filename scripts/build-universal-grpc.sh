#!/usr/bin/env bash
#
# build-universal-grpc.sh
#
# Creates universal (arm64 + x86_64) libraries for gRPC and all transitive
# dependencies by installing both homebrew architectures and merging with lipo.
#
# The result is a self-contained prefix that CMake's find_package(gRPC CONFIG)
# can consume when passed via CMAKE_PREFIX_PATH.
#
# Usage: ./scripts/build-universal-grpc.sh [output-dir]
#   output-dir defaults to $GITHUB_WORKSPACE/grpc-universal (or ./grpc-universal)
#
# Requirements:
#   - macOS with Apple Silicon and Rosetta 2
#   - Native arm64 homebrew at /opt/homebrew
#   - Will bootstrap x86_64 homebrew at /usr/local if absent
#

set -euo pipefail

OUTPUT_DIR="${1:-${GITHUB_WORKSPACE:-$(pwd)}/grpc-universal}"
ARM64_PREFIX="/opt/homebrew"
X86_64_PREFIX="/usr/local"

echo "==> Output directory: ${OUTPUT_DIR}"

#-------------------------------------------------------------------------------
# 1. Install arm64 gRPC (native homebrew)
#-------------------------------------------------------------------------------
echo "==> Installing arm64 gRPC via native homebrew..."
brew install grpc protobuf

#-------------------------------------------------------------------------------
# 2. Bootstrap x86_64 homebrew and install gRPC
#-------------------------------------------------------------------------------
echo "==> Setting up x86_64 homebrew..."
if [ ! -f "${X86_64_PREFIX}/bin/brew" ]; then
  echo "==> Installing x86_64 homebrew at ${X86_64_PREFIX}..."
  arch -x86_64 /bin/bash -c \
    "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

echo "==> Installing x86_64 gRPC..."
arch -x86_64 "${X86_64_PREFIX}/bin/brew" install grpc protobuf

#-------------------------------------------------------------------------------
# 3. Resolve prefix paths
#-------------------------------------------------------------------------------
ARM64_LIB="${ARM64_PREFIX}/lib"
X86_64_LIB="${X86_64_PREFIX}/lib"
ARM64_INCLUDE="${ARM64_PREFIX}/include"
ARM64_CMAKE="${ARM64_PREFIX}/lib/cmake"

OPENSSL_ARM64=$("${ARM64_PREFIX}/bin/brew" --prefix openssl@3)
OPENSSL_X86_64=$(arch -x86_64 "${X86_64_PREFIX}/bin/brew" --prefix openssl@3)

#-------------------------------------------------------------------------------
# 4. Create output directory structure
#-------------------------------------------------------------------------------
rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}/lib/cmake" "${OUTPUT_DIR}/include" "${OUTPUT_DIR}/bin"

#-------------------------------------------------------------------------------
# 5. Copy headers (architecture-independent — use arm64 set)
#-------------------------------------------------------------------------------
echo "==> Copying headers..."
for dir in grpc grpc++ grpcpp google absl re2 upb upb_generator; do
  [ -d "${ARM64_INCLUDE}/${dir}" ] && cp -R "${ARM64_INCLUDE}/${dir}" "${OUTPUT_DIR}/include/"
done

# Single header files
for hdr in ares.h ares_build.h ares_dns.h ares_dns_record.h \
           ares_nameser.h ares_rules.h ares_version.h utf8_range.h; do
  [ -f "${ARM64_INCLUDE}/${hdr}" ] && cp "${ARM64_INCLUDE}/${hdr}" "${OUTPUT_DIR}/include/"
done

# OpenSSL headers
[ -d "${OPENSSL_ARM64}/include/openssl" ] && \
  cp -R "${OPENSSL_ARM64}/include/openssl" "${OUTPUT_DIR}/include/"

#-------------------------------------------------------------------------------
# 6. Copy host tools (arm64 is fine — they run on the build machine only)
#-------------------------------------------------------------------------------
echo "==> Copying tools..."
for tool in protoc grpc_cpp_plugin; do
  [ -f "${ARM64_PREFIX}/bin/${tool}" ] && cp "${ARM64_PREFIX}/bin/${tool}" "${OUTPUT_DIR}/bin/"
done

#-------------------------------------------------------------------------------
# 7. Lipo static and shared libraries into universal binaries
#-------------------------------------------------------------------------------
echo "==> Creating universal libraries with lipo..."
LIPO_COUNT=0
SKIP_COUNT=0

lipo_lib() {
  local arm64_path="$1"
  local libname
  libname=$(basename "${arm64_path}")

  # Skip symlinks — we recreate them later
  [ -L "${arm64_path}" ] && return

  local x86_path="${X86_64_LIB}/${libname}"
  if [ -f "${x86_path}" ]; then
    lipo -create "${arm64_path}" "${x86_path}" -output "${OUTPUT_DIR}/lib/${libname}"
    LIPO_COUNT=$((LIPO_COUNT + 1))
  else
    cp "${arm64_path}" "${OUTPUT_DIR}/lib/${libname}"
    SKIP_COUNT=$((SKIP_COUNT + 1))
  fi
}

# gRPC ecosystem libraries
for pat in grpc gpr address_sorting upb utf8 re2 cares protobuf absl_; do
  for f in "${ARM64_LIB}"/lib${pat}*.a "${ARM64_LIB}"/lib${pat}*.dylib; do
    [ -f "$f" ] && lipo_lib "$f"
  done
done

# OpenSSL
for ssl_lib in libssl libcrypto; do
  for ext in a dylib; do
    local_arm="${OPENSSL_ARM64}/lib/${ssl_lib}.${ext}"
    local_x86="${OPENSSL_X86_64}/lib/${ssl_lib}.${ext}"
    [ -f "${local_arm}" ] || continue
    [ -L "${local_arm}" ] && continue
    if [ -f "${local_x86}" ]; then
      lipo -create "${local_arm}" "${local_x86}" -output "${OUTPUT_DIR}/lib/${ssl_lib}.${ext}"
      LIPO_COUNT=$((LIPO_COUNT + 1))
    fi
  done
done

echo "==> Lipo'd ${LIPO_COUNT} libraries (${SKIP_COUNT} arm64-only fallbacks)"

#-------------------------------------------------------------------------------
# 8. Recreate dylib symlinks
#-------------------------------------------------------------------------------
echo "==> Recreating library symlinks..."
recreate_symlinks() {
  local src_dir="$1"
  local pattern="$2"
  for link in ${src_dir}/${pattern}; do
    [ -L "${link}" ] || continue
    local linkname target
    linkname=$(basename "${link}")
    target=$(basename "$(readlink "${link}")")
    [ -f "${OUTPUT_DIR}/lib/${target}" ] && \
      ln -sf "${target}" "${OUTPUT_DIR}/lib/${linkname}"
  done
}

for pat in "libgrpc*.dylib" "libgpr*.dylib" "libaddress_sorting*.dylib" \
           "libupb*.dylib" "libutf8*.dylib" "libre2*.dylib" "libcares*.dylib" \
           "libprotobuf*.dylib" "libabsl_*.dylib"; do
  recreate_symlinks "${ARM64_LIB}" "${pat}"
done

for pat in "libssl*.dylib" "libcrypto*.dylib"; do
  recreate_symlinks "${OPENSSL_ARM64}/lib" "${pat}"
done

#-------------------------------------------------------------------------------
# 9. Copy and patch CMake config files
#-------------------------------------------------------------------------------
echo "==> Copying CMake config files..."
for pkg in gRPC grpc absl utf8_range protobuf c-ares re2; do
  [ -d "${ARM64_CMAKE}/${pkg}" ] && cp -R "${ARM64_CMAKE}/${pkg}" "${OUTPUT_DIR}/lib/cmake/"
done

echo "==> Patching CMake configs..."
find "${OUTPUT_DIR}/lib/cmake" -name "*.cmake" -exec \
  sed -i '' -e "s|${ARM64_PREFIX}|${OUTPUT_DIR}|g" \
            -e "s|${OPENSSL_ARM64}|${OUTPUT_DIR}|g" {} +

#-------------------------------------------------------------------------------
# 10. Copy and patch pkg-config files
#-------------------------------------------------------------------------------
echo "==> Copying pkg-config files..."
mkdir -p "${OUTPUT_DIR}/lib/pkgconfig"
for pc in grpc grpc++ protobuf re2 libcares; do
  [ -f "${ARM64_LIB}/pkgconfig/${pc}.pc" ] && \
    cp "${ARM64_LIB}/pkgconfig/${pc}.pc" "${OUTPUT_DIR}/lib/pkgconfig/"
done
find "${OUTPUT_DIR}/lib/pkgconfig" -name "*.pc" -exec \
  sed -i '' "s|${ARM64_PREFIX}|${OUTPUT_DIR}|g" {} +

#-------------------------------------------------------------------------------
# Done
#-------------------------------------------------------------------------------
echo ""
echo "==> Universal gRPC installation ready at ${OUTPUT_DIR}"
echo "    Libraries: $(find "${OUTPUT_DIR}/lib" -name '*.a' -not -type l | wc -l | tr -d ' ') static"
echo "    CMake:     cmake -DCMAKE_PREFIX_PATH=\"${OUTPUT_DIR}\" ..."
