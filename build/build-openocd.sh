#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# configuration
source "${SCRIPTDIR}/configuration.sh"
# Do not use rtems autoconfig. It is lacking libtool and pkg-config.
# export PATH="${PREFIX}/bin:${PATH}"

# Build openocd
cd "${PROJECTDIR}/openocd"
git clean -dxf .
./bootstrap
./configure --prefix="${PREFIX}"
make install
