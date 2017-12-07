#!/bin/sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTPATH=$(readlink -- "$0" || echo "$0")
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$SCRIPTPATH")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

ELFFILE=$1

${SCRIPTDIR}/debug-start-openocd.sh \
	-c "reset" -c "halt" -c "program \"${ELFFILE}\"" \
	-c "reset" -c "halt" -c "verify_image_checksum \"${ELFFILE}\"" \
	-c "reset" -c "exit"
