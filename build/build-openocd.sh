#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# configuration
. "${SCRIPTDIR}/configuration.sh"

# Build openocd
cd ${SCRIPTDIR}
rm -rf ./b-openocd
mkdir -p b-openocd
cd ${SCRIPTDIR}/b-openocd
tar -xaf ${SCRIPTDIR}/src/openocd-0.10.0.tar.bz2
cd "openocd-0.10.0"
# Disable most options. It will only increase the build time and the binary.
./configure --prefix="${PREFIX}" \
	--enable-ftdi \
	--disable-stlink \
	--disable-ti-icdi \
	--disable-ulink \
	--disable-usb-blaster-2 \
	--disable-vsllink \
	--disable-osbdm \
	--disable-opendous \
	--disable-aice \
	--disable-usbprog \
	--disable-rlink \
	--disable-armjtagew \
	--disable-cmsis-dap \
	--disable-usb-blaster \
	--disable-presto \
	--disable-openjtag \
	--disable-jlink \
	--disable-parport \
	--disable-parport-ppdev \
	--disable-parport-giveio \
	--disable-jtag_vpi \
	--disable-amtjtagaccel \
	--disable-zy1000-master \
	--disable-zy1000 \
	--disable-ioutil \
	--disable-ep93xx \
	--disable-at91rm9200 \
	--disable-bcm2835gpio \
	--disable-gw16012 \
	--disable-oocd_trace \
	--disable-buspirate \
	--disable-sysfsgpio \
	--disable-minidriver-dummy \
	--disable-remote-bitbang

make -j `nproc` install
