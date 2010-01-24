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
 * Mangler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mangler.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "libventrilo3.h"

/*
 * Initialize a libventrilo3 handle for a server connection.
 * @return A pointer to a v3_handle for performing operations on a server
 */
v3_handle *v3_init(void) {
    return NULL;
}


/*
 * Free all resources allocated for the specified libventrilo3 handle and
 * disconnect if currently connected
 * @param v3h A v3_handle created by v3_init();
 */
void v3_destroy(v3_handle *v3h) {
    free(v3h);
}

/*
 *
 */

