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

#define __USE_UNIX98
#include <pthread.h>
#undef __USE_UNIX98

#define V3_MAX_CONN 64

typedef struct _v3_connection   _v3_connection;
typedef struct _v3_message      _v3_message;
typedef struct ventrilo_key_ctx ventrilo_key_ctx;

pthread_mutex_t _v3_handles_mutex        = PTHREAD_MUTEX_INITIALIZER;
_v3_connection *_v3_handles[V3_MAX_CONN] = { NULL };

int16_t  _stack      = 0;
uint16_t _debug      = V3_DBG_NONE;
char     _error[256] = "";

struct _v3_connection {
    int16_t             stack;
    uint16_t            debug;
    char                error[256];

    uint32_t            ip;
    uint16_t            port;

    pthread_mutex_t *   mutex;

    uint8_t             connecting;
    uint8_t             cancel;

    int                 sd;

    uint16_t            handshake_idx;
    uint8_t             handshake_key[64];
    uint8_t             handshake[16];

    char                password[32];
    v3_user             luser;
    v3_user             lperms;

    ventrilo_key_ctx *  server_key;
    ventrilo_key_ctx *  client_key;

    uint8_t             loggedin;

    struct {
        uint8_t         major;
        uint8_t         minor;
    } protocol;

    char                name[32];
    char                platform[32];
    char                version[32];

    uint16_t            max_clients;
    uint16_t            connected_clients;

    v3_user *           users;
    v3_channel *        channels;
    v3_rank *           ranks;
    v3_account *        accounts;

    char                motd[32768];
    char                guest_motd[32768];

    int                 ev_recvq_pipe[2];
    v3_event *          ev_recvq;
    pthread_mutex_t *   ev_recvq_mutex;
    pthread_cond_t *    ev_recvq_cond;

    int                 ev_sendq_pipe[2];
    v3_event *          ev_sendq;
    pthread_mutex_t *   ev_sendq_mutex;
    pthread_cond_t *    ev_sendq_cond;

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

/* encryption.c / handshake.c */
struct ventrilo_key_ctx {
    uint8_t     key[256];
    uint32_t    pos;
    uint32_t    size;
};

/* handshake.c */
typedef struct {
    uint32_t    vnum;
    char *      host;
    uint16_t    port;
} ventrilo3_auth_t;

static const ventrilo3_auth_t ventrilo3_auth[] = {
    { 1,  "72.51.46.31",   6100 },
    { 2,  "64.34.178.178", 6100 },
    { 3,  "74.54.61.194",  6100 },
    { 4,  "70.85.110.242", 6100 },
    { -1, NULL,            0    }
};

int         _v3_handle_valid(v3_handle v3h);

void        _v3_debug(v3_handle v3h, int level, const char *format, ...);
void        _v3_error(v3_handle v3h, const char *format, ...);
void        _v3_enter(v3_handle v3h, const char *func);
void        _v3_leave(v3_handle v3h, const char *func);
void        _v3_packet(v3_handle v3h, const uint8_t *data, int len);

int         _v3_parse_server(const char *server, uint32_t *ip, uint16_t *port);
uint32_t    _v3_resolv(const char *hostname);

void        _v3_mutex_init(v3_handle v3h);
void        _v3_mutex_destroy(v3_handle v3h);
int         _v3_mutex_lock(v3_handle v3h);
int         _v3_mutex_unlock(v3_handle v3h);

/* encryption.c */
int         ventrilo_read_keys(ventrilo_key_ctx *client, ventrilo_key_ctx *server, uint8_t *data, int size);
void        ventrilo_dec_init(uint8_t *data, int size);
void        ventrilo_enc_init(uint8_t *data, int size);
void        ventrilo_dec(ventrilo_key_ctx *ctx, uint8_t *data, int size);
void        ventrilo_enc(ventrilo_key_ctx *ctx, uint8_t *data, int size);

/* handshake.c */
void        _v3_key_scramble(ventrilo_key_ctx *ctx, uint8_t *v3key);
int         _v3_handshake(v3_handle v3h);
uint16_t    _v3_udp_header(uint32_t type, uint8_t *buf, uint8_t *pck);
int         _v3_udp_send(v3_handle v3h, int sd, uint32_t vnum, uint32_t ip, uint16_t port, uint8_t *data, uint32_t len);
int         _v3_udp_recv(v3_handle v3h, int sd, uint8_t *data, uint32_t maxsz, const ventrilo3_auth_t *vauth, uint16_t *handshake_idx);
uint32_t    getbe(uint8_t *data, uint32_t *ret, uint32_t bits);
uint32_t    putbe(uint8_t *data, uint32_t num, uint32_t bits);

/* network.c */
int         _v3_connected(v3_handle v3h);
int         _v3_canceling(v3_handle v3h);
void        _v3_close(v3_handle v3h);
int         _v3_connect(v3_handle v3h, int udp);

#endif // _LIBVENTRILO3_H

