#!/bin/sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

ELFFILE=$1

${SCRIPTDIR}/debug-start-openocd.sh -c "program \"${ELFFILE}\" verify reset exit"
