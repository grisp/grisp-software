#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTPATH=$(readlink -- "$0" || echo "$0")
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$SCRIPTPATH")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# Configuration
. "${SCRIPTDIR}/configuration.sh"
export PATH="${PREFIX}/bin:${PATH}"

cd "${LIBBSD_SOURCE_DIR}"

if [ "$BSP_NAME" = "atsamv" ]
then
	# ugly workaround to build the tests with external RAM
	if [ ! -e "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.org" ]
	then
		mv "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds" \
		    "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.org"
	fi
	cp "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.sdram" \
	    "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds"
fi

# Evaluate options
DO_CLEAN=0
DO_INSTALL=0
for i in "$@" ; do
	case "$i" in
	clean)
		DO_CLEAN=1
		;;
	install)
		DO_INSTALL=1
		;;
	*)
		echo "Unexpected option: '$i'"
		exit 1
		;;
	esac
done

# Build and install libbsd
if [ $DO_CLEAN -ne 0 ]
then
	if [ -e build ]
	then
		rm -rf build
	fi
fi

waf configure \
	--prefix="${PREFIX}" \
	--rtems-bsps="${RTEMS_CPU}/${BSP_NAME}" \
	--buildset="${PROJECTDIR}/build/src/libbsd.ini"
waf

if [ $DO_INSTALL -ne 0 ]
then
	waf install
fi

if [ "$BSP_NAME" = "atsamv" ]
then
	# part 2 of ugly workaround to build the tests with external RAM
	mv "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.org" \
	    "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds"
fi
