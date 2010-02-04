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

#ifndef _LIBVENTRILO3_H
#define _LIBVENTRILO3_H

#include "config.h"
#include <stdio.h>
#include <stdint.h>

#include "ventrilo3.h"

#define __USE_UNIX98
#include <pthread.h>
#undef __USE_UNIX98

#define V3_CONN_MAX 64

typedef struct _v3_connection {
    pthread_mutex_t *   mutex;

    uint32_t            ip;
    uint16_t            port;
    uint16_t            max_clients;
    uint16_t            connected_clients;

    char                name[32];
    char                version[16];
    char                os[32];

    v3_user *           luser;
    v3_user *           lperms;
    v3_user *           users;
    v3_channel *        channels;
    v3_rank *           ranks;
    v3_account *        accounts;

    char                motd[8193];
    char                guest_motd[8193];

    uint8_t             auth_server_index;
    ventrilo_key_ctx *  server_key;
    ventrilo_key_ctx *  client_key;
    char *              handshake_key;
    char *              handshake;

    int                 ev_recvq_pipe[2];
    FILE *              ev_recvq_instream;
    FILE *              ev_recvq_outstream;
    v3_event *          ev_recvq;
    pthread_mutex_t *   ev_recvq_mutex;
    pthread_cond_t *    ev_recvq_cond;

    int                 ev_sendq_pipe[2];
    FILE *              ev_sendq_instream;
    FILE *              ev_sendq_outstream;
    v3_event *          ev_sendq;
    pthread_mutex_t *   ev_sendq_mutex;
    pthread_cond_t *    ev_sendq_cond;

    int                 loggedin;
    int                 sockd;

    int                 volume;

    void *              gsm_encoder;
    void *              speex_encoder;

    uint32_t            recv_pkt_count;
    uint32_t            send_pkt_count;
    uint32_t            recv_byte_count;
    uint32_t            send_byte_count;

    int16_t             codec_id;
    int16_t             codec_format;
} _v3_connection;

typedef struct _v3_net_message {
    uint16_t len;
    uint16_t type;
    char *data;
    void *contents;
    int (* destroy)(struct __v3_net_message *msg);
    struct __v3_net_message *next;
} _v3_net_message;


_v3_connection _v3_handles[V3_CONN_MAX];

#endif // _LIBVENTRILO3_H

