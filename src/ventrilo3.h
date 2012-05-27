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

#ifndef _VENTRILO3_H
#define _VENTRILO3_H

#include <stdint.h>

#define PACK __attribute__ ((__packed__))

#define V3_OK           0
#define V3_FAILURE      1
#define V3_MALFORM      2
#define V3_NOTIMPL      3

#define V3_BLOCK        1
#define V3_NONBLOCK     0

#define V3_NONE         0
#define V3_CRITICAL     1
#define V3_WARNING      2
#define V3_NOTICE       3
#define V3_INFO         4
#define V3_DEBUG        5

#define V3_DBG_NONE     0
#define V3_DBG_ERROR    1 << 0
#define V3_DBG_STACK    1 << 1
#define V3_DBG_INFO     1 << 2
#define V3_DBG_SOCKET   1 << 3
#define V3_DBG_STATUS   1 << 4
#define V3_DBG_EVENT    1 << 5
#define V3_DBG_MESSAGE  1 << 6
#define V3_DBG_PACKET   1 << 7
#define V3_DBG_MUTEX    1 << 8
#define V3_DBG_MEMORY   1 << 9
#define V3_DBG_ALL      0xffff

#define V3_HANDLE_NONE  (-1)

typedef int16_t             v3_handle;

typedef struct v3_codec     v3_codec;
typedef struct v3_coder     v3_coder;

typedef struct v3_channel   v3_channel;
typedef struct v3_rank      v3_rank;
typedef struct v3_user      v3_user;
typedef struct v3_account   v3_account;

typedef struct v3_event     v3_event;

struct v3_codec {
    int16_t     index;
    int16_t     format;
    uint8_t     frames;
    uint32_t    framesize;
    uint32_t    rate;
    int         quality;
    char        name[128];
};

struct v3_coder {
    int16_t     index;
    int16_t     format;
    uint8_t     channels;
    int         encoder;
    void *      state;
};

struct v3_channel {
    uint16_t    id;                 // 0
    uint16_t    parent;             // 2
    uint8_t     unknown_1;          // 4
    uint8_t     passworded;         // 5
    uint16_t    unknown_2;          // 6
    uint16_t    record;             // 8
    uint16_t    xmit_x_chan;        // 10
    uint16_t    paging;             // 12
    uint16_t    wav_binds;          // 14
    uint16_t    tts_binds;          // 16
    uint16_t    xmit_u2u;           // 18
    uint16_t    xmit_guest_disable; // 20
    uint16_t    events_disable;     // 22
    uint16_t    voice_mode;         // 24
    uint16_t    xmit_time_limit;    // 26
    uint16_t    phantoms;           // 28
    uint16_t    max_clients;        // 30
    uint16_t    guest_join;         // 32
    uint16_t    inactive_ignore;    // 34
    uint16_t    protect_mode;       // 36
    uint16_t    xmit_rank_level;    // 38
    int16_t     codec_index;        // 40
    int16_t     codec_format;       // 42
    uint16_t    voice_targets;      // 44
    uint16_t    command_targets;    // 46

    int         _message_;

    char        name[32];
    char        phonetic[32];
    char        comment[128];

    int         _strings_;

    v3_channel *next;
} PACK;

struct v3_rank {
    uint16_t    id;                 // 0
    uint16_t    level;              // 2

    int         _message_;

    char        name[16];
    char        description[64];

    int         _strings_;

    v3_rank *   next;
} PACK;

struct v3_user {
    uint16_t    id;                 // 0
    uint16_t    channel;            // 2
    uint16_t    flags;              // 4
    uint16_t    rank;               // 6

    int         _message_;

    char        name[32];
    char        phonetic[32];
    char        comment[128];
    char        integration[128];
    char        url[128];

    int         _strings_;

    uint8_t     accept_pages;
    uint8_t     accept_u2u;
    uint8_t     accept_chat;
    uint8_t     allow_record;

    uint8_t     muted_local;
    uint8_t     muted_global;
    uint8_t     muted_channel;
    uint8_t     transmitting;
    uint8_t     guest;
    uint16_t    phantom_owner;

    v3_coder    decoder;
    float       volume;

    v3_user *   next;
} PACK;

enum {
    V3_USER_ACCEPT_PAGES    = 0x00,
    V3_USER_ACCEPT_U2U      = 0x01,
    V3_USER_ALLOW_RECORD    = 0x02,
    V3_USER_ACCEPT_CHAT     = 0x03,
    V3_USER_GLOBAL_MUTE     = 0x04,
    V3_USER_CHANNEL_MUTE    = 0x05
};

struct v3_account {
    uint16_t    id;                 // 0
    uint16_t    owner_replace;      // 2
    uint8_t     password[32];       // 4
    uint16_t    rank;               // 36
    uint16_t    unknown_1;          // 38
    uint8_t     locked;             // 40
    uint8_t     reserved;           // 41
    uint8_t     duplicates;         // 42
    uint8_t     chan_change;        // 43
    uint16_t    chan_default;       // 44
    uint8_t     unknown_2;          // 46
    uint8_t     unknown_3;          // 47
    uint8_t     recv_broadcast;     // 48
    uint8_t     phantom_add;        // 49
    uint8_t     record;             // 50
    uint8_t     recv_complaint;     // 51
    uint8_t     send_complaint;     // 52
    uint8_t     inactive_ignore;    // 53
    uint8_t     unknown_4;          // 54
    uint8_t     unknown_5;          // 55
    uint8_t     srv_admin;          // 56
    uint8_t     user_add;           // 57
    uint8_t     user_del;           // 58
    uint8_t     user_ban;           // 59
    uint8_t     user_kick;          // 60
    uint8_t     user_move;          // 61
    uint8_t     chan_assign;        // 62
    uint8_t     rank_edit;          // 63
    uint8_t     motd_edit;          // 64
    uint8_t     motd_guest_edit;    // 65
    uint8_t     rcon;               // 66
    uint8_t     voice_target_edit;  // 67
    uint8_t     cmd_target_edit;    // 68
    uint8_t     rank_assign;        // 69
    uint8_t     reserve_assign;     // 70
    uint8_t     unknown_6;          // 71
    uint8_t     unknown_7;          // 72
    uint8_t     unknown_8;          // 73
    uint8_t     unknown_9;          // 74
    uint8_t     unknown_10;         // 75
    uint8_t     broadcast;          // 76
    uint8_t     broadcast_lobby;    // 77
    uint8_t     broadcast_u2u;      // 78
    uint8_t     broadcast_x_chan;   // 79
    uint8_t     tts_send;           // 80
    uint8_t     wav_send;           // 81
    uint8_t     page_send;          // 82
    uint8_t     comment_send;       // 83
    uint8_t     phonetic_set;       // 84
    uint8_t     comment_notify;     // 85
    uint8_t     event_notify;       // 86
    uint8_t     mute_global;        // 87
    uint8_t     mute_others;        // 88
    uint8_t     chat_global;        // 89
    uint8_t     chat_priv;          // 90
    uint8_t     unknown_11;         // 91
    uint8_t     equalizer;          // 92
    uint8_t     unknown_12;         // 93
    uint8_t     unknown_13;         // 94
    uint8_t     unknown_14;         // 95
    uint8_t     see_guest;          // 96
    uint8_t     see_nonguest;       // 97
    uint8_t     see_motd;           // 98
    uint8_t     see_srv_comment;    // 99
    uint8_t     see_chan_list;      // 100
    uint8_t     see_chan_comment;   // 101
    uint8_t     see_user_comment;   // 102
    uint8_t     unknown_15;         // 103

    int         _message_;

    char        name[32];
    char        owner[32];
    char        notes[256];
    char        lock_reason[128];

    int         _strings_;

    uint16_t    chan_admin[32];
    uint16_t    chan_admin_count;
    uint16_t    chan_auth[32];
    uint16_t    chan_auth_count;

    v3_account *next;
} PACK;

typedef struct {
    uint16_t    action;
    uint16_t    interval;
    uint16_t    times;
} PACK v3_filter;

typedef struct {
    uint16_t    chat_filter;
    uint16_t    chan_order;
    uint16_t    motd_always;
    v3_filter   chat_spam;
    v3_filter   comment_spam;
    v3_filter   wav_spam;
    v3_filter   tts_spam;
    uint16_t    inactive_timeout;
    uint16_t    inactive_action;
    uint16_t    inactive_channel;
    uint16_t    rem_srv_comment;
    uint16_t    rem_chan_name;
    uint16_t    rem_chan_comment;
    uint16_t    rem_user_name;
    uint16_t    rem_user_comment;
    char        srv_comment[0x100];
    uint16_t    wav_disable;
    uint16_t    tts_disable;
    v3_filter   chan_spam;
    uint16_t    rem_login;
    uint16_t    max_guest;
    uint16_t    auto_kick;
    uint16_t    auto_ban;
} PACK v3_prop;

struct v3_event {
    int         type;

    uint16_t    ping;

    v3_channel  channel;
    v3_rank     rank;
    v3_user     user;
    v3_account  account;

    union {
        v3_prop prop;
        char    message[0x100];
        struct {
            uint32_t rate;
            uint8_t  channels;
            uint32_t length;
            uint8_t  sample[32768];
        } pcm;
    } data;

    v3_event *  next;
};

enum {
    V3_EVENT_ERROR = 1,
    V3_EVENT_LOGIN,
    V3_EVENT_LOGOUT,
    V3_EVENT_PING,
    V3_EVENT_PROP_CHAT_FILTER,
    V3_EVENT_PROP_CHAN_ORDER,
    V3_EVENT_PROP_MOTD_ALWAYS,
    V3_EVENT_MOTD,
    V3_EVENT_CHAN_LIST,
    V3_EVENT_CHAN_ADD,
    V3_EVENT_CHAN_REMOVE,
    V3_EVENT_CHAN_UPDATE,
    V3_EVENT_CHAN_ADMIN,
    V3_EVENT_CHAN_AUTH,
    V3_EVENT_CHAN_CHANGE,
    V3_EVENT_CHAN_MOVE,
    V3_EVENT_RANK_OPEN,
    V3_EVENT_RANK_CLOSE,
    V3_EVENT_RANK_LIST,
    V3_EVENT_RANK_ADD,
    V3_EVENT_RANK_REMOVE,
    V3_EVENT_RANK_UPDATE,
    V3_EVENT_USER_LIST,
    V3_EVENT_USER_LOGIN,
    V3_EVENT_USER_LOGOUT,
    V3_EVENT_USER_UPDATE,
    V3_EVENT_USER_MUTE,
    V3_EVENT_USER_PAGE,
    V3_EVENT_USER_RANK,
    V3_EVENT_ACCT_OPEN,
    V3_EVENT_ACCT_CLOSE,
    V3_EVENT_ACCT_LIST,
    V3_EVENT_ACCT_ADD,
    V3_EVENT_ACCT_REMOVE,
    V3_EVENT_ACCT_UPDATE,
    V3_EVENT_ACCT_LOCAL,
    V3_EVENT_ACCT_OWNER,
    V3_EVENT_AUDIO_START,
    V3_EVENT_AUDIO_STOP,
    V3_EVENT_AUDIO_RECV,
    V3_EVENT_AUDIO_MUTE,
    V3_EVENT_AUDIO_VRF,
    V3_EVENT_CHAT_JOIN,
    V3_EVENT_CHAT_LEAVE,
    V3_EVENT_CHAT_MESSAGE,
    V3_EVENT_CHAT_RCON,
    V3_EVENT_CHAT_PRIV_START,
    V3_EVENT_CHAT_PRIV_END,
    V3_EVENT_CHAT_PRIV_MESSAGE,
    V3_EVENT_CHAT_PRIV_AWAY,
    V3_EVENT_CHAT_PRIV_BACK,
    V3_EVENT_TTS_MESSAGE,
    V3_EVENT_WAV_MESSAGE,
    V3_EVENT_ADMIN_AUTH,

    V3_EVENT_INTERNAL
};

/*
 * Debug functions
 */
int             v3_debug(v3_handle v3h, int level);
const char *    v3_error(v3_handle v3h);

/*
 * Functions to initialize a connection and perform mainloop processing
 */
v3_handle       v3_init(const char *server, const char *username);
v3_handle       v3_find(const char *server, const char *username);
void            v3_destroy(v3_handle v3h);

int             v3_password(v3_handle v3h, const char *password);
int             v3_phonetic(v3_handle v3h, const char *phonetic);
int             v3_default_channel_path(v3_handle v3h, const char *path);
int             v3_default_channel_id(v3_handle v3h, uint16_t id);

int             v3_login(v3_handle v3h);
int             v3_login_cancel(v3_handle v3h);
int             v3_logout(v3_handle v3h);
int             v3_iterate(v3_handle v3h, int block, uint32_t max);

/*
 * Event processing functions
 */
int             v3_event_count(v3_handle v3h);
int             v3_event_get(v3_handle v3h, int block, v3_event *ev);
void            v3_event_clear(v3_handle v3h);

/*
 * Server data retrieval functions
 */
int             v3_logged_in(v3_handle v3h);
uint16_t        v3_luser_id(v3_handle v3h);
uint16_t        v3_luser_channel(v3_handle v3h);
uint16_t        v3_licensed(v3_handle v3h);
uint16_t        v3_slot_count(v3_handle v3h);
uint64_t        v3_sent_bytes(v3_handle v3h);
uint64_t        v3_recv_bytes(v3_handle v3h);
uint32_t        v3_sent_packets(v3_handle v3h);
uint32_t        v3_recv_packets(v3_handle v3h);

int             v3_channel_admin(v3_handle v3h, uint16_t id);
uint16_t        v3_channel_password(v3_handle v3h, uint16_t id);
char *          v3_channel_path(v3_handle v3h, uint16_t id);
uint16_t        v3_channel_id(v3_handle v3h, const char *path);
int             v3_channel_sort(v3_handle v3h, uint16_t left, uint16_t right);

int             v3_channel_count(v3_handle v3h);
int             v3_channel_get(v3_handle v3h, v3_channel *c);

int             v3_rank_count(v3_handle v3h);
int             v3_rank_get(v3_handle v3h, v3_rank *r);

int             v3_user_count(v3_handle v3h);
int             v3_user_get(v3_handle v3h, v3_user *u);

int             v3_account_count(v3_handle v3h);
int             v3_account_get(v3_handle v3h, v3_account *a);

int             v3_lacct(v3_handle v3h, v3_account *a);

const v3_codec *v3_codec_get(int16_t index, int16_t format);
const v3_codec *v3_codec_channel_get(v3_handle v3h, uint16_t id);
const v3_codec *v3_codec_user_get(v3_handle v3h, uint16_t id);
uint32_t        v3_codec_rate_get(int16_t index, int16_t format);
int             v3_codec_valid(const v3_codec *codec);

/*
 * Audio signal processing functions
 */
void            v3_volume_master_set(float level);
float           v3_volume_master_get(void);
int             v3_volume_server_set(v3_handle v3h, float level);
float           v3_volume_server_get(v3_handle v3h);
int             v3_volume_user_set(v3_handle v3h, uint16_t id, float level);
float           v3_volume_user_get(v3_handle v3h, uint16_t id);

/*
 * Outbound user functions
 */
int             v3_luser_option(v3_handle v3h, int type, uint8_t value);
int             v3_luser_text(v3_handle v3h, const char *comment, const char *url, const char *integration, uint8_t silent);

int             v3_channel_change(v3_handle v3h, uint16_t id, const char *password);

int             v3_user_mute(v3_handle v3h, uint16_t id, uint8_t mute);
int             v3_user_page(v3_handle v3h, uint16_t id);

int             v3_chat_join(v3_handle v3h);
int             v3_chat_leave(v3_handle v3h);
int             v3_chat_message(v3_handle v3h, const char *message);

void            v3_chat_priv_start(v3_handle v3h, uint16_t id);
void            v3_chat_priv_end(v3_handle v3h, uint16_t id);
void            v3_chat_priv_message(v3_handle v3h, uint16_t id, const char *message);
void            v3_chat_priv_away(v3_handle v3h, uint16_t id);
void            v3_chat_priv_back(v3_handle v3h, uint16_t id);

int             v3_audio_start(v3_handle v3h);
int             v3_audio_stop(v3_handle v3h);
int             v3_audio_send(v3_handle v3h, uint32_t rate, uint8_t channels, const void *pcm, uint32_t pcmlen);

int             v3_phantom_add(v3_handle v3h, uint16_t id);
int             v3_phantom_remove(v3_handle v3h, uint16_t id);

/*
 * Administrative user functions
 */
int             v3_admin_login(v3_handle v3h, const char *password);
int             v3_admin_logout(v3_handle v3h);
int             v3_admin_channel_kick(v3_handle v3h, uint16_t id);
int             v3_admin_channel_ban(v3_handle v3h, uint16_t id, const char *reason);
int             v3_admin_kick(v3_handle v3h, uint16_t id, const char *reason);
int             v3_admin_ban(v3_handle v3h, uint16_t id, const char *reason);
void            v3_admin_ban_list(v3_handle v3h);
void            v3_admin_ban_add(v3_handle v3h, uint16_t bitmask, uint32_t ip, const char *user, const char *reason);
void            v3_admin_ban_remove(v3_handle v3h, uint16_t bitmask, uint32_t ip);
void            v3_admin_motd_user(v3_handle v3h, const char *motd);
void            v3_admin_motd_guest(v3_handle v3h, const char *motd);
int             v3_admin_move(v3_handle v3h, uint16_t src, uint16_t dest);
int             v3_admin_mute_channel(v3_handle v3h, uint16_t id);
int             v3_admin_mute_global(v3_handle v3h, uint16_t id);

void            v3_channel_add(v3_handle v3h, uint16_t id, const v3_channel *c);
void            v3_channel_update(v3_handle v3h, uint16_t id, const v3_channel *c);
void            v3_channel_remove(v3_handle v3h, uint16_t id);

void            v3_account_open(v3_handle v3h);
void            v3_account_close(v3_handle v3h);
void            v3_account_add(v3_handle v3h, const v3_account *a);
void            v3_account_update(v3_handle v3h, const v3_account *a);
void            v3_account_remove(v3_handle v3h, uint16_t id);
void            v3_account_owner(v3_handle v3h, uint16_t id, uint16_t owner);

#endif // _VENTRILO3_H

