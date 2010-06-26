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

#define V3_SEND_TCP "======= sending TCP packet ======================================"
#define V3_RENC_TCP "======= receiving encrypted TCP packet =========================="
#define V3_RECV_TCP "======= received TCP packet ====================================="

int
_v3_connected(v3_handle v3h) {
    const char func[] = "_v3_connected";

    _v3_connection *v3c;
    int connected;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    connected = (v3c->sd >= 0);
    _v3_debug(v3h, V3_DBG_SOCKET, "%sconnected with socket descriptor: %i", (!connected) ? "not " : "", v3c->sd);

    _v3_leave(v3h, func);
    return connected;
}

int
_v3_canceling(v3_handle v3h) {
    const char func[] = "_v3_canceling";

    _v3_connection *v3c;
    int cancel;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    cancel = v3c->cancel;
    _v3_debug(v3h, V3_DBG_SOCKET, "login %scanceled", (!cancel) ? "not " : "");

    _v3_leave(v3h, func);
    return cancel;
}

void
_v3_close(v3_handle v3h) {
    const char func[] = "_v3_close";

    _v3_connection *v3c;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (_v3_connected(v3h)) {
        close(v3c->sd);
        v3c->sd = -1;
    }
    if (v3c->client_key) {
        _v3_debug(v3h, V3_DBG_MEMORY, "releasing client key");
        free(v3c->client_key);
        v3c->client_key = NULL;
    }
    if (v3c->server_key) {
        _v3_debug(v3h, V3_DBG_MEMORY, "releasing server key");
        free(v3c->server_key);
        v3c->server_key = NULL;
    }

    _v3_leave(v3h, func);
}

int
_v3_connect(v3_handle v3h, int tcp) {
    const char func[] = "_v3_connect";

    _v3_connection *v3c;
    struct linger ling = { 1, 1 };
    struct sockaddr_in sa;

    _v3_enter(v3h, func);

    _v3_close(v3h);
    v3c = _v3_handles[v3h];
    if ((v3c->sd = socket(AF_INET, (tcp) ? SOCK_STREAM : SOCK_DGRAM, (tcp) ? IPPROTO_TCP : IPPROTO_UDP)) < 0) {
        _v3_error(v3h, "failed to create %s socket: %s", (tcp) ? "tcp" : "udp", strerror(errno));
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }
    setsockopt(v3c->sd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
    if (tcp) {
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = v3c->ip;
        sa.sin_port = htons(v3c->port);
        if (connect(v3c->sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            _v3_error(v3h, "failed to connect: %s", strerror(errno));
            _v3_close(v3h);
            _v3_leave(v3h, func);
            return V3_FAILURE;
        }
        _v3_debug(v3h, V3_DBG_STATUS, "tcp connected: '%s:%hu'", inet_ntoa(sa.sin_addr), v3c->port);
    }

    _v3_leave(v3h, func);
    return V3_OK;
}

int
_v3_send(v3_handle v3h, _v3_message *m) {
    const char func[] = "_v3_send";

    _v3_connection *v3c;
    uint8_t buf[0xffff];
    uint8_t *data;
    uint16_t len;

    _v3_enter(v3h, func);

    data = buf;
    len = htons(sizeof(m->type) + m->len);
    memcpy(data, &len, sizeof(len));
    data += sizeof(len);
    memcpy(data, &m->type, sizeof(m->type));
    len = sizeof(m->type);
    memcpy(data + len, m->data, m->len);
    len += m->len;

    //TODO: mutex_lock

    v3c = _v3_handles[v3h];
    _v3_debug(v3h, V3_DBG_PACKET, V3_SEND_TCP);
    _v3_packet(v3h, data, len);
    if (!v3c->client_key) {
        _v3_enc_init(v3h, data, len);
    } else {
        _v3_encrypt(v3h, v3c->client_key, data, len);
    }
    len += sizeof(len);
    v3c->sent_pkt_ctr++;
    v3c->sent_byte_ctr += len;
    _v3_debug(v3h, V3_DBG_SOCKET, "sending %u bytes; session: %u bytes, %u packets", len, v3c->sent_byte_ctr, v3c->sent_pkt_ctr);
    if (send(v3c->sd, buf, len, 0) < 0) {
        _v3_error(v3h, "failed to send message: %s", strerror(errno));
        //TODO: mutex_unlock
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }

    //TODO: mutex_unlock

    _v3_leave(v3h, func);
    return V3_OK;
}

void *
_v3_recv(v3_handle v3h, int block) {
    const char func[] = "_v3_recv";

    _v3_connection *v3c;
    _v3_message *m;
    fd_set rfds;
    struct timeval tv, zero = { 0, 0 };
    int ret, ctr;
    uint16_t rem;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    while (_v3_connected(v3h)) {
        tv.tv_sec = 10;tv.tv_usec = 0;//TODO: timestamp
        FD_ZERO(&rfds);
        FD_SET(v3c->sd, &rfds);
        if (block) {
            _v3_debug(v3h, V3_DBG_SOCKET, "waiting for data...");
        }
        if ((ret = select(v3c->sd+1, &rfds, NULL, NULL, (block) ? &tv : &zero)) < 0 || !_v3_connected(v3h)) {
            if (_v3_connected(v3h)) {
                _v3_error(v3h, "select failed: %s", strerror(errno));
            } else {
                _v3_debug(v3h, V3_DBG_STATUS, "disconnected");
            }
            break;
        }
        //TODO: timestamp
        if (!ret && block) {
            continue;
        }
        if (!FD_ISSET(v3c->sd, &rfds)) {
            _v3_debug(v3h, V3_DBG_SOCKET, "no data waiting to be received");
            break;
        }
        _v3_debug(v3h, V3_DBG_PACKET, V3_RENC_TCP);
        m = _v3_msg_alloc(v3h, 0, 0, NULL);
        for (ctr = 0; !m->data || ctr < m->len; ctr += ret, rem = m->len - ctr) {
            if ((ret = recv(v3c->sd, (!m->data) ? &m->len : m->data+ctr, (!m->data) ? sizeof(m->len) : rem, 0)) <= 0) {
                _v3_error(v3h, (!ret) ? "server closed connection" : strerror(errno));
                _v3_msg_free(v3h, m);
                //TODO: close
                _v3_leave(v3h, func);
                return NULL;
            }
            if (!m->data) {
                m->len = ntohs(m->len);
                _v3_debug(v3h, V3_DBG_SOCKET, "receiving %i bytes", m->len);
                m->data = malloc(m->len);
                memset(m->data, 0, m->len);
                ret = 0;
            } else {
                _v3_debug(v3h, V3_DBG_SOCKET, "received %i of %u remaining bytes", ret, rem);
            }
        }
        if (!v3c->server_key) {
            _v3_dec_init(v3h, m->data, m->len);
        } else {
            _v3_decrypt(v3h, v3c->server_key, m->data, m->len);
        }
        memcpy(&m->type, m->data, sizeof(m->type));
        v3c->recv_pkt_ctr++;
        v3c->recv_byte_ctr += m->len;
        _v3_debug(v3h, V3_DBG_SOCKET, "received %i bytes; session: %u bytes, %u packets", ctr, v3c->recv_byte_ctr, v3c->recv_pkt_ctr);
        _v3_debug(v3h, V3_DBG_PACKET, V3_RECV_TCP);
        _v3_packet(v3h, m->data, m->len);
        m->len -= sizeof(m->type);
        memmove(m->data, m->data + sizeof(m->type), m->len);
        _v3_leave(v3h, func);
        return m;
    }

    _v3_leave(v3h, func);
    return NULL;
}

int
v3_login(v3_handle v3h) {
    const char func[] = "v3_login";

    _v3_connection *v3c;
    _v3_message *m = NULL;
    int ret = V3_FAILURE;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];
    /*if (v3c->connecting) {
        _v3_mutex_unlock(v3h);
        _v3_error(v3h, "already connecting; try cancel first");
        _v3_leave(v3h, func);
        return V3_FAILURE;
    } else*/ if (_v3_connected(v3h)) {
        _v3_mutex_unlock(v3h);
        _v3_error(v3h, "already connected; try logout first");
        _v3_leave(v3h, func);
        return V3_FAILURE;
    } /*else if (_v3_canceling(v3h)) {
        _v3_mutex_unlock(v3h);
        _v3_error(v3h, "cancel in progress");
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }*/
    v3c->connecting = true;
    if (_v3_handshake(v3h) == V3_OK &&
        _v3_connect(v3h, true) == V3_OK &&
        _v3_send(v3h, (m = _v3_msg_handshake_put(v3h))) == V3_OK) {
        for (;;) {
            if (m->type == V3_MSG_SCRAMBLE) {
                ret = V3_OK;
                //break;
            }
            _v3_msg_free(v3h, m);
            if (!(m = _v3_recv(v3h, V3_BLOCK)) || _v3_msg_process(v3h, m) == V3_FAILURE) {
                break;
            }
        }
    }
    _v3_msg_free(v3h, m);
    if (ret == V3_FAILURE) {
        _v3_close(v3h);
    }
    v3c->connecting = false;

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return ret;
}

int
v3_login_cancel(v3_handle v3h) {
    const char func[] = "v3_login_cancel";

    _v3_connection *v3c;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (v3c->connecting && !_v3_canceling(v3h)) {
        _v3_debug(v3h, V3_DBG_INFO, "canceling login");
        v3c->cancel = true;
        _v3_close(v3h);
    } else if (!v3c->connecting && _v3_connected(v3h)) {
        _v3_error(v3h, "already connected; try logout");
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }

    _v3_leave(v3h, func);
    return V3_OK;
}

int
v3_iterate(v3_handle v3h, uint8_t block) {
    const char func[] = "v3_iterate";

    _v3_connection *v3c;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (v3c->connecting) {
        
    }

    _v3_leave(v3h, func);
    return V3_OK;
}


























