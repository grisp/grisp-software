#!/bin/sh

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

openocd -s "${TARGET}/share/openocd/scripts/" \
	-f "${SCRIPTDIR}/src/openocd-grisp.cfg" \
	-c "init" \
	-c "reset halt" \
	"$@"
