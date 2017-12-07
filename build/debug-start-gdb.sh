#!/bin/sh

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
BUILD_DIR="${PROJECTDIR}/build/b-$BSP_NAME"
export PATH="${PREFIX}/bin:${PATH}"

arm-rtems4.12-gdb -x ${SCRIPTDIR}/src/start.gdb "$@"
