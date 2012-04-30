/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 2012, Joyent, Inc. All rights reserved.
 */

/*
 * Tool to lookup PCI IDs
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include "pcidb.h"

typedef enum lcase {
	L_VENDOR,
	L_DEVICE,
	L_SUBVD,
	L_INVALID
} lcase_t;

static int g_vid;
static int g_did;
static int g_svid;
static int g_sdid;
static lcase_t g_case;
static pcidb_hdl_t *g_hdl;

static void
fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(1);
}

static uint16_t
parse_int(const char *arg)
{
	char *end;
	long l;

	l = strtol(arg, &end, 16);
	if (*end != '\0')
		fatal("invalid pci id, not valid number: %s\n", arg);

	if (l > 0xffff || l < 0)
		fatal("value is outside the valid range [0, ffff]: %ld\n", l);

	return ((uint16_t)l);
}

/*
 * ON represents a pci id as vid,did.svid.sdid
 */
static void
parse_arg(char *arg)
{
	char *c;

	c = strchr(arg, ',');
	if (c == NULL) {
		g_vid = parse_int(arg);
		g_case = L_VENDOR;
		return;
	}

	*c = '\0';
	g_vid = parse_int(arg);
	arg = c + 1;
	c = strchr(arg, '.');
	if (c == NULL) {
		g_did = parse_int(arg);
		g_case = L_DEVICE;
		return;
	}

	*c = '\0';
	g_did = parse_int(arg);
	arg = c + 1;
	c = strchr(arg, '.');
	if (c == NULL)
		fatal("found what looks like a sub-vendor id but not a "
		    "sub-device id: %s\n", arg);

	*c = '\0';
	g_svid = parse_int(arg);
	arg = c + 1;
	g_sdid = parse_int(arg);
	g_case = L_SUBVD;
}

static void
print_arg(void)
{
	pcidb_vendor_t *v;
	pcidb_device_t *d;
	pcidb_subvd_t *s;
	const char *vend, *dev, *sub;
	vend = NULL;
	dev = NULL;
	sub = NULL;

	assert(g_case != L_INVALID);
	v = pcidb_lookup_vendor(g_hdl, g_vid);
	if (v == NULL)
		fatal("unknown vendor id: %04x\n", g_vid);

	vend = pcidb_vendor_name(v);
	if (g_case == L_VENDOR)
		goto print;

	d = pcidb_lookup_device_by_vendor(v, g_did);
	if (d == NULL)
		fatal("unknown device id: %04x\n", g_did);

	dev = pcidb_device_name(d);
	if (g_case == L_DEVICE)
		goto print;

	s = pcidb_lookup_subvd_by_device(d, g_svid, g_sdid);
	if (s == NULL)
		fatal("uknown sub-vendor and sub-device id: %04x.%04x\n",
		    g_svid, g_sdid);
	sub = pcidb_subvd_name(s);
print:
	if (vend != NULL)
		printf("%s", vend);

	if (dev != NULL)
		printf("|%s", dev);

	if (sub != NULL)
		printf("|%s", sub);

	printf("\n");
}


int
main(int argc, char *argv[])
{
	int ii;
	int ret = 0;

	if (argc < 2) {
		fprintf(stderr, "arguments plox\n");
		exit(1);
	}

	g_hdl = pcidb_open(PCIDB_VERSION);
	if (g_hdl == NULL)
		fatal("failed to open pci database: %s\n", strerror(errno));

	for (ii = 1; ii < argc; ii++) {
		g_case = L_INVALID;
		parse_arg(argv[ii]);
		print_arg();
	}


	pcidb_close(g_hdl);

	return (ret);
}
