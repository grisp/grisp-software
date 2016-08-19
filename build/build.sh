#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

source "${SCRIPTDIR}/configuration.sh"

# init git submodules
cd "${PROJECTDIR}"
git submodule init
git submodule update

# bootstrap rtems
cd "${RTEMS_SOURCE_DIR}"
./bootstrap

# build parts
cd "${PROJECTDIR}"
"${SCRIPTDIR}/build-toolchain.sh"
"${SCRIPTDIR}/build-bsp.sh" install
