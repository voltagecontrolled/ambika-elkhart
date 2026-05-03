#!/usr/bin/env bash
# Run a make target inside the Squeeze (avr-gcc 4.3.5) container — CI variant.
# Mirrors toolchain/build-squeeze.sh, but assumes the image is named
# ambika-squeeze:ci and the workflow has already built it. Network is
# disabled for the compile step; the image must be cached/built beforehand.
#
# Usage: squeeze-make.sh <target-makefile>   (e.g. controller/makefile)

set -euo pipefail

TARGET="${1:?usage: $0 <target-makefile>}"
IMAGE="ambika-squeeze:ci"

[[ -f "$TARGET" ]] || { echo "error: $TARGET not found" >&2; exit 1; }

exec docker run --rm \
  --platform linux/amd64 \
  --network none \
  --cap-drop=ALL \
  --user "$(id -u):$(id -g)" \
  -v "$PWD:/work" \
  -w /work \
  "$IMAGE" \
  make -f "$TARGET" bin \
    AVRLIB_TOOLS_PATH=/usr/bin/ \
    AR=/usr/bin/avr-ar \
    OBJCOPY=/usr/bin/avr-objcopy \
    OBJDUMP=/usr/bin/avr-objdump \
    SIZE=/usr/bin/avr-size \
    NM=/usr/bin/avr-nm
