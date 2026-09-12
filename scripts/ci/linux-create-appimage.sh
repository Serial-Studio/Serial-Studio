#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# Spec 0065: the glibc-bundled conversion is the ONLY Linux packaging path -- every Linux
# artifact ships the build host's glibc as a matched set behind an in-process loader redirect,
# so one package set runs on everything from Debian 10 / RHEL 8 up. A COPY of the AppDir is
# converted with sharun/lib4bin: sharun hands library dirs to the bundled ld-linux via
# --library-path instead of exporting LD_LIBRARY_PATH, so QProcess children keep a clean host
# environment. The full library closure (executables plus every dlopened plugin/QML module) is
# consolidated into usr/shared/lib because moving the executables breaks their $ORIGIN-relative
# RUNPATH; usr/lib is dropped only after a coverage check proves every library it held is
# present there.
#
# Driven by .github/actions/package-linux. CWD-independent.
#
# Environment:
#   SS_WORKSPACE          repository checkout the build tree lives under
#   MACHINE_ARCH          sharun / appimagetool asset architecture (x86_64 / aarch64)
#   CONFIG_ARCH           arch stamped into ss-config.json (x86_64 / arm64)
#   ARCH_LABEL            artifact-name architecture (x64 / arm64)
#   SYSTEM_LIB_DIR        host library triplet dir the glibc NSS and gconv sets come from
#   SHARUN_SHA256         checksum of the sharun build for MACHINE_ARCH
#   APPIMAGETOOL_SHA256   checksum of the appimagetool build for MACHINE_ARCH
#   EXECUTABLE            artifact base name
#   VERSION               release version
#   UNIXNAME              built executable name inside the AppDir
#   SS_GPG_SIGN           '1' when gpg-prepare.sh imported an identity
#   GPG_KEY_ID            key the AppImage is signed with when SS_GPG_SIGN is '1'
#
set -euo pipefail
cd "${SS_WORKSPACE}/build/app"

SHARUN_VERSION=v0.8.1
APPIMAGETOOL_VERSION=1.9.1

wget -q "https://github.com/VHSgunzo/sharun/releases/download/${SHARUN_VERSION}/sharun-${MACHINE_ARCH}-aio" -O sharun-aio
echo "${SHARUN_SHA256}  sharun-aio" | sha256sum -c -
chmod +x sharun-aio
wget -q "https://github.com/AppImage/appimagetool/releases/download/${APPIMAGETOOL_VERSION}/appimagetool-${MACHINE_ARCH}.AppImage" -O appimagetool
echo "${APPIMAGETOOL_SHA256}  appimagetool" | sha256sum -c -
chmod +x appimagetool

cp -a AppDir AppDir-compat
printf '{"packageType":"appimage","arch":"%s"}\n' "${CONFIG_ARCH}" > AppDir-compat/usr/bin/ss-config.json

BIN_SRC=$(mktemp -d)
mv "AppDir-compat/usr/bin/${UNIXNAME}" "$BIN_SRC/"
WEBPROC_DIR=""
WEBPROC=$(find AppDir-compat/usr -type f -name QtWebEngineProcess | head -n1 || true)
if [ -n "$WEBPROC" ]; then
  WEBPROC_DIR=$(dirname "$WEBPROC")
  mv "$WEBPROC" "$BIN_SRC/"
fi

# usr/lib rides along as explicit input: libraries that are only dlopened (NSS's
# libsoftokn3 and friends) are reachable from no executable or plugin, so ldd-driven
# dependency walking alone would drop them and their private deps (libsqlite3).
DLOPEN_LIBS=$(find AppDir-compat/usr/plugins AppDir-compat/usr/qml AppDir-compat/usr/lib \
  -type f -name '*.so*' 2>/dev/null || true)

# Pre-create shared/lib as a real directory: when it is absent and usr/lib exists,
# lib4bin symlinks shared/lib -> ../lib, so the whole closure lands in usr/lib and
# the coverage checks below then delete the only copy.
mkdir -p AppDir-compat/usr/shared/lib

# lib4bin resolves dependencies with ldd and silently drops unresolved ones; the
# executables were moved out of the AppDir, which breaks their $ORIGIN RUNPATH, so
# the libraries only they need (Widgets, Bluetooth, Mqtt, mimalloc, ...) vanish from
# the closure unless the AppDir library dir is on LD_LIBRARY_PATH for the conversion.
# Qt's openssl TLS backend dlopens libssl at runtime (no NEEDED entry), so the ldd
# walk never sees it; on OpenSSL 1.1 hosts (Debian 10) the backend then rejects the
# host library and every HTTPS request fails. Bundle the build host's OpenSSL 3 pair
# explicitly; it is safe because the compat bundle carries the matching glibc.
TLS_LIBS=$(find /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib64 \
  -maxdepth 1 \( -name 'libssl.so.3*' -o -name 'libcrypto.so.3*' \) 2>/dev/null || true)
if [ -z "$TLS_LIBS" ]; then
  echo "::error::libssl.so.3/libcrypto.so.3 not found on build host"
  exit 1
fi

# The closure carries GLVND's dispatch libs (libGLX/libEGL/libOpenGL) but those
# resolve the actual vendor driver by dlopen, and sharun's loader never searches the
# host's /usr/lib — with no bundled vendor the GLX/EGL vendor list is empty and Qt
# aborts with "Could not initialize GLX" on every desktop host (offscreen smoke runs
# never see it). Bundle Mesa's vendor libs and DRI drivers explicitly (dlopen-only:
# the ldd walk cannot find them); sharun auto-sets LIBGL_DRIVERS_PATH,
# __EGL_VENDOR_LIBRARY_DIRS and GBM_BACKENDS_PATH when the bundled dirs exist.
# NVIDIA-driver hosts fall back to bundled Mesa instead of aborting.
sudo apt-get install -y -q libgl1-mesa-dri libglx-mesa0 libegl-mesa0 >/dev/null
GL_VENDOR_LIBS=$(find /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib64 \
  -maxdepth 1 \( -name 'libGLX_mesa.so*' -o -name 'libEGL_mesa.so*' \
  -o -name 'libglapi.so*' -o -name 'libgallium*.so*' -o -name 'libgbm.so*' \) \
  2>/dev/null || true)
if [ -z "$GL_VENDOR_LIBS" ]; then
  echo "::error::Mesa GLX/EGL vendor libraries not found on build host"
  exit 1
fi
DRI_DIR=$(find /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib64 \
  -maxdepth 1 -type d -name dri 2>/dev/null | head -n1 || true)
if [ -z "$DRI_DIR" ] || ! ls "$DRI_DIR"/*.so >/dev/null 2>&1; then
  echo "::error::Mesa DRI driver directory not found on build host"
  exit 1
fi
GBM_DIR=$(find /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib64 \
  -maxdepth 1 -type d -name gbm 2>/dev/null | head -n1 || true)
GBM_LIBS=""
[ -n "$GBM_DIR" ] && GBM_LIBS=$(ls "$GBM_DIR"/*.so 2>/dev/null || true)

# The bundled libasound carries the build host's plugin dir baked in
# (/usr/lib/<triplet>/alsa-lib), which does not exist on RHEL-family hosts; their
# alsa.conf routes the default PCM through "pulse", so every device probe logs a
# missing libasound_module_pcm_pulse.so and falls back to raw hw. Bundle the pulse
# plugins and point ALSA_PLUGIN_DIR at them through sharun's .env (PipeWire hosts
# reach them via pipewire-pulse).
sudo apt-get install -y -q libasound2-plugins >/dev/null
ALSA_PLUGIN_SRC=$(find /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu /usr/lib64 \
  -maxdepth 1 -type d -name alsa-lib 2>/dev/null | head -n1 || true)
ALSA_PLUGINS=""
[ -n "$ALSA_PLUGIN_SRC" ] && ALSA_PLUGINS=$(ls "$ALSA_PLUGIN_SRC"/libasound_module_*_pulse.so \
  2>/dev/null || true)
if [ -z "$ALSA_PLUGINS" ]; then
  echo "::error::ALSA pulse plugins not found on build host"
  exit 1
fi

LD_LIBRARY_PATH="$PWD/AppDir-compat/usr/lib" \
  ./sharun-aio lib4bin --dst-dir AppDir-compat/usr --with-sharun --hard-links --with-hooks \
  "$BIN_SRC"/* $DLOPEN_LIBS $TLS_LIBS $GL_VENDOR_LIBS "$DRI_DIR"/*.so $GBM_LIBS \
  $ALSA_PLUGINS

mkdir -p AppDir-compat/usr/shared/lib/alsa-lib
for f in $ALSA_PLUGINS; do
  b=$(basename "$f")
  dst="AppDir-compat/usr/shared/lib/alsa-lib/$b"
  if [ ! -e "$dst" ]; then
    src="AppDir-compat/usr/shared/lib/$b"
    [ -f "$src" ] && ln -f "$src" "$dst"
  fi
  if [ ! -e "$dst" ]; then
    echo "::error::ALSA plugin $b missing from compat closure"
    exit 1
  fi
done
if ! grep -q '^ALSA_PLUGIN_DIR=' AppDir-compat/usr/.env 2>/dev/null; then
  echo 'ALSA_PLUGIN_DIR=${SHARUN_DIR}/shared/lib/alsa-lib' >> AppDir-compat/usr/.env
fi

if ! find AppDir-compat/usr/shared/lib -name 'libssl.so.3*' | grep -q .; then
  echo "::error::libssl.so.3 missing from compat bundle; TLS would fail on legacy hosts"
  exit 1
fi

if [ -L AppDir-compat/usr/shared/lib ]; then
  echo "::error::usr/shared/lib is a symlink into usr/lib; compat conversion unsafe"
  exit 1
fi

# Verify (and backfill) the vendor discovery layout sharun keys on: DRI drivers
# under shared/lib/dri, GBM backends under shared/lib/gbm, and the GLVND EGL
# vendor JSON under usr/share/glvnd with its library_path reduced to a bare
# soname so it resolves inside the bundle. lib4bin keys destinations off the
# SOURCE dir, so Mesa 25's symlinked per-driver names already land in
# shared/lib/dri pointing at the flattened dril/gallium megadriver — the loops
# only create the per-name link when lib4bin left the driver flattened in the
# closure root (pre-DRIL Mesa), and fail on names resolvable in neither spot.
mkdir -p AppDir-compat/usr/shared/lib/dri
for f in "$DRI_DIR"/*.so; do
  b=$(basename "$f")
  dst="AppDir-compat/usr/shared/lib/dri/$b"
  if [ ! -e "$dst" ]; then
    src="AppDir-compat/usr/shared/lib/$b"
    [ -f "$src" ] || src="AppDir-compat/usr/shared/lib/$(basename "$(readlink -f "$f")")"
    [ -f "$src" ] && ln -f "$src" "$dst"
  fi
  if [ ! -e "$dst" ]; then
    echo "::error::DRI driver $b missing from compat closure"
    exit 1
  fi
done
if [ -n "$GBM_LIBS" ]; then
  mkdir -p AppDir-compat/usr/shared/lib/gbm
  for f in $GBM_LIBS; do
    b=$(basename "$f")
    dst="AppDir-compat/usr/shared/lib/gbm/$b"
    [ -e "$dst" ] && continue
    src="AppDir-compat/usr/shared/lib/$b"
    [ -f "$src" ] || src="AppDir-compat/usr/shared/lib/$(basename "$(readlink -f "$f")")"
    [ -f "$src" ] && ln -f "$src" "$dst"
  done
fi
if [ ! -d /usr/share/glvnd/egl_vendor.d ]; then
  echo "::error::/usr/share/glvnd/egl_vendor.d not found on build host"
  exit 1
fi
mkdir -p AppDir-compat/usr/share/glvnd
cp -a /usr/share/glvnd/egl_vendor.d AppDir-compat/usr/share/glvnd/
sed -i -e 's|/usr/lib.*/||g' AppDir-compat/usr/share/glvnd/egl_vendor.d/*.json
grep -rq 'libEGL_mesa' AppDir-compat/usr/share/glvnd/egl_vendor.d
for probe in libGLX_mesa.so.0 libEGL_mesa.so.0; do
  if ! find AppDir-compat/usr/shared/lib -maxdepth 1 -name "$probe*" | grep -q .; then
    echo "::error::$probe missing from compat bundle; GL would abort on desktop hosts"
    exit 1
  fi
done

if [ -n "$WEBPROC_DIR" ] && [ "$WEBPROC_DIR" != "AppDir-compat/usr/bin" ]; then
  ln -f AppDir-compat/usr/bin/QtWebEngineProcess "$WEBPROC_DIR/QtWebEngineProcess"
fi

for m in libsoftokn3 libfreebl3 libfreeblpriv3 libnssckbi libnssdbm3; do
  src="AppDir-compat/usr/lib/$m.so"
  dst="AppDir-compat/usr/shared/lib/$m.so"
  [ -f "$src" ] || continue
  [ -e "$dst" ] || mv "$src" "$dst"
  rm -f "$src"
done

for m in libnss_files.so.2 libnss_dns.so.2 libnss_resolve.so.2; do
  [ -e "AppDir-compat/usr/shared/lib/$m" ] && continue
  src=$(find "${SYSTEM_LIB_DIR}" /usr/lib64 -maxdepth 1 -name "$m" 2>/dev/null | head -n1 || true)
  [ -n "$src" ] && cp -a "$src" AppDir-compat/usr/shared/lib/ || true
done
for m in libnss_files.so.2 libnss_dns.so.2; do
  if [ ! -e "AppDir-compat/usr/shared/lib/$m" ]; then
    echo "::error::$m missing; bundled glibc NSS set incomplete"
    exit 1
  fi
done
if [ ! -d AppDir-compat/usr/shared/lib/gconv ]; then
  gconv=$(find "${SYSTEM_LIB_DIR}/gconv" /usr/lib64/gconv -maxdepth 0 -type d 2>/dev/null | head -n1 || true)
  if [ -z "$gconv" ]; then
    echo "::error::gconv modules not found on build host"
    exit 1
  fi
  cp -a "$gconv" AppDir-compat/usr/shared/lib/gconv
fi
if ! find AppDir-compat/usr/shared/lib -name 'ld-linux*.so*' | grep -q .; then
  echo "::error::bundled dynamic loader missing; compat conversion failed"
  exit 1
fi

missing=0
for lib in AppDir-compat/usr/lib/*.so*; do
  [ -e "$lib" ] || continue
  stem=$(basename "$lib")
  stem="${stem%%.so*}"
  if ! find AppDir-compat/usr/shared/lib -maxdepth 1 -name "${stem}.so*" | grep -q .; then
    echo "::error::${stem} from usr/lib has no shared/lib counterpart"
    missing=1
  fi
done
[ "$missing" = 0 ] || exit 1
rm -rf AppDir-compat/usr/lib
rm -rf "$BIN_SRC"

APPIMAGE="${EXECUTABLE}-${VERSION}-Linux-${ARCH_LABEL}.AppImage"
if [ "${SS_GPG_SIGN:-0}" = "1" ]; then
  ARCH=${MACHINE_ARCH} ./appimagetool --sign --sign-key "$GPG_KEY_ID" AppDir-compat "$APPIMAGE"
else
  ARCH=${MACHINE_ARCH} ./appimagetool AppDir-compat "$APPIMAGE"
fi
rm sharun-aio appimagetool
mv "$APPIMAGE" "../../$APPIMAGE"
