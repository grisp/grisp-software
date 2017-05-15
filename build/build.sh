#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

source "${SCRIPTDIR}/configuration.sh"
BUILD_DIR="${PROJECTDIR}/build/b-$BSP_NAME"
export PATH="${PREFIX}/bin:${PATH}"

# init git submodules
cd "${PROJECTDIR}"
git submodule init
git submodule update
cd "${LIBBSD_SOURCE_DIR}"
git submodule init
git submodule update rtems_waf

# build parts
cd "${PROJECTDIR}"
"${SCRIPTDIR}/build-toolchain.sh"
cd "${RTEMS_SOURCE_DIR}"
./bootstrap
cd "${PROJECTDIR}"
"${SCRIPTDIR}/build-bsp.sh" clean install
"${SCRIPTDIR}/build-libbsd.sh" clean install
"${SCRIPTDIR}/build-libinih.sh"
"${SCRIPTDIR}/build-libgrisp.sh"
