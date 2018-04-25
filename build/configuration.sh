# This script is expected to be sourced. It expects the following variables to
# be already set:
#   $PROJECTDIR -- set to the base dir of the project

BSP_NAME="atsamv"
RTEMS_CPU="arm"
RTEMS_VERSION="5"
TARGET="${RTEMS_CPU}-rtems${RTEMS_VERSION}"
PREFIX="${PROJECTDIR}/rtems-install/rtems/${RTEMS_VERSION}/"

RTEMS_SOURCE_DIR="${PROJECTDIR}/rtems"
LIBBSD_SOURCE_DIR="${PROJECTDIR}/rtems-libbsd"
LIBINIH_SOURCE_DIR="${PROJECTDIR}/libinih"
LIBGRISP_SOURCE_DIR="${PROJECTDIR}/libgrisp"

BSP_CONFIG_OPT=( \
	"--disable-tests" \
	"--disable-networking" \
	"--enable-chip=same70q21" \
	"--enable-sdram=is42s16320f-7bl" \
	"ATSAM_CONSOLE_DEVICE_TYPE=1" \
	"ATSAM_CONSOLE_DEVICE_INDEX=2" \
	"ATSAM_MEMORY_QSPIFLASH_SIZE=0x0" \
	"ATSAM_MEMORY_NOCACHE_SIZE=0x8000")
