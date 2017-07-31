#! /usr/bin/env sh

# ****************************************
# NORMALLY YOU SHOULD NOT NEED THIS SCRIPT
# ****************************************

# This script set's up an environment that can be used for building RTEMS for
# GRiSP manually. This is only necessary for RTEMS system development. You
# should never need it to build an application.

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# Configuration
source "${SCRIPTDIR}/configuration.sh"
ADDPATH=`readlink -f "${PREFIX}/bin"`
export PATH="${ADDPATH}:${PATH}"

export PREPROMPT="%B(GRISP-RTEMS)%b "

${SHELL}
