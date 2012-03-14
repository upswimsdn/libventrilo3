/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
 *
 * Copyright 2009-2011 Eric Connell
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

#define V3_CLIENT_PLATFORM  "WIN32"
#define V3_CLIENT_VERSION   "3.0.5"
#define V3_PROTO_VERSION    "3.0.0"

#define V3_MAX_CONN 64

typedef struct _v3_connection   _v3_connection;
typedef struct v3_key_ctx       v3_key_ctx;

pthread_mutex_t _v3_handles_mutex        = PTHREAD_MUTEX_INITIALIZER;
_v3_connection *_v3_handles[V3_MAX_CONN] = { NULL };

int16_t  _stack      = 0;
uint16_t _debug      = V3_DBG_NONE;
char     _error[256] = "";
float    _volume     = 1.0;

struct _v3_connection {
    int16_t             stack;
    uint16_t            debug;
    char                error[256];

    uint32_t            ip;
    uint16_t            port;

    pthread_mutex_t *   mutex;

    uint8_t             connecting;

    int                 sd;

    uint16_t            handshake_idx;
    uint8_t             handshake_key[64];
    uint8_t             handshake[16];

    char                password[32];
    char                def_path[128];
    uint16_t            def_id;

    v3_user             luser;
    v3_account          lacct;

    v3_key_ctx *        client_key;
    v3_key_ctx *        server_key;

    uint8_t             logged_in;

    struct timeval      timestamp;

    struct {
        uint8_t         major;
        uint8_t         minor;
    } protocol;

    uint16_t            licensed;
    uint16_t            slots;

    char                name[32];
    char                platform[32];
    char                version[32];

    v3_channel *        channels;
    v3_rank *           ranks;
    v3_user *           users;
    v3_account *        accounts;

    v3_event *          eventq;
    pthread_mutex_t *   event_mutex;
    pthread_cond_t *    event_cond;

    float               volume;
    v3_coder            encoder;
#if HAVE_SPEEXDSP
    struct {
        void *          state;
        uint32_t        in_rate;
        uint32_t        out_rate;
        uint8_t         channels;
    } resampler;
#endif
    uint8_t             pcmq[1 << 16];
    uint32_t            pcmqueued;

    v3_prop             prop;
    uint16_t            transaction;

    uint64_t            sent_byte_ctr;
    uint64_t            recv_byte_ctr;
    uint32_t            sent_pkt_ctr;
    uint32_t            recv_pkt_ctr;

    uint16_t            codec_index;
    uint16_t            codec_format;
};

typedef struct {
    uint16_t    len;
    uint32_t    type;
    void *      data;
} _v3_message;

/* data.c */
enum {
    V3_DATA_UPDATE = 1,
    V3_DATA_COPY,
    V3_DATA_RETURN,
    V3_DATA_REMOVE,
    V3_DATA_CLEAR
};

enum {
    V3_DATA_TYPE_CHANNEL = 1,
    V3_DATA_TYPE_RANK,
    V3_DATA_TYPE_USER,
    V3_DATA_TYPE_ACCOUNT
};

/* dsp.c */
const v3_codec v3_codecs[] = {
#if HAVE_GSM
    { 0, 0,  3,  640,  8000, -1, "GSM 6.10 8kHz" },
    { 0, 1,  4,  640, 11025, -1, "GSM 6.10 11kHz" },
    { 0, 2,  7,  640, 22050, -1, "GSM 6.10 22kHz" },
    { 0, 3,  15, 640, 44100, -1, "GSM 6.10 44kHz" },
#endif
#if HAVE_CELT
    { 1, 0,  15, 640, 44100, -1, "CELT 0.7 44kHz" },
    { 2, 0,  7,  640, 22050, -1, "CELT 0.7 22kHz" },
#endif
#if HAVE_SPEEX
    { 3, 0,  6,  320,  8000,  0, "Speex 8kHz Quality 0" },
    { 3, 1,  6,  320,  8000,  1, "Speex 8kHz Quality 1" },
    { 3, 2,  6,  320,  8000,  2, "Speex 8kHz Quality 2" },
    { 3, 3,  6,  320,  8000,  3, "Speex 8kHz Quality 3" },
    { 3, 4,  6,  320,  8000,  4, "Speex 8kHz Quality 4" },
    { 3, 5,  6,  320,  8000,  5, "Speex 8kHz Quality 5" },
    { 3, 6,  6,  320,  8000,  6, "Speex 8kHz Quality 6" },
    { 3, 7,  6,  320,  8000,  7, "Speex 8kHz Quality 7" },
    { 3, 8,  6,  320,  8000,  8, "Speex 8kHz Quality 8" },
    { 3, 9,  6,  320,  8000,  9, "Speex 8kHz Quality 9" },
    { 3, 10, 6,  320,  8000, 10, "Speex 8kHz Quality 10" },
    { 3, 11, 6,  640, 16000,  0, "Speex 16kHz Quality 0" },
    { 3, 12, 6,  640, 16000,  1, "Speex 16kHz Quality 1" },
    { 3, 13, 6,  640, 16000,  2, "Speex 16kHz Quality 2" },
    { 3, 14, 6,  640, 16000,  3, "Speex 16kHz Quality 3" },
    { 3, 15, 6,  640, 16000,  4, "Speex 16kHz Quality 4" },
    { 3, 16, 6,  640, 16000,  5, "Speex 16kHz Quality 5" },
    { 3, 17, 6,  640, 16000,  6, "Speex 16kHz Quality 6" },
    { 3, 18, 6,  640, 16000,  7, "Speex 16kHz Quality 7" },
    { 3, 19, 6,  640, 16000,  8, "Speex 16kHz Quality 8" },
    { 3, 20, 6,  640, 16000,  9, "Speex 16kHz Quality 9" },
    { 3, 21, 6,  640, 16000, 10, "Speex 16kHz Quality 10" },
    { 3, 22, 6, 1280, 32000,  0, "Speex 32kHz Quality 0" },
    { 3, 23, 6, 1280, 32000,  1, "Speex 32kHz Quality 1" },
    { 3, 24, 6, 1280, 32000,  2, "Speex 32kHz Quality 2" },
    { 3, 25, 6, 1280, 32000,  3, "Speex 32kHz Quality 3" },
    { 3, 26, 6, 1280, 32000,  4, "Speex 32kHz Quality 4" },
    { 3, 27, 6, 1280, 32000,  5, "Speex 32kHz Quality 5" },
    { 3, 28, 6, 1280, 32000,  6, "Speex 32kHz Quality 6" },
    { 3, 29, 6, 1280, 32000,  7, "Speex 32kHz Quality 7" },
    { 3, 30, 6, 1280, 32000,  8, "Speex 32kHz Quality 8" },
    { 3, 31, 6, 1280, 32000,  9, "Speex 32kHz Quality 9" },
    { 3, 32, 6, 1280, 32000, 10, "Speex 32kHz Quality 10" },
#endif
    { -1, -1, 0, 0, 0, -1, "Unsupported Codec" }
};

/* encryption.c / handshake.c */
struct v3_key_ctx {
    uint8_t     key[256];
    uint32_t    pos;
    uint32_t    size;
};

/* encryption.c */
const uint32_t _v3_hash_table[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
    0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
    0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
    0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
    0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
    0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
    0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
    0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
    0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
    0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
    0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
    0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
    0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
    0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
    0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
    0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/* handshake.c */
typedef struct {
    uint32_t    vnum;
    char *      host;
    uint16_t    port;
} v3_auth_t;

const v3_auth_t v3_auth[] = {
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

char *      _v3_strncpy(char *dest, const char *src, size_t n);

/* data.c */
int         _v3_data(v3_handle v3h, int oper, int type, void *data, size_t n);

/* dsp.c */
void        _v3_coder_destroy(v3_handle v3h, v3_coder *coder);
void        _v3_audio_amplify(v3_handle v3h, int16_t *pcm, uint32_t pcmlen, float **volume, size_t count);
int         _v3_audio_send(v3_handle v3h, uint32_t rate, uint8_t channels, const void *pcm, uint32_t pcmlen);
int         _v3_audio_encode(
                    v3_handle v3h,
                    /* pcm input */
                    const uint8_t *pcm,
                    uint32_t pcmlen,
                    /* encoded output */
                    int16_t index,
                    int16_t format,
                    v3_coder *coder,
                    uint8_t *data,
                    uint32_t *datalen,
                    /* optional args */
                    uint8_t channels,
                    uint16_t *framecount,
                    uint8_t *celtfragsize);
int         _v3_audio_decode(
                    v3_handle v3h,
                    /* encoded input */
                    int16_t index,
                    int16_t format,
                    v3_coder *coder,
                    const uint8_t *data,
                    int32_t datalen,
                    /* pcm output */
                    uint8_t *pcm,
                    uint32_t *pcmlen,
                    /* optional args */
                    uint32_t *rate,
                    uint8_t channels);

/* encryption.c */
void        _v3_password(v3_handle v3h, const char *password, uint8_t *hash);
int         _v3_read_keys(v3_handle v3h, v3_key_ctx *client, v3_key_ctx *server, uint8_t *data, uint32_t len);
void        _v3_dec_init(v3_handle v3h, uint8_t *data, uint32_t len);
void        _v3_enc_init(v3_handle v3h, uint8_t *data, uint32_t len);
void        _v3_decrypt(v3_handle v3h, v3_key_ctx *ctx, uint8_t *data, uint32_t len);
void        _v3_encrypt(v3_handle v3h, v3_key_ctx *ctx, uint8_t *data, uint32_t len);

/* handshake.c */
int         _v3_handshake(v3_handle v3h);
void        _v3_key_scramble(v3_handle v3h, v3_key_ctx *ctx, uint8_t *v3key);
uint16_t    _v3_udp_header(v3_handle v3h, uint32_t type, uint8_t *buf, uint8_t *pck);
int         _v3_udp_send(v3_handle v3h, int sd, uint32_t vnum, uint32_t ip, uint16_t port, uint8_t *data, uint32_t len);
int         _v3_udp_recv(v3_handle v3h, int sd, uint8_t *data, uint32_t maxsz, const v3_auth_t *vauth, uint16_t *handshake_idx);

/* message.c */
int         _v3_msg_move_put(v3_handle v3h, uint16_t id, uint16_t channel);
int         _v3_msg_chat_put(v3_handle v3h, uint16_t subtype, const char *message);
int         _v3_msg_user_option_put(v3_handle v3h, uint16_t user_id, uint16_t subtype, uint32_t value);
int         _v3_msg_chan_list_put(v3_handle v3h, uint16_t subtype, uint16_t user, const char *password, const v3_channel *c);
int         _v3_msg_audio_put(v3_handle v3h, uint16_t subtype, int16_t index, int16_t format, uint32_t pcmlen, const void *data, uint32_t datalen);
int         _v3_msg_phantom_put(v3_handle v3h, uint16_t subtype, uint16_t phantom, uint16_t channel);
int         _v3_msg_user_list_put(v3_handle v3h, uint16_t subtype, const v3_user *u);
int         _v3_msg_user_page_put(v3_handle v3h, uint16_t to, uint16_t from);
int         _v3_msg_admin_put(v3_handle v3h, uint16_t subtype, uint16_t user, const void *data);

/* network.c */
int         _v3_connected(v3_handle v3h);
int         _v3_canceling(v3_handle v3h);
void        _v3_close(v3_handle v3h);
int         _v3_connect(v3_handle v3h, int tcp);
void        _v3_timestamp(v3_handle v3h, struct timeval *tv);
int         _v3_send(v3_handle v3h, _v3_message *m);
void *      _v3_recv(v3_handle v3h, int block);

#endif // _LIBVENTRILO3_H

