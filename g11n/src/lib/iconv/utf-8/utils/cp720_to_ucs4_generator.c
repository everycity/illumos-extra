/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").  
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma	ident	"@(#)cp720_to_ucs4_generator.c	1.0	07/12/03 SMI"


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include "../common_defs.h"

int
main(int ac, char **av)
{
	to_utf8_table_component_t tbl[256];
	register int i, j;
	char buf[BUFSIZ], num[100];
	unsigned int l, k;
	char ascii_only = 0;

	if (ac > 1 && strcmp(av[1], "-ascii") == 0)
		ascii_only = 1;

	for (i = 0; i < 256; i++) {
		if (i <= 0x1f || i == 0x7f || (ascii_only && i <= 0x7f)) {
			tbl[i].size = 1;
			tbl[i].u8 = (unsigned int)i;
		} else if (!ascii_only && (i >= 0x80 && i <= 0x9f)) {
			tbl[i].size = 2;
			tbl[i].u8 = (unsigned int)i;
		} else {
			tbl[i].size = ICV_TYPE_ILLEGAL_CHAR;
			tbl[i].u8 = 0;
		}
	}


	while (fgets(buf, BUFSIZ, stdin)) {
		i = 0;
		while (buf[i] && isspace(buf[i]))
			i++;

		if (strncmp(buf + i, "<U", 2) != 0)
			continue;

		i += 2;
		for (j = 0; isxdigit(buf[i]); i++, j++)
			num[j] = buf[i];
		num[j] = '\0';

		l = strtol(num, (char **)NULL, 16);

		while (!isxdigit(buf[i]))
			i++;

		for (j = 0; isxdigit(buf[i]); i++, j++)
			num[j] = buf[i];
		num[j] = '\0';

		k = strtol(num, (char **)NULL, 16);

		while (buf[i] == ' ' || buf[i] == '\t')
			i++;

		if (buf[i] && buf[i] == '|' && buf[i + 1] && buf[i + 1] != '0')
			continue;

		tbl[k].u8 = l;
		if (l < 0x80)
			tbl[k].size = 1;
		else if (l < 0x800)
			tbl[k].size = 2;
		else if (l < 0x10000)
			tbl[k].size = 3;
		else if (l < 0x200000)
			tbl[k].size = 4;
		else if (l < 0x4000000)
			tbl[k].size = 5;
		else
			tbl[k].size = 6;
	}

	for (i = 0; i < 256; i++) {
		l = tbl[i].u8;
		printf("/* 0x%02X */  {  0x%08X, %-3d},\n", i, l, tbl[i].size);
	}
}
