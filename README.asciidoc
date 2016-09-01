GRISP
=====

== Quick Start Guide

You can build the tools by just using +./build/build.sh+. This will do the
following:

- check out the necessary git submodules
- bootstrap RTEMS
- build and install the toolchain
- build and install the RTEMS BSP
- build and install necessary libs

All installations are made inside the +rtems-install+ subdirectory in the base
directory of the repository. To change the install location edit the +PREFIX+ in
+build/configuration.sh+.

After this, you can build the simple RTEMS sample application. Go to
+grisp-simple-sample+ and call +make+.

== Boot Loader

The boot loader will try to initialize and mount the SD card. In case this is
successful it tries to read the +grisp.ini+ configuration file from the SD root
directory.

. Sample grisp.ini (showing the default values)
--------------------------------------------------
[boot]
timeout_in_seconds = 3

[file]
image_path = /ffs/br_uid.bin
--------------------------------------------------

All values are optional and in case something is missing default values will be
used (presented in the listing above). Once the timeout expired without user
input the automatic application load sequence starts.
