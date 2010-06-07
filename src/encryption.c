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
ventrilo_read_keys(ventrilo_key_ctx *client, ventrilo_key_ctx *server, uint8_t *data, int size) {
    ventrilo_key_ctx *tmp;
    int i, del;

    del = -1;
    for (i = 0; i < size && data[i]; i++) {
        if (del >= 0) {
            continue;
        }
        if (data[i] == '|') { // 2.3 (protocol 2), 3.0 (protocol 3) and so on
            del = i;
        } else if (data[i] == ',') { // 2.1 (protocol 1)
            del = i;
            tmp = server;
            server = client;
            client = tmp;
        }
    }
    if (del < 0) {
        return V3_FAILURE;
    }
    size = i;

    server->size = size - (del + 1);
    client->size = del;
    if (client->size > 256 || server->size > 256) {
        return V3_FAILURE;
    }
    client->pos = 0;
    server->pos = 0;
    memcpy(client->key, data, client->size);
    memcpy(server->key, data + del + 1, server->size);

    return V3_OK;
}

void
ventrilo_dec_init(uint8_t *data, int size) {
    static const uint8_t first[] = VENTRILO_KEY_INIT;
    int i;

    for (i = 0; i < size; i++) {
        data[i] -= first[i % 11] + (i % 27);
    }
}

void
ventrilo_enc_init(uint8_t *data, int size) {
    static const uint8_t first[] = VENTRILO_KEY_INIT;
    int i;

    for (i = 0; i < size; i++) {
        data[i] += first[i % 11] + (i % 27);
    }
}

void
ventrilo_dec(ventrilo_key_ctx *ctx, uint8_t *data, int size) {
    int i;

    for (i = 0; i < size; i++) {
        data[i] -= ctx->key[ctx->pos] + (i % 45);
        ctx->pos++;
        if (ctx->pos == ctx->size) {
            ctx->pos = 0;
        }
    }
}

void
ventrilo_enc(ventrilo_key_ctx *ctx, uint8_t *data, int size) {
    int i;

    for (i = 0; i < size; i++) {
        data[i] += ctx->key[ctx->pos] + (i % 45);
        ctx->pos++;
        if (ctx->pos == ctx->size) {
            ctx->pos = 0;
        }
    }
}

