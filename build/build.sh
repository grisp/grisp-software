#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTPATH=$(readlink -- "$0" || echo "$0")
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$SCRIPTPATH")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

. "${SCRIPTDIR}/configuration.sh"
BUILD_DIR="${PROJECTDIR}/build/b-$BSP_NAME"
export PATH="${PREFIX}/bin:${PATH}"

# Evaluate options
DO_SUBMODULES=1
DO_TOOLCHAIN=1
DO_BOOTSTRAP=1
for i in "$@" ; do
	case "$i" in
	--no-submodules)
		DO_SUBMODULES=0
		;;
	--no-toolchain)
		DO_TOOLCHAIN=0
		;;
	--no-bootstrap)
		DO_BOOTSTRAP=0
		;;
	*)
		echo "Unexpected option: '$i'"
		exit 1
		;;
	esac
done

if [ ${DO_SUBMODULES} -ne 0 ]
then
	# init git submodules
	cd "${PROJECTDIR}"
	git submodule init
	git submodule update
	cd "${LIBBSD_SOURCE_DIR}"
	git submodule init
	git submodule update rtems_waf
fi

# build parts
cd "${PROJECTDIR}"
if [ ${DO_TOOLCHAIN} -ne 0 ]
then
	"${SCRIPTDIR}/build-toolchain.sh"
fi
if [ ${DO_BOOTSTRAP} -ne 0 ]
then
	cd "${RTEMS_SOURCE_DIR}"
	./bootstrap
fi
cd "${PROJECTDIR}"
"${SCRIPTDIR}/build-bsp.sh" clean install
"${SCRIPTDIR}/build-libbsd.sh" clean install
"${SCRIPTDIR}/build-libinih.sh"
"${SCRIPTDIR}/build-libgrisp.sh"
