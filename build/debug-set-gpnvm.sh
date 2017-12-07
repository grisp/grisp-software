#!/bin/sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTPATH=$(readlink -- "$0" || echo "$0")
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$SCRIPTPATH")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

${SCRIPTDIR}/debug-start-openocd.sh \
	-c "reset" -c "halt" -c "mww 0x400e0c04 0x5a00010b"\
	-c "reset" -c "exit"
