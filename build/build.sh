#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# init git submodules
cd "${PROJECTDIR}"
git submodule init
git submodule update

# bootstrap rtems
cd "${PROJECTDIR}/rtems"
./bootstrap

# build parts
cd "${PROJECTDIR}"
"${PROJECTDIR}/build/build-toolchain.sh"
"${PROJECTDIR}/build/build-bsp.sh" install
