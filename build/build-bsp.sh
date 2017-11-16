#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# Configuration
. "${SCRIPTDIR}/configuration.sh"
BUILD_DIR="${PROJECTDIR}/build/b-$BSP_NAME"
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
"${RTEMS_SOURCE_DIR}/configure" \
	"--target=${TARGET}" \
	"--prefix=${PREFIX}" \
	"--enable-rtemsbsp=${BSP_NAME}" \
	"--enable-maintainer-mode" \
	${BSP_CONFIG_OPT}

# Make
make ${make_targets}

# Generate .mk file
mkdir -p "${PREFIX}/make/custom/"
cat "${PROJECTDIR}/build/src/bsp.mk" | \
	sed 	-e "s/##RTEMS_API##/$RTEMS_VERSION/g" \
		-e "s/##RTEMS_BSP##/$BSP_NAME/g" \
		-e "s/##RTEMS_CPU##/$RTEMS_CPU/g" \
	> "${PREFIX}/make/custom/${BSP_NAME}.mk"
