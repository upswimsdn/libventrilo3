/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
 *
 * Copyright 2009-2010 Eric Kilfoil
 *
 * This file is part of libventrilo3.
 *
 * libventrilo3 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libventrilo3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libventrilo3.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ventrilo3.h>

int
main(int argc, char **argv) {
	v3_handle v3h;

    if (argc < 3) {
        fprintf(stderr, "%s hostname:port username\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    v3_debug(V3_HANDLE_NONE, V3_DBG_ALL);
	if ((v3h = v3_init(argv[1], argv[2])) == V3_HANDLE_NONE) {
	    fprintf(stderr, "v3_init() error: %s\n", v3_error(v3h));
	    exit(EXIT_FAILURE);
	}
	v3_debug(v3h, V3_DBG_ALL);
	fprintf(stderr, "got lv3 handle: %i\n", v3h);
	v3_handle find = v3_find_handle(argv[1], argv[2]);
	fprintf(stderr, "found lv3 handle: %i\n", find);
	fprintf(stderr, "login lv3 handle: %i\n", v3h);
	if (v3_login(v3h) != V3_OK) {
	    fprintf(stderr, "v3_login() error: %s\n", v3_error(v3h));
	    exit(EXIT_FAILURE);
	}
	v3_login(v3h);
	v3_destroy(v3h);
	fprintf(stderr, "destroyed handle: %i\n", v3h);
	v3_destroy(v3h);
	v3_destroy(V3_HANDLE_NONE);
	v3_destroy(-3);

    exit(EXIT_SUCCESS);
    return 0;
}

