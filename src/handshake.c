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
 * Copyright 2008,2009,2010 Luigi Auriemma
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

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "libventrilo3.h"

#define V3_SEND_UDP "======= sending UDP packet ======================================"
#define V3_RECV_UDP "======= received UDP packet ====================================="

uint32_t
getbe(uint8_t *data, uint32_t *ret, uint32_t bits) {
    uint32_t i, bytes, num;

    bytes = bits >> 3;
    for (num = i = 0; i < bytes; ++i) {
        num |= (data[i] << ((bytes - 1 - i) << 3));
    }
    if (!ret) {
        return num;
    }
    *ret = num;

    return bytes;
}

uint32_t
putbe(uint8_t *data, uint32_t num, uint32_t bits) {
    uint32_t i, bytes;

    bytes = bits >> 3;
    for (i = 0; i < bytes; ++i) {
        data[i] = (num >> ((bytes - 1 - i) << 3)) & 0xff;
    }

    return bytes;
}

int
_v3_handshake(v3_handle v3h) {
    _v3_connection *v3c;
    int len, i;
    uint8_t sbuf[0x200], rbuf[0x200];

    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];
    if (_v3_connect(v3h, false) != V3_OK) {
        _v3_leave(v3h, __func__);
        return V3_FAILURE;
    }
    memset(rbuf, 0, sizeof(rbuf));
    len = _v3_udp_header(v3h, 4, sbuf, rbuf);
    putbe(sbuf+12, 2, 8);
    putbe(sbuf+16, 1, 16);
    putbe(sbuf+18, time(NULL), 16);
    _v3_udp_send(v3h, v3c->sd, -1, v3c->ip, v3c->port, sbuf, len);
    if ((len = _v3_udp_recv(v3h, v3c->sd, rbuf, sizeof(rbuf), NULL, NULL)) <= 0) {
        _v3_error(v3h, "udp receive failed: %s", (!len) ? "timed out" : strerror(errno));
        _v3_close(v3h);
        _v3_leave(v3h, __func__);
        return V3_FAILURE;
    }
    if (!strncmp((char *)rbuf+56, "DDDDDDDDDDDDDDDD", 16)) {
        _v3_close(v3h);
        v3c->handshake_idx = 1;
        memset(v3c->handshake_key, 0x44, sizeof(v3c->handshake_key));
        memset(v3c->handshake, 0x44, sizeof(v3c->handshake));
        _v3_leave(v3h, __func__);
        return V3_OK;
    }
    for (i = 0; v3_auth[i].host; ++i) {
        len = _v3_udp_header(v3h, 5, sbuf, rbuf);
        _v3_udp_send(v3h, v3c->sd, v3_auth[i].vnum, inet_addr(v3_auth[i].host), v3_auth[i].port, sbuf, len);
    }
    for (;;) {
        if ((len = _v3_udp_recv(v3h, v3c->sd, rbuf, sizeof(rbuf), v3_auth, &v3c->handshake_idx)) <= 0) {
            _v3_error(v3h, "udp receive failed: %s", (!len) ? "timed out" : strerror(errno));
            _v3_close(v3h);
            _v3_leave(v3h, __func__);
            return V3_FAILURE;
        }
        if (len >= 92 + 16) {
            break;
        }
    }
    _v3_close(v3h);
    memcpy(v3c->handshake_key, rbuf+28, sizeof(v3c->handshake_key));
    memcpy(v3c->handshake, rbuf+92, sizeof(v3c->handshake));

    _v3_leave(v3h, __func__);
    return V3_OK;
}

void
_v3_key_scramble(v3_handle v3h, v3_key_ctx *ctx, uint8_t *v3key) {
    uint32_t i, keylen;
    uint8_t *key;

    _v3_enter(v3h, __func__);

    key = ctx->key;
    if (ctx->size < 64) {
        memset(key + ctx->size, 0, 64 - ctx->size);
        ctx->size = 64;
    }
    keylen = ctx->size;
    for (i = 0; i < keylen; ++i) {
        if (i < 64) {
            key[i] += v3key[i];
        } else {
            key[i] += i + keylen;
        }
        if (!key[i]) {
            key[i] = i + 36;
        }
    }
    ctx->pos = 0;

    _v3_leave(v3h, __func__);
}

uint16_t
_v3_udp_header(v3_handle v3h, uint32_t type, uint8_t *buf, uint8_t *pck) {
    uint16_t c;

    _v3_enter(v3h, __func__);

    memset(buf, 0, 0x200);
    switch (type - 1) {
        case 0:  c = 0xb4;  break;
        case 1:  c = 0x70;  break;
        case 2:  c = 0x24;  break;
        case 3:  c = 0xb8;  break;
        case 4:  c = 0x74;  break;
        case 5:  c = 0x5c;  break;
        case 6:  c = 0xd0;  break;
        case 7:  c = 0x08;  break;
        case 8:  c = 0x50;  break;
        default: c = 0;     break;
    }
    c += 0x10;
    putbe(buf+8, type, 16);
    putbe(buf+10, c, 16);
    buf[4] = 'U';
    buf[5] = 'D';
    buf[6] = 'C';
    buf[7] = 'L';
    buf[12] = 1;
    putbe(buf+16, 0xb401, 32); // x[seq], recheck
    putbe(buf+20, getbe(pck+20, NULL, 32), 32);
    putbe(buf+24, getbe(pck+24, NULL, 32), 32);
    putbe(buf+28, getbe(pck+28, NULL, 32), 32);
    putbe(buf+32, getbe(pck+32, NULL, 32), 32);
    putbe(buf+36, getbe(pck+36, NULL, 32), 32);
    buf[40] = pck[48];
    buf[41] = 0;
    buf[42] = 0;
    buf[43] = 0;
    putbe(buf+44, getbe(pck+36, NULL, 16), 16);
    putbe(buf+46, 0, 16);
    putbe(buf+48, getbe(pck+20, NULL, 16), 16);
    memcpy(buf+52, pck+56, 16); // hash
    memcpy(buf+68, pck+136, 32); // platform
    buf[99] = 0;
    memcpy(buf+100, pck+168, 32); // version
    buf[131] = 0;

    _v3_leave(v3h, __func__);
    return c;
}

int
_v3_udp_send(v3_handle v3h, int sd, uint32_t vnum, uint32_t ip, uint16_t port, uint8_t *data, uint32_t len) {
    _v3_connection *v3c;
    struct sockaddr_in sa;
    uint32_t i, k;
    uint8_t tmp[4];
    int ret;

    _v3_enter(v3h, __func__);

    if (vnum != (uint32_t)-1) {
        tmp[0] = ip;
        tmp[1] = ip >> 8;
        tmp[2] = ip >> 16;
        tmp[3] = ip >> 24;
        k = (tmp[0] & 0x0f) * vnum;
        for (i = 16; i < len; ++i, ++k) {
            data[i] += tmp[k & 3];
        }
    }
    sa.sin_addr.s_addr = ip;
    sa.sin_port = htons(port);
    sa.sin_family = AF_INET;
    _v3_debug(v3h, V3_DBG_SOCKET, "sending udp packet: '%s:%hu'", inet_ntoa(sa.sin_addr), port);
    _v3_debug(v3h, V3_DBG_PACKET, V3_SEND_UDP);
    _v3_packet(v3h, data, len);
    if ((ret = sendto(sd, data, len, 0, (struct sockaddr *)&sa, sizeof(sa))) > 0) {
        v3c = _v3_handles[v3h];
        ++v3c->sent_pkt_ctr;
        v3c->sent_byte_ctr += ret;
    }

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_udp_recv(v3_handle v3h, int sd, uint8_t *data, uint32_t maxsz, const v3_auth_t *vauth, uint16_t *handshake_idx) {
    _v3_connection *v3c;
    struct timeval tout;
    fd_set rfds;
    struct sockaddr_in sa;
    uint32_t sasz, vnum;
    int len, i;
    uint32_t k;
    uint32_t ip;
    uint8_t tmp[4];

    _v3_enter(v3h, __func__);

    tout.tv_sec = 2;
    tout.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(sd, &rfds);
    _v3_debug(v3h, V3_DBG_SOCKET, "waiting for data...");
    if (select(sd+1, &rfds, NULL, NULL, &tout) <= 0) {
        _v3_leave(v3h, __func__);
        return 0;
    }
    sasz = sizeof(sa);
    if ((len = recvfrom(sd, data, maxsz, 0, (struct sockaddr *)&sa, &sasz)) < 0) {
        _v3_leave(v3h, __func__);
        return len;
    }
    if (len > 0) {
        v3c = _v3_handles[v3h];
        ++v3c->recv_pkt_ctr;
        v3c->recv_byte_ctr += len;
    }
    _v3_debug(v3h, V3_DBG_SOCKET, "received udp packet: '%s:%hu'", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
    _v3_debug(v3h, V3_DBG_PACKET, V3_RECV_UDP);
    _v3_packet(v3h, data, len);
    if (!vauth) {
        _v3_leave(v3h, __func__);
        return len;
    }
    for (i = 0; vauth[i].host; ++i) {
        ip = inet_addr(vauth[i].host);
        if (ip == sa.sin_addr.s_addr) {
            vnum = vauth[i].vnum;
            break;
        }
    }
    if (!vauth[i].host) {
        _v3_leave(v3h, __func__);
        return 0;
    }
    *handshake_idx = i;
    if ((data[10] | (data[11] << 8)) > 16) {
        tmp[0] = ip;
        tmp[1] = ip >> 8;
        tmp[2] = ip >> 16;
        tmp[3] = ip >> 24;
        k = (tmp[0] & 0x0f) * vnum;
        for (i = 16; i < len; ++i, ++k) {
            data[i] -= tmp[k & 3];
        }
    }

    _v3_leave(v3h, __func__);
    return len;
}

