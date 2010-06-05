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

#include "ventrilo3.h"

#define V3_MAX_CONN 64

typedef struct _v3_connection   _v3_connection;
typedef struct _v3_message      _v3_message;

pthread_mutex_t _v3_handles_mutex        = PTHREAD_MUTEX_INITIALIZER;
_v3_connection *_v3_handles[V3_MAX_CONN] = { NULL };

uint16_t _debug      = V3_DBG_NONE;
char     _error[256] = "";

struct _v3_connection {
    uint16_t            stack;
    uint16_t            debug;
    char                error[256];

    pthread_mutex_t *   mutex;

    uint32_t            ip;
    uint16_t            port;
    uint16_t            max_clients;
    uint16_t            connected_clients;

    struct {
        uint8_t         major;
        uint8_t         minor;
    } protocol;

    char                name[32];
    char                version[16];
    char                platform[32];

    v3_user             luser;
    v3_user             lperms;
    v3_user *           users;
    v3_channel *        channels;
    v3_rank *           ranks;
    v3_account *        accounts;

    char                motd[32768];
    char                guest_motd[32768];

    uint8_t             auth_server_index;
    ventrilo_key_ctx *  server_key;
    ventrilo_key_ctx *  client_key;
    char *              handshake_key;
    char *              handshake;

    int                 ev_recvq_pipe[2];
    v3_event *          ev_recvq;
    pthread_mutex_t *   ev_recvq_mutex;
    pthread_cond_t *    ev_recvq_cond;

    int                 ev_sendq_pipe[2];
    v3_event *          ev_sendq;
    pthread_mutex_t *   ev_sendq_mutex;
    pthread_cond_t *    ev_sendq_cond;

    int                 loggedin;
    int                 sockd;

    int                 volume;

    void *              gsm_encoder;
    void *              speex_encoder;

    uint32_t            recv_pkt_count;
    uint64_t            recv_byte_count;
    uint32_t            send_pkt_count;
    uint64_t            send_byte_count;

    int16_t             codec_index;
    int16_t             codec_format;
};

struct _v3_message {
    uint16_t    len;
    uint16_t    type;
    void        *data;
    void        *contents;
    _v3_message *next;
};

int         _v3_check_handle(v3_handle v3h);

void        _v3_debug(v3_handle v3h, int level, const char *format, ...);
void        _v3_error(v3_handle v3h, const char *format, ...);

int         _v3_parse_server_info(const char *server, uint32_t *ip, uint16_t *port);
uint32_t    _v3_resolv(const char *hostname);

#endif // _LIBVENTRILO3_H

