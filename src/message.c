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

void *
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
        _v3_debug(v3h, V3_DBG_MEMORY, "allocating data: %u bytes", len);
        m->data = malloc(len);
        memset(m->data, 0, len);
    }
    if (contents) {
        *contents = m->data;
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
    if (m->data) {
        _v3_debug(v3h, V3_DBG_MEMORY, "releasing data");
        free(m->data);
    }
    _v3_debug(v3h, V3_DBG_MEMORY, "releasing message");
    free(m);

    _v3_leave(v3h, func);
}

const void *
_v3_msg_string_get(v3_handle v3h, const void *src, char *dest, size_t n) {
    const char func[] = "_v3_msg_string_get";

    uint16_t len;

    _v3_enter(v3h, func);

    len = ntohs(*(uint16_t *)src);
    src += sizeof(len);
    memcpy(dest, src, (len > n) ? n : len);
    if (len < n) {
        dest[len] = 0;
    }
    src += len;

    _v3_leave(v3h, func);
    return src;
}

size_t
_v3_msg_string_put(v3_handle v3h, const char *src, size_t n, void **dest, size_t size) {
    const char func[] = "_v3_msg_string_put";

    uint16_t len;

    _v3_enter(v3h, func);

    len = strlen(src);
    len = (len > n) ? n : len;
    *dest = realloc(*dest, size + sizeof(len) + len);
    *((uint16_t *)(*dest + size)) = htons(len);
    size += sizeof(len);
    memcpy(*dest + size, src, len);
    size += len;

    _v3_leave(v3h, func);
    return size;
}

const void *
_v3_msg_uint16_get(v3_handle v3h, const void *src, uint16_t *dest, size_t n, uint16_t *count) {
    const char func[] = "_v3_msg_uint16_get";

    uint16_t len;
    size_t ctr;

    _v3_enter(v3h, func);

    len = ntohs(*(uint16_t *)src);
    if (count) {
        *count = (len > n) ? n : len;
    }
    src += sizeof(len);
    for (ctr = 0; ctr < ((len > n) ? n : len); ctr++) {
        dest[ctr] = ntohs(*((uint16_t *)src + ctr));
    }
    src += len * sizeof(*dest);

    _v3_leave(v3h, func);
    return src;
}

const void *
_v3_msg_channel_get(v3_handle v3h, const void *src, v3_channel *c) {
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

size_t
_v3_msg_channel_put(v3_handle v3h, const v3_channel *c, void **dest, size_t size, int skip) {
    const char func[] = "_v3_msg_channel_put";

    int len;

    _v3_enter(v3h, func);

    len = (void *)&c->_internal_ - (void *)c;
    *dest = realloc(*dest, size + len);
    memcpy(*dest + size, c, len);
    size += len;
    if (!skip) {
        size = _v3_msg_string_put(v3h, c->name, sizeof(c->name) - 1, dest, size);
        size = _v3_msg_string_put(v3h, c->phonetic, sizeof(c->phonetic) - 1, dest, size);
        size = _v3_msg_string_put(v3h, c->comment, sizeof(c->comment) - 1, dest, size);
    }

    _v3_leave(v3h, func);
    return size;
}

const void *
_v3_msg_rank_get(v3_handle v3h, const void *src, v3_rank *r) {
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

const void *
_v3_msg_user_get(v3_handle v3h, const void *src, v3_user *u) {
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

size_t
_v3_msg_user_put(v3_handle v3h, const v3_user *u, void **dest, size_t size) {
    const char func[] = "_v3_msg_user_put";

    int len;

    _v3_enter(v3h, func);

    len = (void *)&u->_internal_ - (void *)u;
    *dest = realloc(*dest, size + len);
    memcpy(*dest + size, u, len);
    size += len;
    size = _v3_msg_string_put(v3h, u->name, sizeof(u->name) - 1, dest, size);
    size = _v3_msg_string_put(v3h, u->phonetic, sizeof(u->phonetic) - 1, dest, size);
    size = _v3_msg_string_put(v3h, u->comment, sizeof(u->comment) - 1, dest, size);
    size = _v3_msg_string_put(v3h, u->integration, sizeof(u->integration) - 1, dest, size);
    size = _v3_msg_string_put(v3h, u->url, sizeof(u->url) - 1, dest, size);

    _v3_leave(v3h, func);
    return size;
}

int
_v3_msg_handshake_put(v3_handle v3h) {
    const char func[] = "_v3_msg_handshake_put";

    _v3_message *m;
    _v3_msg_handshake *mc;
    size_t ctr;
    int ret;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_HANDSHAKE, sizeof(_v3_msg_handshake), (void **)&mc);

    strncpy(mc->protocol, V3_PROTO_VERSION, sizeof(mc->protocol) - 1);
    for (ctr = 0; ctr < sizeof(mc->salt_1) - 1; ctr++) {
        mc->salt_1[ctr] = rand() % 93 + 33;
        mc->salt_2[ctr] = rand() % 93 + 33;
    }

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_msg_chat_put(v3_handle v3h, uint16_t subtype, const char *message) {
    const char func[] = "_v3_msg_chat_put";

    _v3_message *m;
    _v3_msg_chat *mc;
    int ret;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_CHAT, sizeof(_v3_msg_chat), (void **)&mc);

    switch ((mc->subtype = subtype)) {
      case V3_CHAT_MESSAGE:
      case V3_CHAT_RCON:
        m->len = _v3_msg_string_put(v3h, message, 0xff, &m->data, m->len);
        break;
    }

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_msg_user_option_put(v3_handle v3h, uint16_t user, uint16_t subtype, uint32_t value) {
    const char func[] = "_v3_msg_user_option_put";

    _v3_message *m;
    _v3_msg_user_option *mc;
    int ret;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_USER_OPTION, sizeof(_v3_msg_user_option), (void **)&mc);

    mc->user = user;
    mc->subtype = subtype;
    mc->value = value;

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_msg_login_put(v3_handle v3h) {
    const char func[] = "_v3_msg_login_put";

    _v3_connection *v3c;
    _v3_message *m;
    _v3_msg_login *mc;
    int ret;

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

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_msg_chan_list_put(v3_handle v3h, uint16_t subtype, uint16_t user, const char *password, const v3_channel *c) {
    const char func[] = "_v3_msg_chan_list_put";

    _v3_message *m;
    _v3_msg_chan_list *mc;
    int ret;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_CHAN_LIST, sizeof(_v3_msg_chan_list), (void **)&mc);

    switch ((mc->subtype = subtype)) {
      case V3_CHAN_ADD:
        break;
      case V3_CHAN_REMOVE:
        break;
      case V3_CHAN_CHANGE:
        mc->user = user;
        if (password && *password) {
            _v3_password(v3h, password, mc->password);
        }
        m->len = _v3_msg_channel_put(v3h, c, &m->data, m->len, true);
        break;
      case V3_CHAN_UPDATE:
        break;
    }

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_msg_timestamp_put(v3_handle v3h) {
    const char func[] = "_v3_msg_timestamp_put";

    _v3_message *m;
    _v3_msg_timestamp *mc;
    int ret;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_TIMESTAMP, sizeof(_v3_msg_timestamp), (void **)&mc);

    mc->timestamp = time(NULL);

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_msg_hash_table_put(v3_handle v3h, uint16_t subtype) {
    const char func[] = "_v3_msg_hash_table_put";

    _v3_message *m;
    _v3_msg_hash_table *mc;
    int ret;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_HASH_TABLE, sizeof(_v3_msg_hash_table), (void **)&mc);

    switch ((mc->subtype = subtype)) {
      case 0x00:
      case 0x02:
      case 0x04:
      case 0x06:
      case 0x08:
      case 0x09:
        mc->sum_2 = _v3_hash_table_sum(v3h, subtype);
        break;
      case 0x01:
        mc->sum_1 = rand() & 0xff;
        break;
      case 0x03:
      case 0x05:
      case 0x07:
        mc->sum_2 = _v3_hash_table_scramble(v3h, subtype, 0);
        break;
      case 0x0a:
        mc->sum_2 = ~(rand() & 0xffff);
        break;
    }

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_msg_user_list_put(v3_handle v3h, uint16_t subtype, const v3_user *u) {
    const char func[] = "_v3_msg_user_list_put";

    _v3_message *m;
    _v3_msg_user_list *mc;
    int ret;

    _v3_enter(v3h, func);

    m = _v3_msg_alloc(v3h, V3_MSG_USER_LIST, sizeof(_v3_msg_user_list), (void **)&mc);

    mc->subtype = subtype;
    mc->count = 1;
    m->len = _v3_msg_user_put(v3h, u, &m->data, m->len);

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_msg_process(v3_handle v3h, _v3_message *m) {
    const char func[] = "_v3_msg_process";

    _v3_connection *v3c;
    int ret = V3_OK;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    _v3_debug(v3h, V3_DBG_MESSAGE, "processing message type 0x%02x (%u)", m->type, m->type);

    switch (m->type) {
      case V3_MSG_AUTH:
        {
            _v3_msg_auth *mc = m->data;

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
                        &mc->key,
                        m->len - ((void *)&mc->key - m->data)) != V3_OK) {
                    _v3_error(v3h, "failed to retrieve encryption keys");
                    ret = V3_FAILURE;
                    break;
                }
                if (!v3c->logged_in) {
                    ret = _v3_msg_login_put(v3h);
                }
                break;
              case V3_AUTH_INVALID:
                break;
              case V3_AUTH_DISABLED:
                break;
              default:
                _v3_debug(v3h, V3_DBG_MESSAGE, "message type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                ret = V3_NOTIMPL;
                break;
            }
        }
        break;
      case V3_MSG_CHAN_ADMIN:
        {
            /* _v3_msg_chan_admin *mc = m->data; */
            v3_event ev = { .type = V3_EVENT_CHAN_ADMIN };

            _v3_mutex_lock(v3h);

            _v3_msg_uint16_get(
                    v3h,
                    m->data + sizeof(_v3_msg_chan_admin),
                    v3c->lacct.chan_admin,
                    sizeof(v3c->lacct.chan_admin) / sizeof(uint16_t),
                    &v3c->lacct.chan_admin_count);
            ev.account = v3c->lacct;
            _v3_event_push(v3h, &ev);

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_SCRAMBLE:
        {
            int type[] = {
                V3_USER_ACCEPT_PAGES,
                V3_USER_ACCEPT_U2U,
                V3_USER_ACCEPT_CHAT,
                V3_USER_ALLOW_RECORD
            };
            uint16_t value[] = {
                v3c->luser.accept_pages,
                v3c->luser.accept_u2u,
                v3c->luser.accept_chat,
                v3c->luser.allow_record
            };
            size_t ctr;
            v3_user u;
            v3_event ev = { .type = V3_EVENT_LOGIN };

            _v3_debug(v3h, V3_DBG_MESSAGE, "scrambling client key");
            _v3_key_scramble(v3h, v3c->client_key, v3c->handshake_key);
            _v3_debug(v3h, V3_DBG_MESSAGE, "scrambling server key");
            _v3_key_scramble(v3h, v3c->server_key, v3c->handshake_key);
            if (!v3c->logged_in) {
                ret = _v3_msg_hash_table_put(v3h, 0);
                for (ctr = 0; ctr < sizeof(type) / sizeof(int); ctr++) {
                    if (value[ctr]) {
                        ret = _v3_msg_user_option_put(v3h, v3c->luser.id, type[ctr], true);
                    }
                }
                if (*v3c->luser.comment || *v3c->luser.url || *v3c->luser.integration) {
                    memset(&u, 0, sizeof(v3_user));
                    u.id = v3c->luser.id;
                    u.flags = 1 << 8;
                    _v3_strncpy(u.comment, v3c->luser.comment, sizeof(u.comment) - 1);
                    _v3_strncpy(u.url, v3c->luser.url, sizeof(u.url) - 1);
                    _v3_strncpy(u.integration, v3c->luser.integration, sizeof(u.integration) - 1);
                    ret = _v3_msg_user_list_put(v3h, V3_USER_UPDATE, &u);
                }
                _v3_timestamp(v3h, NULL);
                v3c->logged_in = true;
                _v3_event_push(v3h, &ev);
            }
        }
        break;
      case V3_MSG_RANK_LIST:
        {
            _v3_msg_rank_list *mc = m->data;
            const void *ptr;
            uint16_t ctr;
            v3_rank r;
            v3_event ev = { .type = 0 };

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
                    if (!ev.type) {
                        ev.type = V3_EVENT_RANK_LIST;
                    }
                  case V3_RANK_ADD:
                    if (!ev.type) {
                        ev.type = V3_EVENT_RANK_ADD;
                    }
                  case V3_RANK_UPDATE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_RANK_UPDATE;
                    }
                    _v3_data(v3h, V3_DATA_UPDATE, V3_DATA_TYPE_RANK, &r);
                    ev.rank = r;
                    ev.rank.next = NULL;
                    break;
                  default:
                    _v3_debug(v3h, V3_DBG_MESSAGE, "message type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ret = V3_NOTIMPL;
                    ctr = mc->count;
                    break;
                }
                if (ev.type) {
                    _v3_event_push(v3h, &ev);
                }
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_PING:
        {
            _v3_msg_ping *mc = m->data;
            v3_event ev = { .type = V3_EVENT_PING };

            _v3_debug(v3h, V3_DBG_MESSAGE, "ping: user: %u | sequence: %u | ping: %u | inactive: %u",
                    mc->user,
                    mc->sequence,
                    mc->ping,
                    mc->inactive);
            ret = _v3_send(v3h, m);
            ev.ping = mc->ping;
            _v3_event_push(v3h, &ev);
        }
        break;
      case V3_MSG_CHAN_MOVE:
        {
            _v3_msg_chan_move *mc = m->data;
            v3_user u = { .id = mc->id };
            v3_channel c = { .id = mc->id };
            v3_event ev = { .type = 0 };

            _v3_mutex_lock(v3h);

            _v3_debug(v3h, V3_DBG_MESSAGE, "channel move: id: %u | channel: %u | error: %u", mc->id, mc->channel, mc->error);
            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u) == V3_OK) {
                ev.type = V3_EVENT_CHAN_CHANGE;
                ev.user = u;
                ev.user.next = NULL;
                u.next->channel = mc->channel;
                ev.user.channel = mc->channel;
            } else if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_CHANNEL, &c) == V3_OK) {
                ev.type = V3_EVENT_CHAN_MOVE;
            }
            if (ev.type) {
                _v3_event_push(v3h, &ev);
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_SRV_CODEC:
        {
            _v3_msg_srv_codec *mc = m->data;

            _v3_debug(v3h, V3_DBG_MESSAGE, "server codec: index: %u | format: %u", mc->index, mc->format);
            v3c->codec_index = mc->index;
            v3c->codec_format = mc->format;
        }
        break;
      case V3_MSG_CHAT:
        {
            _v3_msg_chat *mc = m->data;
            v3_user u = { .id = mc->user };
            v3_event ev = { .type = 0 };

            _v3_debug(v3h, V3_DBG_MESSAGE, "chat: user: %u | subtype: %u", mc->user, mc->subtype);
            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u) == V3_OK) {
                ev.user = u;
                ev.user.next = NULL;
                switch (mc->subtype) {
                  case V3_CHAT_JOIN:
                    if (!ev.type) {
                        ev.type = V3_EVENT_CHAT_JOIN;
                    }
                  case V3_CHAT_LEAVE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_CHAT_LEAVE;
                    }
                    break;
                  case V3_CHAT_MESSAGE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_CHAT_MESSAGE;
                    }
                  case V3_CHAT_RCON:
                    if (!ev.type) {
                        ev.type = V3_EVENT_CHAT_RCON;
                    }
                    _v3_msg_string_get(v3h, m->data + sizeof(_v3_msg_chat), ev.data.message, sizeof(ev.data.message) - 1);
                    _v3_debug(v3h, V3_DBG_MESSAGE, "message: '%s'", ev.data.message);
                    break;
                  case V3_CHAT_PERM:
                    if (!ev.type) {
                    
                    }
                    break;
                  default:
                    _v3_debug(v3h, V3_DBG_MESSAGE, "message type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ret = V3_NOTIMPL;
                    break;
                }
                if (ev.type) {
                    _v3_event_push(v3h, &ev);
                }
            }
        }
        break;
      case V3_MSG_USER_OPTION:
        {
            _v3_msg_user_option *mc = m->data;
            v3_user u = { .id = mc->user };
            v3_event ev = { .type = V3_EVENT_USER_MUTE };

            _v3_mutex_lock(v3h);

            _v3_debug(v3h, V3_DBG_MESSAGE, "user option: user: %u | subtype: %u | value: %u", mc->user, mc->subtype, mc->value);
            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u) == V3_OK) {
                ev.user = u;
                ev.user.next = NULL;
                switch (mc->subtype) {
                  case V3_USER_ACCEPT_PAGES:
                    u.next->accept_pages = mc->value;
                    break;
                  case V3_USER_ACCEPT_U2U:
                    u.next->accept_u2u = mc->value;
                    break;
                  case V3_USER_ALLOW_RECORD:
                    u.next->allow_record = mc->value;
                    break;
                  case V3_USER_ACCEPT_CHAT:
                    u.next->accept_chat = mc->value;
                    break;
                  case V3_USER_GLOBAL_MUTE:
                    u.next->muted_global = mc->value;
                    ev.user.muted_global = mc->value;
                    _v3_event_push(v3h, &ev);
                    break;
                  case V3_USER_CHANNEL_MUTE:
                    u.next->muted_channel = mc->value;
                    ev.user.muted_channel = mc->value;
                    _v3_event_push(v3h, &ev);
                    break;
                  default:
                    _v3_debug(v3h, V3_DBG_MESSAGE, "message type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ret = V3_NOTIMPL;
                    break;
                }
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_CHAN_LIST:
        {
            _v3_msg_chan_list *mc = m->data;
            v3_channel c;
            v3_event ev = { .type = 0 };

            _v3_mutex_lock(v3h);

            _v3_msg_channel_get(v3h, m->data + sizeof(_v3_msg_chan_list), &c);
            _v3_debug(v3h, V3_DBG_MESSAGE, "channel: id: %u | name: '%s' | phonetic: '%s' | comment: '%s'",
                        c.id,
                        c.name,
                        c.phonetic,
                        c.comment);
            switch (mc->subtype) {
              case V3_CHAN_ADD:
                if (!ev.type) {
                    ev.type = V3_EVENT_CHAN_ADD;
                }
              case V3_CHAN_REMOVE:
                if (!ev.type) {
                    ev.type = V3_EVENT_CHAN_REMOVE;
                }
              case V3_CHAN_UPDATE:
                if (!ev.type) {
                    ev.type = V3_EVENT_CHAN_UPDATE;
                }
                _v3_data(v3h, (mc->subtype == V3_CHAN_REMOVE) ? V3_DATA_REMOVE : V3_DATA_UPDATE, V3_DATA_TYPE_CHANNEL, &c);
              case V3_CHAN_AUTH:
                if (!ev.type) {
                    ev.type = V3_EVENT_CHAN_AUTH;
                }
                ev.channel = c;
                ev.channel.next = NULL;
                break;
              default:
                _v3_debug(v3h, V3_DBG_MESSAGE, "message type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                ret = V3_NOTIMPL;
                break;
            }
            if (ev.type) {
                _v3_event_push(v3h, &ev);
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_ACCT_LIST:
        {
            _v3_msg_acct_list *mc = m->data;
            const void *ptr;
            uint16_t ctr;
            //v3_account a;
            v3_event ev = { .type = 0 };

            _v3_mutex_lock(v3h);

            for (ptr = m->data + sizeof(_v3_msg_acct_list), ctr = 0; ctr < mc->count; ctr++) {
                switch (mc->subtype) {
                  case V3_ACCT_OPEN:
                    if (!ev.type) {
                        ev.type = V3_EVENT_ACCT_OPEN;
                    }
                  case V3_ACCT_ADD:
                    if (!ev.type) {
                        ev.type = V3_EVENT_ACCT_ADD;
                    }
                  case V3_ACCT_UPDATE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_ACCT_UPDATE;
                    }
                    break;
                  case V3_ACCT_LOCAL:
                    if (!ev.type) {
                        ev.type = V3_EVENT_ACCT_LOCAL;
                    }
                    memcpy(&v3c->lacct, ptr, (void *)&v3c->lacct._internal_ - (void *)&v3c->lacct);
                    ev.account = v3c->lacct;
                    ctr = mc->count;
                    break;
                  default:
                    _v3_debug(v3h, V3_DBG_MESSAGE, "message type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ret = V3_NOTIMPL;
                    ctr = mc->count;
                    break;
                }
                if (ev.type) {
                    _v3_event_push(v3h, &ev);
                }
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_SRV_INFO:
        {
            _v3_msg_srv_info *mc = m->data;

            _v3_debug(v3h, V3_DBG_MESSAGE, "server info: licensed: %u | port: %u | slots: %u | clients: %u | name: '%s' | version: '%s'",
                    mc->licensed,
                    mc->port,
                    mc->slots,
                    mc->clients,
                    mc->name,
                    mc->version);
            v3c->licensed = mc->licensed;
            v3c->slots = mc->slots;
            v3c->clients = mc->clients;
            strncpy(v3c->name, mc->name, sizeof(v3c->name) - 1);
            strncpy(v3c->version, mc->version, sizeof(v3c->version) - 1);
        }
        break;
      case V3_MSG_SRV_PROP:
        {
            _v3_msg_srv_prop *mc = m->data;
            v3_prop *prop = &v3c->prop;
            void *values[] = {
                [0x02] = &prop->chat_filter,
                [0x03] = &prop->chan_order,
                [0x05] = &prop->motd_always,
                [0x0b] = &prop->inactive_timeout,
                [0x0c] = &prop->inactive_action,
                [0x0d] = &prop->inactive_channel,
                [0x0e] = &prop->rem_srv_comment,
                [0x0f] = &prop->rem_chan_name,
                [0x10] = &prop->rem_chan_comment,
                [0x11] = &prop->rem_user_name,
                [0x12] = &prop->rem_user_comment,
                [0x13] = &prop->srv_comment,
                [0x14] = &prop->wav_disable,
                [0x15] = &prop->tts_disable,
                [0x18] = &prop->rem_login,
                [0x19] = &prop->max_guest,
                [0x1a] = &prop->auto_kick,
                [0x1b] = &prop->auto_ban,
                [0x1c] = NULL
            };
            v3_filter *filters[] = {
                [0x07] = &prop->chat_spam,
                [0x08] = &prop->comment_spam,
                [0x09] = &prop->wav_spam,
                [0x0a] = &prop->tts_spam,
                [0x16] = &prop->chan_spam,
                [0x1c] = NULL
            };
            v3_filter *filter;
            const size_t count = sizeof(values) / sizeof(void *);
            char value[0x100] = "";
            v3_event ev = { .type = 0 };

            switch (mc->subtype) {
              case V3_PROP_RECV:
              case V3_PROP_SEND:
                if (mc->transaction != v3c->transaction) {
                    break;
                }
              case V3_PROP_CLIENT:
                if (mc->property >= count) {
                    break;
                }
                if (mc->subtype != V3_PROP_SEND) {
                    if (!mc->ignore) {
                        _v3_msg_string_get(v3h, m->data + sizeof(_v3_msg_srv_prop), value, sizeof(value) - 1);
                        if (values[mc->property] == &prop->inactive_channel) {
                            prop->inactive_channel = v3_channel_id(v3h, value);
                        } else if (values[mc->property] == &prop->srv_comment) {
                            strncpy(prop->srv_comment, value, sizeof(prop->srv_comment) - 1);
                        } else if (values[mc->property]) {
                            *(uint16_t *)values[mc->property] = atoi(value);
                        } else if ((filter = filters[mc->property])) {
                            sscanf(value, "%hu,%hu,%hu", &filter->action, &filter->interval, &filter->times);
                        }
                    }
                    if (mc->subtype == V3_PROP_CLIENT) {
                        if (values[mc->property] == &prop->chat_filter) {
                            ev.type = V3_EVENT_PROP_CHAT_FILTER;
                        } else if (values[mc->property] == &prop->chan_order) {
                            ev.type = V3_EVENT_PROP_CHAN_ORDER;
                        } else if (values[mc->property] == &prop->motd_always) {
                            ev.type = V3_EVENT_PROP_MOTD_ALWAYS;
                        }
                        if (ev.type) {
                            ev.data.prop = v3c->prop;
                            _v3_event_push(v3h, &ev);
                        }
                    }
                    mc->property++;
                } else {
                
                }
                break;
            }
        }
        break;
      case V3_MSG_AUDIO:
        {
            _v3_msg_audio *mc = m->data;
            v3_user u = { .id = mc->user };
            v3_event ev = { .type = 0 };

            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u) == V3_OK) {
                ev.user = u;
                ev.user.next = NULL;
                switch (mc->subtype) {
                  case V3_AUDIO_START:
                  case V3_AUDIO_LOGIN:
                    if (!ev.type) {
                        ev.type = V3_EVENT_AUDIO_START;
                    }
                  case V3_AUDIO_STOP:
                    if (!ev.type) {
                        ev.type = V3_EVENT_AUDIO_STOP;
                    }
                    u.next->transmitting = (mc->subtype != V3_AUDIO_STOP);
                    ev.user.transmitting = (mc->subtype != V3_AUDIO_STOP);
                  case V3_AUDIO_MUTE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_AUDIO_MUTE;
                    }
                  case V3_AUDIO_DATA:
                    if (ev.type || u.muted_local) {
                        _v3_decoder_destroy(v3h, &u.next->decoder);
                        break;
                    }
                    ev.data.pcm.length = sizeof(ev.data.pcm.sample);
                    ev.data.pcm.channels = (mc->pcmlen - 2000 == 2) ? 2 : 1;
                    if (_v3_audio_decode(
                            v3h,
                            mc->index,
                            mc->format,
                            &u.next->decoder,
                            m->data + sizeof(_v3_msg_audio),
                            m->len - sizeof(_v3_msg_audio),
                            ev.data.pcm.sample,
                            &ev.data.pcm.length,
                            &ev.data.pcm.rate,
                            ev.data.pcm.channels) == V3_OK) {
                        ev.type = V3_EVENT_AUDIO_RECV;
                    }
                    break;
                  case V3_AUDIO_AVAIL:
                  case V3_AUDIO_TAKEN:
                    break;
                  default:
                    _v3_debug(v3h, V3_DBG_MESSAGE, "message type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ret = V3_NOTIMPL;
                    break;
                }
                if (ev.type) {
                    _v3_event_push(v3h, &ev);
                }
            }
        }
        break;
      case V3_MSG_CHAN_CHANGE:
        {
            _v3_msg_chan_change *mc = m->data;
            v3_user u = { .id = mc->user };
            v3_event ev = { .type = V3_EVENT_CHAN_CHANGE };

            _v3_mutex_lock(v3h);

            _v3_debug(v3h, V3_DBG_MESSAGE, "channel change: user: %u | channel: %u", mc->user, mc->channel);
            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u) == V3_OK) {
                ev.user = u;
                ev.user.next = NULL;
                u.next->channel = mc->channel;
                ev.user.channel = mc->channel;
                _v3_event_push(v3h, &ev);
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_HASH_TABLE:
        {
            _v3_msg_hash_table *mc = m->data;

            if (v3c->logged_in) {
                ret = _v3_msg_hash_table_put(v3h, mc->subtype);
            }
        }
        break;
      case V3_MSG_USER_LIST:
        {
            _v3_msg_user_list *mc = m->data;
            const void *ptr;
            uint16_t ctr;
            v3_user u;
            v3_event ev = { .type = 0 };

            _v3_mutex_lock(v3h);

            for (ptr = m->data + sizeof(_v3_msg_user_list), ctr = 0; ctr < mc->count; ctr++) {
                memset(&u, 0, sizeof(v3_user));
                ptr = _v3_msg_user_get(v3h, ptr, &u);
                _v3_debug(v3h, V3_DBG_MESSAGE, "user: id: %u | channel: %u | flags: %u | rank: %u",
                        u.id,
                        u.channel,
                        u.flags,
                        u.rank);
                _v3_debug(v3h, V3_DBG_MESSAGE, "name: '%s' | phonetic: '%s' | comment: '%s' | integration: '%s' | url: '%s'",
                        u.name,
                        u.phonetic,
                        u.comment,
                        u.integration,
                        u.url);
                switch (mc->subtype) {
                  case V3_USER_REMOVE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_USER_LOGOUT;
                    }
                  case V3_USER_ADD:
                    if (!ev.type) {
                        ev.type = V3_EVENT_USER_LOGIN;
                    }
                  case V3_USER_UPDATE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_USER_UPDATE;
                    }
                  case V3_USER_LIST:
                    if (!ev.type) {
                        ev.type = V3_EVENT_USER_LIST;
                    }
                  case V3_USER_RANK:
                    if (!ev.type) {
                        ev.type = V3_EVENT_USER_RANK;
                    }
                    _v3_data(v3h, (mc->subtype == V3_USER_REMOVE) ? V3_DATA_REMOVE : V3_DATA_UPDATE, V3_DATA_TYPE_USER, &u);
                    if ((!v3c->luser.id && ctr == 1) || (v3c->luser.id && u.id == v3c->luser.id)) {
                        memcpy(&v3c->luser, &u, (void *)&u._internal_ - (void *)&u);
                    }
                    ev.user = u;
                    ev.user.next = NULL;
                    break;
                  default:
                    _v3_debug(v3h, V3_DBG_MESSAGE, "message type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ret = V3_NOTIMPL;
                    ctr = mc->count;
                    break;
                }
                if (ev.type) {
                    _v3_event_push(v3h, &ev);
                }
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_LIST_CHAN:
        {
            _v3_msg_list_chan *mc = m->data;
            const void *ptr;
            uint16_t ctr;
            v3_channel c;
            v3_event ev = { .type = V3_EVENT_CHAN_LIST };

            _v3_mutex_lock(v3h);

            for (ptr = m->data + sizeof(_v3_msg_list_chan), ctr = 0; ctr < mc->count; ctr++) {
                memset(&c, 0, sizeof(v3_channel));
                ptr = _v3_msg_channel_get(v3h, ptr, &c);
                _v3_debug(v3h, V3_DBG_MESSAGE, "channel: id: %u | name: '%s' | phonetic: '%s' | comment: '%s'",
                        c.id,
                        c.name,
                        c.phonetic,
                        c.comment);
                _v3_data(v3h, V3_DATA_UPDATE, V3_DATA_TYPE_CHANNEL, &c);
                ev.channel = c;
                ev.channel.next = NULL;
                _v3_event_push(v3h, &ev);
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      default:
        _v3_debug(v3h, V3_DBG_MESSAGE, "unknown message type 0x%02x", m->type);
        ret = V3_NOTIMPL;
        break;
    }
    switch (ret) {
      case V3_OK:
        _v3_debug(v3h, V3_DBG_MESSAGE, "message processed");
        break;
      case V3_MALFORM:
        _v3_debug(v3h, V3_DBG_MESSAGE, "malformed message");
        break;
      case V3_NOTIMPL:
        _v3_debug(v3h, V3_DBG_MESSAGE, "message not implemented");
        break;
      case V3_FAILURE:
        _v3_debug(v3h, V3_DBG_MESSAGE, "failed to process message");
        break;
    }

    _v3_leave(v3h, func);
    return ret;
}

