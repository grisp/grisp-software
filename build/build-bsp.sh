#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# Configuration
BSP_NAME="atsamv"
SOURCE_DIR="${PROJECTDIR}/rtems"
BUILD_DIR="${PROJECTDIR}/build/b-$BSP_NAME"
ARCH="arm"
RTEMS_VERSION="4.12"
TARGET="${ARCH}-rtems${RTEMS_VERSION}"
PREFIX="${PROJECTDIR}/rtems-install/rtems-${RTEMS_VERSION}/"
CONFIG_OPT=( \
	"--disable-tests" \
	"--disable-networking" \
	"--enable-chip=samv71q21" \
	"ATSAM_CONSOLE_DEVICE_TYPE=1" \
	"ATSAM_CONSOLE_DEVICE_INDEX=2" \
	"ATSAM_MEMORY_QSPIFLASH_SIZE=0x0")

# Path
export PATH="${PREFIX}/bin:${PATH}"

# Evaluate options
make_targets="all"
for i in "$@" ; do
	case "$i" in
	clean)
		rm -rf "$BUILD_DIR"
		;;
	install)
		make_targets="all install"
		;;
	*)
		echo "Unexpected option: '$i'"
		exit 1
		;;
	esac
done

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure
"${SOURCE_DIR}/configure" \
	"--target=${TARGET}" \
	"--prefix=${PREFIX}" \
	"--enable-rtemsbsp=${BSP_NAME}" \
	"--enable-maintainer-mode" \
	"${CONFIG_OPT[@]}"

# Make
make ${make_targets}
