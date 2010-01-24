/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2010-01-16 20:03:53 -0800 (Sat, 16 Jan 2010) $
 * $Revision: 533 $
 * $LastChangedBy: Haxar $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/libventrilo3.h $
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

#ifndef _LIBVENTRILO3_H
#define _LIBVENTRILO3_H

#include "config.h"
#include <stdint.h>

#define __USE_UNIX98
#include <pthread.h>
#undef __USE_UNIX98

typedef struct v3_handle {
    v3_user *luser;
    v3_user *lperms;
    v3_user *users;
    v3_channel *channels;
    v3_rank *ranks;
    v3_account *accounts;

    pthread_mutext_t *luser_mutex;
    pthread_mutext_t *lperms_mutex;
    pthread_mutext_t *users_mutex;
    pthread_mutext_t *channels_mutex;
    pthread_mutext_t *ranks_mutex;
    pthread_mutext_t *accounts_mutex;

    ventrilo_key_ctx *encryptionkey;

    v3_event *evqueue;
    pthread_mutext_t *evqueue_mutex;
    pthread_cond_t *evqueue_cond;

    int debuglevel;
    int loggedin;
    int sockd;
    int evpipe[2];

    int master_volume;

    void *speex_encoder;
    gsm_handle gsm_encoder;
} v3_handle;

#endif // _LIBVENTRILO3_H

