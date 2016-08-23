GRISP
=====

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
