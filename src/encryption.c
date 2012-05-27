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

/*
 * Copyright 2004,2005,2006,2007,2008,2009,2010 Luigi Auriemma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * http://www.gnu.org/licenses/gpl-2.0.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "libventrilo3.h"

void
_v3_password(v3_handle v3h, const char *password, uint8_t *hash) {
    uint32_t crc, i, j, cnt, len;
    uint8_t tmp[sizeof(crc)] = { 0 };

    _v3_enter(v3h, __func__);

    len = cnt = strlen(password);
    for (i = 0; i < 32; ++i, ++cnt) {
        hash[i] = (i < len) ? password[i] : ((tmp[(cnt + 1) & 3] + hash[i-len]) - 0x3f) & 0x7f;
        for (j = 0, crc = 0; j < i + 1; ++j) {
            crc = _v3_hash_table[hash[j] ^ (crc & 0xff)] ^ (crc >> 8);
        }
        *(typeof(crc) *)tmp = htonl(crc);
        cnt += hash[i];
        while (crc && !tmp[cnt & 3]) {
            ++cnt;
        }
        hash[i] += tmp[cnt & 3];
    }

    _v3_leave(v3h, __func__);
}

uint32_t
_v3_hash_table_scramble(v3_handle v3h, uint16_t type, uint32_t val) {
    uint8_t in[16];
    uint32_t out, i;

    _v3_enter(v3h, __func__);

    snprintf((char *)in, sizeof(in), (type == 0x05 || val) ? "%08x" : "%08X", (!val) ? (uint32_t)rand() : val);
    for (out = 0, i = 0; i < 8; ++i) {
        out = (out >> 8) ^ _v3_hash_table[(uint8_t)(in[i] ^ out)];
    }

    _v3_leave(v3h, __func__);
    return out;
}

uint32_t
_v3_hash_table_sum(v3_handle v3h, uint32_t type) {
    uint32_t seed, iter, out, i, j;
    uint8_t idx;

    _v3_enter(v3h, __func__);

    switch (type) {
      case 0x00:
      case 0x06:
      case 0x09:
        seed = 0xBAADF00D;
        iter = 16;
        break;
      case 0x02:
        seed = 0x0DBAADF0;
        iter = 16;
        break;
      case 0x04:
      case 0x08:
        seed = 0xBAADF00D;
        iter = 32;
        break;
    }
    for (out = 0, i = 0; i < iter; ++i) {
        for (j = 0; j < 4; ++j) {
            idx = ((seed >> (j * 8)) ^ out) & 0xff;
            out = (out >> 8) ^ _v3_hash_table[idx];
        }
    }

    _v3_leave(v3h, __func__);
    return _v3_hash_table_scramble(v3h, 0, out);
}

/*
 * Ventrilo Encryption/Decryption Algorithm 0.2a
 * by: Luigi Auriemma
 * email: aluigi@autistici.org
 * web: aluigi.org
 *
 * This algorithm is a method used by the chat program Ventrilo for
 * encrypting the communication stream between clients and servers.
 *
 * Thanks to:
 * Georg Hofstetter (http://www.g3gg0.de)
 */

#define VENTRILO_KEY_INIT "\xAA\x55\x22\xCC\x69\x7C\x38\x91\x88\xF5\xE1"

int
_v3_read_keys(v3_handle v3h, v3_key_ctx *client, v3_key_ctx *server, uint8_t *data, uint32_t len) {
    v3_key_ctx *tmp;
    uint32_t ctr;
    int del;

    _v3_enter(v3h, __func__);

    del = -1;
    for (ctr = 0; ctr < len && data[ctr]; ++ctr) {
        if (del >= 0) {
            continue;
        }
        if (data[ctr] == '|') { // 2.3 (protocol 2), 3.0 (protocol 3) and so on
            del = ctr;
        } else if (data[ctr] == ',') { // 2.1 (protocol 1)
            del = ctr;
            tmp = server;
            server = client;
            client = tmp;
        }
    }
    if (del < 0) {
        _v3_leave(v3h, __func__);
        return V3_FAILURE;
    }
    len = ctr;

    server->size = len - (del + 1);
    client->size = del;
    if (client->size > 256 || server->size > 256) {
        _v3_leave(v3h, __func__);
        return V3_FAILURE;
    }
    client->pos = 0;
    server->pos = 0;
    memcpy(client->key, data, client->size);
    memcpy(server->key, data + del + 1, server->size);

    _v3_leave(v3h, __func__);
    return V3_OK;
}

void
_v3_dec_init(v3_handle v3h, uint8_t *data, uint32_t len) {
    uint32_t ctr;

    _v3_enter(v3h, __func__);

    for (ctr = 0; ctr < len; ++ctr) {
        data[ctr] -= VENTRILO_KEY_INIT[ctr % 11] + (ctr % 27);
    }

    _v3_leave(v3h, __func__);
}

void
_v3_enc_init(v3_handle v3h, uint8_t *data, uint32_t len) {
    uint32_t ctr;

    _v3_enter(v3h, __func__);

    for (ctr = 0; ctr < len; ++ctr) {
        data[ctr] += VENTRILO_KEY_INIT[ctr % 11] + (ctr % 27);
    }

    _v3_leave(v3h, __func__);
}

void
_v3_decrypt(v3_handle v3h, v3_key_ctx *ctx, uint8_t *data, uint32_t len) {
    uint32_t ctr;

    _v3_enter(v3h, __func__);

    for (ctr = 0; ctr < len; ++ctr) {
        data[ctr] -= ctx->key[ctx->pos] + (ctr % 45);
        ++ctx->pos;
        if (ctx->pos == ctx->size) {
            ctx->pos = 0;
        }
    }

    _v3_leave(v3h, __func__);
}

void
_v3_encrypt(v3_handle v3h, v3_key_ctx *ctx, uint8_t *data, uint32_t len) {
    uint32_t ctr;

    _v3_enter(v3h, __func__);

    for (ctr = 0; ctr < len; ++ctr) {
        data[ctr] += ctx->key[ctx->pos] + (ctr % 45);
        ++ctx->pos;
        if (ctx->pos == ctx->size) {
            ctx->pos = 0;
        }
    }

    _v3_leave(v3h, __func__);
}

