/*
 * Copyright (c) 2018 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "fatbench.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rtems/blkdev.h>
#include <rtems/bsd.h>
#include <rtems/dosfs.h>
#include <rtems/printer.h>

static char buf[32 * 1024];

static rtems_printer printer;

static double
now(void)
{
	double t;
	struct timespec ts;

	rtems_bsd_nanouptime(&ts);
	t = (double)ts.tv_sec;
	t += (double)ts.tv_nsec / 1000000000.0;
	return (t);
}

static void
create_file(const char *path, size_t len)
{
	double t0;
	double t1;
	double d;
	int rv;
	int fd;
	ssize_t n;
	size_t todo;

	t0 = now();

	fd = open(path, O_CREAT | O_TRUNC | O_WRONLY);
	assert(fd >= 0);

	todo = len;
	while (todo > 0) {
		size_t junk;

		if (todo > sizeof(buf)) {
			junk = sizeof(buf);
		} else {
			junk = todo;
		}

		n = write(fd, buf, junk);
		assert(n == (ssize_t) junk);

		todo -= junk;
	}

	rv = close(fd);
	assert(rv == 0);

	t1 = now();
	d = t1 - t0;
	printf("write %zu in %.2fs (%.2fKiB/s)\n", len, d, (double)(len / 1024) / d);
}

static size_t
read_file(const char *path)
{
	double t0;
	double t1;
	double d;
	int rv;
	int fd;
	size_t len;

	t0 = now();
	len = 0;

	fd = open(path, O_RDONLY);
	assert(fd >= 0);

	while (true) {
		ssize_t n;

		n = read(fd, buf, sizeof(buf));
		if (n <= 0) {
			break;
		}

		len += (size_t)n;
	}

	rv = close(fd);
	assert(rv == 0);

	t1 = now();
	d = t1 - t0;
	printf("read %zu in %.2fs (%.2fKiB/s)\n", len, d, (double)(len / 1024) / d);

	return (len);
}

static void
format(const char *dev_path, const char *mount_path, uint32_t sectors_per_cluster)
{
	msdos_format_request_param_t rqdata;
	int rv;

	unmount(mount_path);

	memset(&rqdata, 0, sizeof(rqdata));
	rqdata.sectors_per_cluster = sectors_per_cluster;
	rqdata.quick_format = true;
	rv = msdos_format(dev_path, &rqdata);
	assert(rv == 0);

	rv = mount_and_make_target_path(dev_path, mount_path,
	    RTEMS_FILESYSTEM_TYPE_DOSFS, RTEMS_FILESYSTEM_READ_WRITE, NULL);
	assert(rv == 0);
}

static void
simple_write_read(const char *dev_path, const char *mount_path,
    uint32_t sectors_per_cluster, size_t len)
{
	static const char file[] = "test.bin";
	int rv;
	size_t actual_len;

	printf("\nformat with %" PRIu32 " sectors per cluster\n",
	    sectors_per_cluster);
	format(dev_path, mount_path, sectors_per_cluster);

	rv = chdir(mount_path);
	assert(rv == 0);

	rtems_blkstats(&printer, dev_path, true);
	create_file(file, len);
	rtems_blkstats(&printer, dev_path, false);

	rtems_blkstats(&printer, dev_path, true);
	actual_len = read_file(file);
	assert(len == actual_len);
	rtems_blkstats(&printer, dev_path, false);

	rv = chdir("/");
	assert(rv == 0);
}

void
fatbench(const char *dev_path, const char *mount_path)
{
	uint32_t spc;
	size_t len;

	rtems_print_printer_fprintf(&printer, stdout);
	len = 32 * 1024 * 1024;

	for (spc = 1; spc <= 64; spc *= 2) {
		simple_write_read(dev_path, mount_path, spc, len);
	}
}
