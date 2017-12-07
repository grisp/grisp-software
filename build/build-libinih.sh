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

cd "${LIBINIH_SOURCE_DIR}"
make clean install
