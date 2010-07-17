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

#ifndef _MESSAGE_H
#define _MESSAGE_H

enum {
    V3_MSG_HANDSHAKE    = 0x00,
    V3_MSG_AUTH         = 0x06,
    V3_MSG_CHAN_ADMIN   = 0x33,
    V3_MSG_SCRAMBLE     = 0x34,
    V3_MSG_RANK_LIST    = 0x36,
    V3_MSG_PING         = 0x37,
    V3_MSG_TTS          = 0x3a,
    V3_MSG_MOVE         = 0x3b,
    V3_MSG_SRV_CODEC    = 0x3c,
    V3_MSG_WAV          = 0x3f,
    V3_MSG_CHAT         = 0x42,
    V3_MSG_USER_OPTION  = 0x46,
    V3_MSG_LOGIN        = 0x48,
    V3_MSG_CHAN_LIST    = 0x49,
    V3_MSG_ACCT_LIST    = 0x4a,
    V3_MSG_TIMESTAMP    = 0x4b,
    V3_MSG_SRV_PROP     = 0x4c,
    V3_MSG_MOTD         = 0x50,
    V3_MSG_AUDIO        = 0x52,
    V3_MSG_CHAN_CHANGE  = 0x53,
//    V3_MSG_COMM_TARGET  = 0x54,
//    V3_MSG_COMPLAINT    = 0x56,
    V3_MSG_SRV_INFO     = 0x57,
    V3_MSG_PHANTOM      = 0x58,
    V3_MSG_ERROR        = 0x59,
    V3_MSG_CHAT_PRIV    = 0x5a,
    V3_MSG_HASH_TABLE   = 0x5c,
    V3_MSG_USER_LIST    = 0x5d,
    V3_MSG_LIST_CHAN    = 0x60,
    V3_MSG_BAN_LIST     = 0x61,
    V3_MSG_USER_PAGE    = 0x62,
    V3_MSG_ADMIN        = 0x63
};

enum {
    V2_MSG_HANDSHAKE    = 0x00,
    V2_MSG_AUTH         = 0x06,
    V2_MSG_MOTD         = 0x33,
    V2_MSG_SRV_INFO     = 0x34,
    V2_MSG_LOGIN        = 0x36,
    V2_MSG_ERROR        = 0x3a,
    V2_MSG_USER_PAGE    = 0x3c,
    V2_MSG_CHAT         = 0x43,
    V2_MSG_USER_LIST    = 0x44,
    V2_MSG_CHAN_LIST    = 0x46,
    V2_MSG_WAV          = 0x4e,
    V2_MSG_SRV_PROP     = 0x52,
    V2_MSG_TTS          = 0x53,
    V2_MSG_ADMIN        = 0x54,
    V2_MSG_SCRAMBLE     = 0x56,
    V2_MSG_CHAN_CHANGE  = 0x58,
    V2_MSG_HASH_TABLE   = 0x5a,
    V2_MSG_ACCT_LIST    = 0x5e,
    V2_MSG_USER_OPTION  = 0x60,
    V2_MSG_PING         = 0x63
};

enum {
    V3_AUTH_DISCONNECT  = 1 << 0,
    V3_AUTH_ADMIN       = 1 << 1,
    V3_AUTH_KEYS        = 1 << 2,
    V3_AUTH_INVALID     = 1 << 6,
    V3_AUTH_DISABLED    = 1 << 9
};

enum {
    V3_RANK_LIST        = 0x00,
    V3_RANK_OPEN        = 0x01,
    V3_RANK_CLOSE       = 0x02,
    V3_RANK_ADD         = 0x03,
    V3_RANK_REMOVE      = 0x04,
    V3_RANK_UPDATE      = 0x05
};

enum {
    V3_CHAT_JOIN        = 0x00,
    V3_CHAT_LEAVE       = 0x01,
    V3_CHAT_MESSAGE     = 0x02,
    V3_CHAT_RCON        = 0x03,
    V3_CHAT_PERM        = 0x04
};

enum {
    V3_CHAN_ADD         = 0x01,
    V3_CHAN_REMOVE      = 0x02,
    V3_CHAN_CHANGE      = 0x03,
    V3_CHAN_UPDATE      = 0x05,
    V3_CHAN_AUTH        = 0x07,
    V3_CHAN_KICK        = 0x09
};

enum {
    V3_ACCT_OPEN        = 0x00,
    V3_ACCT_ADD         = 0x01,
    V3_ACCT_REMOVE      = 0x02,
    V3_ACCT_UPDATE      = 0x03,
    V3_ACCT_CLOSE       = 0x04,
    V3_ACCT_LOCAL       = 0x05,
    V3_ACCT_OWNER       = 0x06
};

enum {
    V3_PROP_RECV        = 0x00,
    V3_PROP_SEND        = 0x01,
    V3_PROP_CLIENT      = 0x02,
    V3_PROP_COMMIT      = 0x03,
    V3_PROP_DONE        = 0x04
};

enum {
    V3_AUDIO_START      = 0x00,
    V3_AUDIO_DATA       = 0x01,
    V3_AUDIO_STOP       = 0x02,
    V3_AUDIO_MUTE       = 0x03,
    V3_AUDIO_LOGIN      = 0x04,
    V3_AUDIO_TAKEN      = 0x05,
    V3_AUDIO_AVAIL      = 0x06
};

enum {
    V3_PHANTOM_ADD      = 0x00,
    V3_PHANTOM_REMOVE   = 0x01
};

enum {
    V3_METHOD_CURRENT   = 0x02,
    V3_METHOD_CHANNEL   = 0x03,
    V3_METHOD_NEST      = 0x04,
    V3_METHOD_USER      = 0x05,
    V3_METHOD_VOICE     = 0x06
};

enum {
    V3_USER_REMOVE      = 0x00,
    V3_USER_ADD         = 0x01,
    V3_USER_UPDATE      = 0x02,
    V3_USER_LIST        = 0x04,
    V3_USER_RANK        = 0x06
};

enum {
    V3_ADMIN_LOGIN      = 0x00,
    V3_ADMIN_KICK       = 0x01,
    V3_ADMIN_BAN        = 0x03,
    V3_ADMIN_LOGOUT     = 0x04,
    V3_ADMIN_CHAN_BAN   = 0x05
};

typedef struct {
    char     protocol[16];  // 4
    uint8_t  salt_1[32];    // 20
    uint8_t  salt_2[32];    // 52
} PACK _v3_msg_handshake;

typedef struct {
    uint16_t unknown;       // 4
    uint16_t error;         // 6
    uint32_t subtype;       // 8
} PACK _v3_msg_auth;

typedef struct {
    uint8_t  unknown[48];   // 4
} PACK _v3_msg_chan_admin;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t error;         // 6
    uint16_t unknown_1;     // 8
    uint16_t count;         // 10
    uint16_t unknown_2;     // 12
    uint16_t unknown_3;     // 14
} PACK _v3_msg_rank_list;

typedef struct {
    uint16_t user;          // 4
    uint16_t sequence;      // 6
    uint16_t ping;          // 8
    uint16_t inactive;      // 10
} PACK _v3_msg_ping;

typedef struct {
    uint16_t id;            // 4
    uint16_t channel;       // 6
    uint32_t error;         // 8
} PACK _v3_msg_move;

typedef struct {
    uint16_t unknown_1;     // 4
    uint16_t unknown_2;     // 6
    int16_t  index;         // 8
    int16_t  format;        // 10
    uint8_t  unknown_3[12]; // 12
} PACK _v3_msg_srv_codec;

typedef struct {
    uint16_t user;          // 4
    uint16_t subtype;       // 6
    uint32_t unknown;       // 8
} PACK _v3_msg_chat;

typedef struct {
    uint16_t user;          // 4
    uint16_t subtype;       // 6
    uint32_t value;         // 8
} PACK _v3_msg_user_option;

typedef struct {
    uint32_t subtype;       // 4
    uint32_t unknown_1;     // 8
    uint32_t ip;            // 12
    uint16_t port;          // 16
    uint16_t remote_status; // 18
    uint16_t unknown_2;     // 20
    uint16_t handshake_idx; // 22
    uint8_t  handshake[16]; // 24
    char     version[64];   // 40
    char     protocol[16];  // 104
    uint8_t  password[32];  // 120
    char     username[32];  // 152
    char     phonetic[32];  // 184
    char     platform[64];  // 216
} PACK _v3_msg_login;

typedef struct {
    uint16_t user;          // 4
    uint16_t subtype;       // 6
    uint8_t  password[32];  // 8
} PACK _v3_msg_chan_list;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t error;         // 6
    uint16_t unknown_1;     // 8
    uint16_t count;         // 10
    uint16_t start;         // 12
    uint16_t end;           // 14
    uint32_t unknown_2;     // 16
} PACK _v3_msg_acct_list;

typedef struct {
    uint32_t timestamp;     // 4
    uint32_t unused;        // 8
} PACK _v3_msg_timestamp;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t property;      // 6
    uint16_t ignore;        // 8
    uint16_t transaction;   // 10
} PACK _v3_msg_srv_prop;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t user;          // 6
    int16_t  index;         // 8
    int16_t  format;        // 10
    uint16_t method;        // 12
    uint16_t unknown;       // 14
    uint32_t datalen;       // 16
    uint32_t pcmlen;        // 20
} PACK _v3_msg_audio;

typedef struct {
    uint16_t user;          // 4
    uint16_t channel;       // 6
} PACK _v3_msg_chan_change;

typedef struct {
    uint16_t unknown_1;     // 4
    uint16_t licensed;      // 6
    uint16_t port;          // 8
    uint16_t slots;         // 10
    uint16_t clients;       // 12
    uint16_t unknown_2[7];  // 14
    char     name[32];      // 28
    char     version[16];   // 60
    char     unknown_3[32]; // 76
} PACK _v3_msg_srv_info;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t error;         // 6
    uint16_t unknown_1;     // 8
    uint16_t user;          // 10
    uint16_t phantom;       // 12
    uint16_t channel;       // 14
    uint16_t unknown_2;     // 16
    uint16_t unknown_3;     // 18
} PACK _v3_msg_phantom;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t sum_1;         // 6
    uint32_t sum_2;         // 8
} PACK _v3_msg_hash_table;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t count;         // 6
} PACK _v3_msg_user_list;

typedef struct {
    uint32_t count;         // 4
} PACK _v3_msg_list_chan;

typedef struct {
    uint16_t to;            // 4
    uint16_t from;          // 6
    uint32_t error;         // 8
} PACK _v3_msg_user_page;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t user;          // 6
    uint32_t unused;        // 8
    uint8_t  data[128];     // 12
} PACK _v3_msg_admin;

#endif

