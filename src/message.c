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

#include "message.h"

_v3_message *
_v3_msg_alloc(v3_handle v3h, uint32_t type, size_t len, void **contents) {
    const char func[] = "_v3_msg_alloc";

    _v3_message *m;

    _v3_enter(v3h, func);

    _v3_debug(v3h, V3_DBG_MEMORY, "allocating message: %u bytes", sizeof(_v3_message));
    m = malloc(sizeof(_v3_message));
    memset(m, 0, sizeof(_v3_message));

    m->type = type;
    m->len = len;

    if (len) {
        _v3_debug(v3h, V3_DBG_MEMORY, "allocating contents: %u bytes", len);
        m->contents = malloc(len);
        memset(m->contents, 0, len);
    }
    if (contents) {
        *contents = m->contents;
    }

    _v3_leave(v3h, func);
    return m;
}

void
_v3_msg_free(v3_handle v3h, _v3_message *m) {
    const char func[] = "_v3_msg_free";

    _v3_enter(v3h, func);

    if (!m) {
        _v3_leave(v3h, func);
        return;
    }
    if (m->data && m->data != m->contents) {
        _v3_debug(v3h, V3_DBG_MEMORY, "releasing data");
        free(m->data);
    }
    if (m->contents) {
        _v3_debug(v3h, V3_DBG_MEMORY, "releasing contents");
        free(m->contents);
    }
    _v3_debug(v3h, V3_DBG_MEMORY, "releasing message");
    free(m);

    _v3_leave(v3h, func);
}

void *
_v3_msg_string_get(v3_handle v3h, void *src, char *dest, size_t n) {
    const char func[] = "_v3_msg_string_get";

    uint16_t len;

    _v3_enter(v3h, func);

    len = ntohs(*(uint16_t *)src);
    src += sizeof(len);
    strncpy(dest, src, (len > n) ? n : len);
    src += len;

    _v3_leave(v3h, func);
    return src;
}

void *
_v3_msg_channel_get(v3_handle v3h, void *src, v3_channel *c) {
    const char func[] = "_v3_msg_channel_get";

    int len;

    _v3_enter(v3h, func);

    len = (void *)&c->_internal_ - (void *)c;
    memcpy(c, src, len);
    src += len;
    src = _v3_msg_string_get(v3h, src, c->name, sizeof(c->name) - 1);
    src = _v3_msg_string_get(v3h, src, c->phonetic, sizeof(c->phonetic) - 1);
    src = _v3_msg_string_get(v3h, src, c->comment, sizeof(c->comment) - 1);

    _v3_leave(v3h, func);
    return src;
}

void *
_v3_msg_rank_get(v3_handle v3h, void *src, v3_rank *r) {
    const char func[] = "_v3_msg_rank_get";

    int len;

    _v3_enter(v3h, func);

    len = (void *)&r->_internal_ - (void *)r;
    memcpy(r, src, len);
    src += len;
    src = _v3_msg_string_get(v3h, src, r->name, sizeof(r->name) - 1);
    src = _v3_msg_string_get(v3h, src, r->description, sizeof(r->description) - 1);

    _v3_leave(v3h, func);
    return src;
}

void *
_v3_msg_user_get(v3_handle v3h, void *src, v3_user *u) {
    const char func[] = "_v3_msg_user_get";

    int len;

    _v3_enter(v3h, func);

    len = (void *)&u->_internal_ - (void *)u;
    memcpy(u, src, len);
    src += len;
    src = _v3_msg_string_get(v3h, src, u->name, sizeof(u->name) - 1);
    src = _v3_msg_string_get(v3h, src, u->phonetic, sizeof(u->phonetic) - 1);
    src = _v3_msg_string_get(v3h, src, u->comment, sizeof(u->comment) - 1);
    src = _v3_msg_string_get(v3h, src, u->integration, sizeof(u->integration) - 1);
    src = _v3_msg_string_get(v3h, src, u->url, sizeof(u->url) - 1);

    _v3_leave(v3h, func);
    return src;
}

_v3_message *
_v3_msg_handshake_put(v3_handle v3h) {
    const char func[] = "_v3_msg_handshake_put";

    _v3_message *m;
    _v3_msg_handshake *mc;
    size_t ctr;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_HANDSHAKE, sizeof(_v3_msg_handshake), (void **)&mc);

    strncpy(mc->protocol, V3_PROTO_VERSION, sizeof(mc->protocol) - 1);
    for (ctr = 1; ctr < sizeof(mc->salt); ctr++) {
        if (ctr % (sizeof(mc->salt) / 2)) {
            mc->salt[ctr-1] = rand() % 93 + 33;
        }
    }

    m->data = mc;

    _v3_leave(v3h, func);
    return m;
}

_v3_message *
_v3_msg_login_put(v3_handle v3h) {
    const char func[] = "_v3_msg_login_put";

    _v3_connection *v3c;
    _v3_message *m;
    _v3_msg_login *mc;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_LOGIN, sizeof(_v3_msg_login), (void **)&mc);

    v3c = _v3_handles[v3h];
    mc->subtype = 2;
    mc->ip = v3c->ip;
    mc->port = v3c->port ^ 0xffff;
    mc->remote_status = true;
    mc->handshake_idx = v3c->handshake_idx;
    memcpy(mc->handshake, v3c->handshake, sizeof(mc->handshake));
    strncpy(mc->version, V3_CLIENT_VERSION, sizeof(mc->version) - 1);
    strncpy(mc->protocol, V3_PROTO_VERSION, sizeof(mc->protocol) - 1);
    if (*v3c->password) {
        _v3_password(v3h, v3c->password, mc->password);
    }
    strncpy(mc->username, v3c->luser.name, sizeof(mc->username) - 1);
    strncpy(mc->phonetic, v3c->luser.phonetic, sizeof(mc->phonetic) - 1);
    strncpy(mc->platform, V3_CLIENT_PLATFORM, sizeof(mc->platform) - 1);

    m->data = mc;

    _v3_leave(v3h, func);
    return m;
}

int
_v3_msg_process(v3_handle v3h, _v3_message *m) {
    const char func[] = "_v3_msg_process";

    _v3_connection *v3c;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    _v3_debug(v3h, V3_DBG_MESSAGE, "processing packet type 0x%02x (%u)", m->type, m->type);

    switch (m->type) {
      case V3_MSG_AUTH:
        {
            _v3_msg_auth *mc = m->data;
            int ret;

            switch (mc->subtype) {
              case V3_AUTH_DISCONNECT:
                break;
              case V3_AUTH_ADMIN:
                break;
              case V3_AUTH_KEYS:
                v3c->client_key = calloc(1, sizeof(ventrilo_key_ctx));
                v3c->server_key = calloc(1, sizeof(ventrilo_key_ctx));
                if (_v3_read_keys(
                        v3h,
                        v3c->client_key,
                        v3c->server_key,
                        &mc->enc_key,
                        m->len - ((void *)&mc->enc_key - m->data)) != V3_OK) {
                    _v3_error(v3h, "failed to retrieve encryption keys");
                    _v3_leave(v3h, func);
                    return V3_FAILURE;
                }
                if (!v3c->logged_in) {
                    m = _v3_msg_login_put(v3h);
                    ret = _v3_send(v3h, m);
                    _v3_msg_free(v3h, m);
                    _v3_leave(v3h, func);
                    return ret;
                }
                break;
              case V3_AUTH_INVALID:
                break;
              case V3_AUTH_DISABLED:
                break;
              default:
                _v3_debug(v3h, V3_DBG_MESSAGE, "packet type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                _v3_leave(v3h, func);
                return V3_NOTIMPL;
            }
        }
        break;
      case V3_MSG_SCRAMBLE:
        {
            _v3_debug(v3h, V3_DBG_MESSAGE, "scrambling client key");
            _v3_key_scramble(v3h, v3c->client_key, v3c->handshake_key);
            _v3_debug(v3h, V3_DBG_MESSAGE, "scrambling server key");
            _v3_key_scramble(v3h, v3c->server_key, v3c->handshake_key);
            v3c->logged_in = true;
        }
        break;
      case V3_MSG_RANK_LIST:
        {
            _v3_msg_rank_list *mc = m->data;
            v3_rank r;
            void *ptr;
            uint16_t ctr;

            _v3_mutex_lock(v3h);

            for (ptr = m->data + sizeof(_v3_msg_rank_list), ctr = 0; ctr < mc->count; ctr++) {
                memset(&r, 0, sizeof(v3_rank));
                ptr = _v3_msg_rank_get(v3h, ptr, &r);
                _v3_debug(v3h, V3_DBG_MESSAGE, "rank: id: %u | level: %u | name: '%s' | description: '%s'",
                        r.id,
                        r.level,
                        r.name,
                        r.description);
                switch (mc->subtype) {
                  case V3_RANK_LIST:
                  case V3_RANK_ADD:
                  case V3_RANK_MODIFY:
                    _v3_data_update(v3h, V3_DATA_RANK, &r);
                    break;
                  default:
                    _v3_debug(v3h, V3_DBG_MESSAGE, "packet type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ctr = mc->count;
                    break;
                }
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_SRV_CODEC:
        {
            _v3_msg_srv_codec *mc = m->data;

            _v3_debug(v3h, V3_DBG_MESSAGE, "default server codec: index: %u | format: %u", mc->index, mc->format);
            v3c->codec_index = mc->index;
            v3c->codec_format = mc->format;
        }
        break;
      case V3_MSG_USER_PERM:
        {
        
        }
        break;
      case V3_MSG_SRV_INFO:
        {
        
        }
        break;
      case V3_MSG_USER_LIST:
        {
            _v3_msg_user_list *mc = m->data;
            v3_user u;
            void *ptr;
            uint16_t ctr;

            _v3_mutex_lock(v3h);

            for (ptr = m->data + sizeof(_v3_msg_user_list), ctr = 0; ctr < mc->count; ctr++) {
                memset(&u, 0, sizeof(v3_user));
                ptr = _v3_msg_user_get(v3h, ptr, &u);
                _v3_debug(v3h, V3_DBG_MESSAGE, "user: id: %u | channel: %u | flags: %u | rank_id: %u",
                        u.id,
                        u.channel,
                        u.flags,
                        u.rank_id);
                _v3_debug(v3h, V3_DBG_MESSAGE, "name: '%s' | phonetic: '%s' | comment: '%s' | integration: '%s' | url: '%s'",
                        u.name,
                        u.phonetic,
                        u.comment,
                        u.integration,
                        u.url);
                switch (mc->subtype) {
                  case V3_USER_ADD:
                  case V3_USER_MODIFY:
                  case V3_USER_LIST:
                    _v3_data_update(v3h, V3_DATA_USER, &u);
                    break;
                  default:
                    _v3_debug(v3h, V3_DBG_MESSAGE, "packet type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ctr = mc->count;
                    break;
                }
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_LIST_CHAN:
        {
            _v3_msg_list_chan *mc = m->data;
            v3_channel c;
            void *ptr;
            uint16_t ctr;

            _v3_mutex_lock(v3h);

            for (ptr = m->data + sizeof(_v3_msg_list_chan), ctr = 0; ctr < mc->count; ctr++) {
                memset(&c, 0, sizeof(v3_channel));
                ptr = _v3_msg_channel_get(v3h, ptr, &c);
                _v3_debug(v3h, V3_DBG_MESSAGE, "channel: id: %u | name: '%s' | phonetic: '%s' | comment: '%s'",
                        c.id,
                        c.name,
                        c.phonetic,
                        c.comment);
                _v3_data_update(v3h, V3_DATA_CHANNEL, &c);
            }

            _v3_mutex_unlock(v3h);
        }
        break;
    }

    _v3_leave(v3h, func);
    return V3_OK;
}





















