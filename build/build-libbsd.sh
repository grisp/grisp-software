#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# Configuration
source "${SCRIPTDIR}/configuration.sh"
export PATH="${PREFIX}/bin:${PATH}"

cd "${LIBBSD_SOURCE_DIR}"

# ugly workaround to build the tests with external RAM
if [ ! -e "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.org" ]
then
	mv "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds" "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.org"
fi
cp "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.sdram" "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds"

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
		waf clean
	fi
fi

waf configure \
	--prefix="${PREFIX}" \
	--rtems-bsps="${RTEMS_CPU}/${BSP_NAME}"
waf

if [ $DO_INSTALL -ne 0 ]
then
	waf install
fi

# part 2 of ugly workaround to build the tests with external RAM
mv "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.org" "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds"
