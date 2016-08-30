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

# Build and install libbsd
waf clean
waf configure \
	--prefix="${PREFIX}" \
	--rtems-bsps="${RTEMS_CPU}/${BSP_NAME}"
waf install

# part 2 of ugly workaround to build the tests with external RAM
mv "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds.org" "${PREFIX}/${TARGET}/${BSP_NAME}/lib/linkcmds"
