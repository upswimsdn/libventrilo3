

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

