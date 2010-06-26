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

void
_v3_password(v3_handle v3h, char *password, uint8_t *hash) {
    const char func[] = "_v3_password";

    uint32_t crc, i, j, cnt, len;
    uint8_t  tmp[4] = { 0 };

    _v3_enter(v3h, func);

    len = cnt = strlen(password);
    for (i = 0; i < 32; i++, cnt++) {
        hash[i] = (i < len) ? password[i] : ((tmp[(cnt + 1) & 3] + hash[i-len]) - 0x3f) & 0x7f;
        for (j = 0, crc = 0; j < i + 1; j++) {
            crc = _v3_hash_table[hash[j] ^ (crc & 0xff)] ^ (crc >> 8);
        }
        *(uint32_t *)tmp = htonl(crc);
        cnt += hash[i];
        while (crc && !tmp[cnt & 3] && ++cnt);
        hash[i] += tmp[cnt & 3];
    }

    _v3_leave(v3h, func);
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
_v3_read_keys(v3_handle v3h, ventrilo_key_ctx *client, ventrilo_key_ctx *server, uint8_t *data, uint32_t len) {
    const char func[] = "_v3_read_keys";

    ventrilo_key_ctx *tmp;
    uint32_t ctr;
    int del;

    _v3_enter(v3h, func);

    del = -1;
    for (ctr = 0; ctr < len && data[ctr]; ctr++) {
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
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }
    len = ctr;

    server->size = len - (del + 1);
    client->size = del;
    if (client->size > 256 || server->size > 256) {
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }
    client->pos = 0;
    server->pos = 0;
    memcpy(client->key, data, client->size);
    memcpy(server->key, data + del + 1, server->size);

    _v3_leave(v3h, func);
    return V3_OK;
}

void
_v3_dec_init(v3_handle v3h, uint8_t *data, uint32_t len) {
    const char func[] = "_v3_dec_init";

    static const uint8_t first[] = VENTRILO_KEY_INIT;
    uint32_t ctr;

    _v3_enter(v3h, func);

    for (ctr = 0; ctr < len; ctr++) {
        data[ctr] -= first[ctr % 11] + (ctr % 27);
    }

    _v3_leave(v3h, func);
}

void
_v3_enc_init(v3_handle v3h, uint8_t *data, uint32_t len) {
    const char func[] = "_v3_enc_init";

    static const uint8_t first[] = VENTRILO_KEY_INIT;
    uint32_t ctr;

    _v3_enter(v3h, func);

    for (ctr = 0; ctr < len; ctr++) {
        data[ctr] += first[ctr % 11] + (ctr % 27);
    }

    _v3_leave(v3h, func);
}

void
_v3_decrypt(v3_handle v3h, ventrilo_key_ctx *ctx, uint8_t *data, uint32_t len) {
    const char func[] = "_v3_decrypt";

    uint32_t ctr;

    _v3_enter(v3h, func);

    for (ctr = 0; ctr < len; ctr++) {
        data[ctr] -= ctx->key[ctx->pos] + (ctr % 45);
        ctx->pos++;
        if (ctx->pos == ctx->size) {
            ctx->pos = 0;
        }
    }

    _v3_leave(v3h, func);
}

void
_v3_encrypt(v3_handle v3h, ventrilo_key_ctx *ctx, uint8_t *data, uint32_t len) {
    const char func[] = "_v3_encrypt";

    uint32_t ctr;

    _v3_enter(v3h, func);

    for (ctr = 0; ctr < len; ctr++) {
        data[ctr] += ctx->key[ctx->pos] + (ctr % 45);
        ctx->pos++;
        if (ctx->pos == ctx->size) {
            ctx->pos = 0;
        }
    }

    _v3_leave(v3h, func);
}

