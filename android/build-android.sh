#!/bin/bash
set -e

BUILD_TYPE="android"
NDK_ROOT="${ANDROID_NDK_ROOT:-}"
ANDROID_ARCH="armeabi-v7a"
BUILD_DIR="build"
EXTRA_ARGS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --ndk-root)
            NDK_ROOT="$2"
            shift 2
            ;;
        --arch)
            ANDROID_ARCH="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        *)
            EXTRA_ARGS+=("$1")
            shift
            ;;
    esac
done

# if [[ -z "$NDK_ROOT" ]]; then
#     echo "Error: NDK root not specified. Use --ndk-root or set ANDROID_NDK_ROOT"
#     exit 1
# fi

# if [[ ! -d "$NDK_ROOT" ]]; then
#     echo "Error: NDK root directory does not exist: $NDK_ROOT"
#     exit 1
# fi

mkdir -p "$BUILD_DIR"

meson setup "$BUILD_DIR" \
	--cross-file "android/$ANDROID_ARCH.ini"
    "${EXTRA_ARGS[@]}"

ninja -C "$BUILD_DIR" -j$(nproc)

echo "Build completed successfully for Android ($ANDROID_ARCH)"
