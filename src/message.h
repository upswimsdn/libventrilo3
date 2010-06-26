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
    V3_MSG_CHAN_MOVE    = 0x3b,
    V3_MSG_SRV_CODEC    = 0x3c,
    V3_MSG_WAV          = 0x3f,
    V3_MSG_CHAT         = 0x42,
    V3_MSG_USER_OPTION  = 0x46,
    V3_MSG_LOGIN        = 0x48,
    V3_MSG_CHAN_LIST    = 0x49,
    V3_MSG_USER_PERM    = 0x4a,
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
    V3_MSG_CIPH_TABLE   = 0x5c,
    V3_MSG_USER_LIST    = 0x5d,
    V3_MSG_LIST_CHAN    = 0x60,
    V3_MSG_BAN_LIST     = 0x61,
    V3_MSG_PAGE         = 0x62,
    V3_MSG_ADMIN        = 0x63
};

enum {
    V2_MSG_HANDSHAKE    = 0x00,
    V2_MSG_AUTH         = 0x06,
    V2_MSG_MOTD         = 0x33,
    V2_MSG_SRV_INFO     = 0x34,
    V2_MSG_LOGIN        = 0x36,
    V2_MSG_ERROR        = 0x3a,
    V2_MSG_PAGE         = 0x3c,
    V2_MSG_CHAT         = 0x43,
    V2_MSG_USER_LIST    = 0x44,
    V2_MSG_CHAN_LIST    = 0x46,
    V2_MSG_WAV          = 0x4e,
    V2_MSG_SRV_PROP     = 0x52,
    V2_MSG_TTS          = 0x53,
    V2_MSG_ADMIN        = 0x54,
    V2_MSG_SCRAMBLE     = 0x56,
    V2_MSG_CHAN_CHANGE  = 0x58,
    V2_MSG_CIPH_TABLE   = 0x5a,
    V2_MSG_USER_PERM    = 0x5e,
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
    V3_RANK_MODIFY      = 0x05
};

enum {
    V3_USER_REMOVE      = 0x00,
    V3_USER_ADD         = 0x01,
    V3_USER_MODIFY      = 0x02,
    V3_USER_LIST        = 0x04,
    V3_USER_RANK        = 0x06
};

typedef struct {
    char     protocol[16];  // 4
    uint8_t  salt[64];      // 20
} PACK _v3_msg_handshake;

typedef struct {
    uint16_t unknown_1;     // 4
    uint16_t error_id;      // 6
    uint32_t subtype;       // 8
    uint8_t  enc_key;       // 12
} PACK _v3_msg_auth;

typedef struct {
    uint16_t subtype;       // 4
    uint16_t error_id;      // 6
    uint16_t unknown_1;     // 8
    uint16_t count;         // 10
    uint16_t unknown_2;     // 12
    uint16_t unknown_3;     // 14
} PACK _v3_msg_rank_list;

typedef struct {
    uint16_t unknown_1;     // 4
    uint16_t unknown_2;     // 6
    uint16_t index;         // 8
    uint16_t format;        // 10
    uint8_t  unknown_3[12]; // 12
} PACK _v3_msg_srv_codec;

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
    uint16_t subtype;       // 4
    uint16_t count;         // 6
} PACK _v3_msg_user_list;

typedef struct {
    uint32_t count;         // 4
} PACK _v3_msg_list_chan;

#endif

