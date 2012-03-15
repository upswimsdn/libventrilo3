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

void *
_v3_msg_alloc(v3_handle v3h, uint32_t type, size_t len, void **contents) {
    _v3_message *m;

    _v3_enter(v3h, __func__);

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

    _v3_leave(v3h, __func__);
    return m;
}

void
_v3_msg_free(v3_handle v3h, _v3_message *m) {
    _v3_enter(v3h, __func__);

    if (!m) {
        _v3_leave(v3h, __func__);
        return;
    }
    if (m->data) {
        _v3_debug(v3h, V3_DBG_MEMORY, "releasing data");
        free(m->data);
    }
    _v3_debug(v3h, V3_DBG_MEMORY, "releasing message");
    free(m);

    _v3_leave(v3h, __func__);
}

const void *
_v3_msg_string_get(v3_handle v3h, const void *src, char *dest, size_t n) {
    uint16_t len;

    _v3_enter(v3h, __func__);

    len = ntohs(*(uint16_t *)src);
    src += sizeof(len);
    memcpy(dest, src, (len > n) ? n : len);
    if (len < n) {
        dest[len] = 0;
    }
    src += len;

    _v3_leave(v3h, __func__);
    return src;
}

size_t
_v3_msg_string_put(v3_handle v3h, const char *src, size_t n, void **dest, size_t size) {
    uint16_t len;

    _v3_enter(v3h, __func__);

    len = strlen(src);
    len = (len > n) ? n : len;
    *dest = realloc(*dest, size + sizeof(len) + len);
    *(uint16_t *)(*dest + size) = htons(len);
    size += sizeof(len);
    memcpy(*dest + size, src, len);
    size += len;

    _v3_leave(v3h, __func__);
    return size;
}

const void *
_v3_msg_uint16_get(v3_handle v3h, const void *src, uint16_t *dest, size_t n, uint16_t *count) {
    uint16_t len;
    size_t ctr;

    _v3_enter(v3h, __func__);

    len = ntohs(*(uint16_t *)src);
    if (count) {
        *count = (len > n) ? n : len;
    }
    src += sizeof(len);
    for (ctr = 0; ctr < ((len > n) ? n : len); ++ctr) {
        dest[ctr] = ntohs(*((uint16_t *)src + ctr));
    }
    src += len * sizeof(*dest);

    _v3_leave(v3h, __func__);
    return src;
}

size_t
_v3_msg_uint16_put(v3_handle v3h, const uint16_t *src, uint16_t count, void **dest, size_t size) {
    uint16_t ctr;

    _v3_enter(v3h, __func__);

    *dest = realloc(*dest, size + sizeof(count) + sizeof(*src) * count);
    *(uint16_t *)(*dest + size) = htons(count);
    size += sizeof(count);
    for (ctr = 0; ctr < count; ++ctr) {
        *(uint16_t *)(*dest + size) = htons(src[ctr]);
        size += sizeof(*src);
    }

    _v3_leave(v3h, __func__);
    return size;
}

const void *
_v3_msg_channel_get(v3_handle v3h, const void *src, v3_channel *c) {
    int len;

    _v3_enter(v3h, __func__);

    len = (void *)&c->_internal_ - (void *)c;
    memcpy(c, src, len);
    src += len;
    src = _v3_msg_string_get(v3h, src, c->name, sizeof(c->name) - 1);
    src = _v3_msg_string_get(v3h, src, c->phonetic, sizeof(c->phonetic) - 1);
    src = _v3_msg_string_get(v3h, src, c->comment, sizeof(c->comment) - 1);

    _v3_leave(v3h, __func__);
    return src;
}

size_t
_v3_msg_channel_put(v3_handle v3h, const v3_channel *c, void **dest, size_t size, int skip) {
    int len;

    _v3_enter(v3h, __func__);

    len = (void *)&c->_internal_ - (void *)c;
    *dest = realloc(*dest, size + len);
    memcpy(*dest + size, c, len);
    size += len;
    if (!skip) {
        size = _v3_msg_string_put(v3h, c->name, sizeof(c->name) - 1, dest, size);
        size = _v3_msg_string_put(v3h, c->phonetic, sizeof(c->phonetic) - 1, dest, size);
        size = _v3_msg_string_put(v3h, c->comment, sizeof(c->comment) - 1, dest, size);
    }

    _v3_leave(v3h, __func__);
    return size;
}

const void *
_v3_msg_rank_get(v3_handle v3h, const void *src, v3_rank *r) {
    int len;

    _v3_enter(v3h, __func__);

    len = (void *)&r->_internal_ - (void *)r;
    memcpy(r, src, len);
    src += len;
    src = _v3_msg_string_get(v3h, src, r->name, sizeof(r->name) - 1);
    src = _v3_msg_string_get(v3h, src, r->description, sizeof(r->description) - 1);

    _v3_leave(v3h, __func__);
    return src;
}

const void *
_v3_msg_user_get(v3_handle v3h, const void *src, v3_user *u) {
    int len;

    _v3_enter(v3h, __func__);

    len = (void *)&u->_internal_ - (void *)u;
    memcpy(u, src, len);
    src += len;
    src = _v3_msg_string_get(v3h, src, u->name, sizeof(u->name) - 1);
    src = _v3_msg_string_get(v3h, src, u->phonetic, sizeof(u->phonetic) - 1);
    src = _v3_msg_string_get(v3h, src, u->comment, sizeof(u->comment) - 1);
    src = _v3_msg_string_get(v3h, src, u->integration, sizeof(u->integration) - 1);
    src = _v3_msg_string_get(v3h, src, u->url, sizeof(u->url) - 1);

    _v3_leave(v3h, __func__);
    return src;
}

size_t
_v3_msg_user_put(v3_handle v3h, const v3_user *u, void **dest, size_t size) {
    int len;

    _v3_enter(v3h, __func__);

    len = (void *)&u->_internal_ - (void *)u;
    *dest = realloc(*dest, size + len);
    memcpy(*dest + size, u, len);
    size += len;
    size = _v3_msg_string_put(v3h, u->name, sizeof(u->name) - 1, dest, size);
    size = _v3_msg_string_put(v3h, u->phonetic, sizeof(u->phonetic) - 1, dest, size);
    size = _v3_msg_string_put(v3h, u->comment, sizeof(u->comment) - 1, dest, size);
    size = _v3_msg_string_put(v3h, u->integration, sizeof(u->integration) - 1, dest, size);
    size = _v3_msg_string_put(v3h, u->url, sizeof(u->url) - 1, dest, size);

    _v3_leave(v3h, __func__);
    return size;
}

int
_v3_msg_handshake_put(v3_handle v3h) {
    _v3_message *m;
    _v3_msg_handshake *mc;
    size_t ctr;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_HANDSHAKE, sizeof(_v3_msg_handshake), (void **)&mc);

    strncpy(mc->protocol, V3_PROTOCOL_VERSION, sizeof(mc->protocol) - 1);
    for (ctr = 0; ctr < sizeof(mc->salt_1) - 1; ++ctr) {
        mc->salt_1[ctr] = rand() % 93 + 33;
        mc->salt_2[ctr] = rand() % 93 + 33;
    }

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_move_put(v3_handle v3h, uint16_t id, uint16_t channel) {
    _v3_message *m;
    _v3_msg_move *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_MOVE, sizeof(_v3_msg_move), (void **)&mc);

    mc->id = id;
    mc->channel = channel;

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_chat_put(v3_handle v3h, uint16_t subtype, const char *message) {
    _v3_message *m;
    _v3_msg_chat *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_CHAT, sizeof(_v3_msg_chat), (void **)&mc);

    switch ((mc->subtype = subtype)) {
      case V3_CHAT_MESSAGE:
      case V3_CHAT_RCON:
        m->len = _v3_msg_string_put(v3h, message, 0xff, &m->data, m->len);
        break;
    }

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_user_option_put(v3_handle v3h, uint16_t user, uint16_t subtype, uint32_t value) {
    _v3_message *m;
    _v3_msg_user_option *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_USER_OPTION, sizeof(_v3_msg_user_option), (void **)&mc);

    mc->user = user;
    mc->subtype = subtype;
    mc->value = value;

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_login_put(v3_handle v3h) {
    _v3_connection *v3c;
    _v3_message *m;
    _v3_msg_login *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_LOGIN, sizeof(_v3_msg_login), (void **)&mc);

    v3c = _v3_handles[v3h];
    mc->subtype = 2;
    mc->ip = v3c->ip;
    mc->port = v3c->port ^ 0xffff;
    mc->remote_status = true;
    mc->handshake_idx = v3c->handshake_idx;
    memcpy(mc->handshake, v3c->handshake, sizeof(mc->handshake));
    strncpy(mc->version, V3_CLIENT_VERSION, sizeof(mc->version) - 1);
    strncpy(mc->protocol, V3_PROTOCOL_VERSION, sizeof(mc->protocol) - 1);
    if (*v3c->password) {
        _v3_password(v3h, v3c->password, mc->password);
    }
    strncpy(mc->username, v3c->luser.name, sizeof(mc->username) - 1);
    strncpy(mc->phonetic, v3c->luser.phonetic, sizeof(mc->phonetic) - 1);
    strncpy(mc->platform, V3_CLIENT_PLATFORM, sizeof(mc->platform) - 1);

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_chan_list_put(v3_handle v3h, uint16_t subtype, uint16_t user, const char *password, const v3_channel *c) {
    _v3_message *m;
    _v3_msg_chan_list *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_CHAN_LIST, sizeof(_v3_msg_chan_list), (void **)&mc);

    switch ((mc->subtype = subtype)) {
      case V3_CHAN_ADD:
        break;
      case V3_CHAN_REMOVE:
        break;
      case V3_CHAN_CHANGE:
      case V3_CHAN_KICK:
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

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_timestamp_put(v3_handle v3h) {
    _v3_message *m;
    _v3_msg_timestamp *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_TIMESTAMP, sizeof(_v3_msg_timestamp), (void **)&mc);

    mc->timestamp = time(NULL);

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_audio_put(v3_handle v3h, uint16_t subtype, int16_t index, int16_t format, uint32_t pcmlen, const void *data, uint32_t datalen) {
    _v3_message *m;
    _v3_msg_audio *mc;
    uint16_t method[] = { V3_METHOD_CURRENT };
    uint16_t dest[] = { 0 };
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_AUDIO, sizeof(_v3_msg_audio), (void **)&mc);

    mc->index = index;
    mc->format = format;
    mc->datalen = datalen;
    mc->pcmlen = pcmlen;
    switch ((mc->subtype = subtype)) {
      case V3_AUDIO_START:
      case V3_AUDIO_DATA:
        m->len = _v3_msg_uint16_put(v3h, method, sizeof(method) / sizeof(*method), &m->data, m->len);
        m->len = _v3_msg_uint16_put(v3h, dest, sizeof(dest) / sizeof(*dest), &m->data, m->len);
        if (subtype == V3_AUDIO_DATA) {
            m->data = realloc(m->data, m->len + datalen);
            memcpy(m->data + m->len, data, datalen);
            m->len += datalen;
        }
        break;
      case V3_AUDIO_STOP:
        m->len = _v3_msg_uint16_put(v3h, NULL, 0, &m->data, m->len);
        m->len = _v3_msg_uint16_put(v3h, NULL, 0, &m->data, m->len);
        break;
    }

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_phantom_put(v3_handle v3h, uint16_t subtype, uint16_t phantom, uint16_t channel) {
    _v3_message *m;
    _v3_msg_phantom *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_PHANTOM, sizeof(_v3_msg_phantom), (void **)&mc);

    switch ((mc->subtype = subtype)) {
      case V3_PHANTOM_ADD:
        mc->channel = channel;
        break;
      case V3_PHANTOM_REMOVE:
        mc->phantom = phantom;
        break;
    }
    mc->user = v3_luser_id(v3h);

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_hash_table_put(v3_handle v3h, uint16_t subtype) {
    _v3_message *m;
    _v3_msg_hash_table *mc;
    int ret;

    _v3_enter(v3h, __func__);

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

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_user_list_put(v3_handle v3h, uint16_t subtype, const v3_user *u) {
    _v3_message *m;
    _v3_msg_user_list *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_USER_LIST, sizeof(_v3_msg_user_list), (void **)&mc);

    mc->subtype = subtype;
    mc->count = 1;
    m->len = _v3_msg_user_put(v3h, u, &m->data, m->len);

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_user_page_put(v3_handle v3h, uint16_t to, uint16_t from) {
    _v3_message *m;
    _v3_msg_user_page *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_USER_PAGE, sizeof(_v3_msg_user_page), (void **)&mc);

    mc->to = to;
    mc->from = from;

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_admin_put(v3_handle v3h, uint16_t subtype, uint16_t user, const void *data) {
    _v3_message *m;
    _v3_msg_admin *mc;
    int ret;

    _v3_enter(v3h, __func__);

    m = _v3_msg_alloc(v3h, V3_MSG_ADMIN, sizeof(_v3_msg_admin), (void **)&mc);

    switch ((mc->subtype = subtype)) {
      case V3_ADMIN_LOGIN:
        _v3_password(v3h, data, mc->data);
        break;
      case V3_ADMIN_LOGOUT:
        break;
      case V3_ADMIN_KICK:
      case V3_ADMIN_BAN:
      case V3_ADMIN_CHAN_BAN:
        mc->user = user;
        _v3_strncpy((void *)mc->data, data, sizeof(mc->data) - 1);
        break;
    }

    ret = _v3_send(v3h, m);

    _v3_msg_free(v3h, m);

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_msg_process(v3_handle v3h, _v3_message *m) {
    _v3_connection *v3c;
    int ret = V3_OK;

    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];
    _v3_debug(v3h, V3_DBG_MESSAGE, "processing message type 0x%02x (%u)", m->type, m->type);

    switch (m->type) {
      case V3_MSG_AUTH:
        {
            _v3_msg_auth *mc = m->data;
            v3_event ev = { .type = 0 };

            switch (mc->subtype) {
              case V3_AUTH_DISCONNECT:
                ev.type = V3_EVENT_LOGOUT;
                break;
              case V3_AUTH_ADMIN:
                ev.type = V3_EVENT_ADMIN_AUTH;
                break;
              case V3_AUTH_KEYS:
                v3c->client_key = calloc(1, sizeof(v3_key_ctx));
                v3c->server_key = calloc(1, sizeof(v3_key_ctx));
                if (_v3_read_keys(
                        v3h,
                        v3c->client_key,
                        v3c->server_key,
                        m->data + sizeof(_v3_msg_auth),
                        m->len - sizeof(_v3_msg_auth)) != V3_OK) {
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
                _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                ret = V3_NOTIMPL;
                break;
            }
            if (ev.type) {
                _v3_event_push(v3h, &ev);
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
                for (ctr = 0; ctr < sizeof(type) / sizeof(*type); ++ctr) {
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

            for (ptr = m->data + sizeof(_v3_msg_rank_list), ctr = 0; ctr < mc->count; ++ctr) {
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
                  case V3_RANK_REMOVE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_RANK_REMOVE;
                    }
                  case V3_RANK_UPDATE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_RANK_UPDATE;
                    }
                    _v3_data(
                            v3h,
                            (mc->subtype == V3_RANK_REMOVE)
                                ? V3_DATA_REMOVE
                                : V3_DATA_UPDATE,
                            V3_DATA_TYPE_RANK,
                            &r,
                            (void *)&r._strings_ - (void *)&r);
                    ev.rank = r;
                    ev.rank.next = NULL;
                    break;
                  default:
                    _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
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
      case V3_MSG_MOVE:
        {
            _v3_msg_move *mc = m->data;
            v3_user u = { .id = mc->id };
            v3_channel c = { .id = mc->id };
            v3_event ev = { .type = 0 };

            _v3_mutex_lock(v3h);

            _v3_debug(v3h, V3_DBG_MESSAGE, "move: id: %u | channel: %u | error: %u", mc->id, mc->channel, mc->error);
            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
                ev.type = V3_EVENT_CHAN_CHANGE;
                ev.user = u;
                ev.user.next = NULL;
                u.next->channel = mc->channel;
                ev.user.channel = mc->channel;
            } else if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_CHANNEL, &c, 0) == V3_OK) {
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
            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
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
                        //TODO
                    }
                    break;
                  default:
                    _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
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
            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
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
                    _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
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
                _v3_data(
                        v3h,
                        (mc->subtype == V3_CHAN_REMOVE)
                            ? V3_DATA_REMOVE
                            : V3_DATA_UPDATE,
                        V3_DATA_TYPE_CHANNEL,
                        &c,
                        (void *)&c._strings_ - (void *)&c);
              case V3_CHAN_AUTH:
                if (!ev.type) {
                    ev.type = V3_EVENT_CHAN_AUTH;
                }
                ev.channel = c;
                ev.channel.next = NULL;
                break;
              default:
                _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
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

            for (ptr = m->data + sizeof(_v3_msg_acct_list), ctr = 0; ctr < mc->count; ++ctr) {
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
                    _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
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
      case V3_MSG_SRV_PROP:
        {
            _v3_msg_srv_prop *mc = m->data;
            void *values[] = {
                [0x02] = &v3c->prop.chat_filter,
                [0x03] = &v3c->prop.chan_order,
                [0x05] = &v3c->prop.motd_always,
                [0x0b] = &v3c->prop.inactive_timeout,
                [0x0c] = &v3c->prop.inactive_action,
                [0x0d] = &v3c->prop.inactive_channel,
                [0x0e] = &v3c->prop.rem_srv_comment,
                [0x0f] = &v3c->prop.rem_chan_name,
                [0x10] = &v3c->prop.rem_chan_comment,
                [0x11] = &v3c->prop.rem_user_name,
                [0x12] = &v3c->prop.rem_user_comment,
                [0x13] = &v3c->prop.srv_comment,
                [0x14] = &v3c->prop.wav_disable,
                [0x15] = &v3c->prop.tts_disable,
                [0x18] = &v3c->prop.rem_login,
                [0x19] = &v3c->prop.max_guest,
                [0x1a] = &v3c->prop.auto_kick,
                [0x1b] = &v3c->prop.auto_ban,
                [0x1c] = NULL
            };
            v3_filter *filters[] = {
                [0x07] = &v3c->prop.chat_spam,
                [0x08] = &v3c->prop.comment_spam,
                [0x09] = &v3c->prop.wav_spam,
                [0x0a] = &v3c->prop.tts_spam,
                [0x16] = &v3c->prop.chan_spam,
                [0x1c] = NULL
            };
            v3_filter *filter;
            char value[0x100] = "";
            v3_event ev = { .type = 0 };

            switch (mc->subtype) {
              case V3_PROP_RECV:
              case V3_PROP_SEND:
                if (mc->transaction != v3c->transaction) {
                    break;
                }
              case V3_PROP_CLIENT:
                if (mc->property >= sizeof(values) / sizeof(*values)) {
                    break;
                }
                if (mc->subtype != V3_PROP_SEND) {
                    if (!mc->ignore) {
                        _v3_msg_string_get(v3h, m->data + sizeof(_v3_msg_srv_prop), value, sizeof(value) - 1);
                        if (values[mc->property] == &v3c->prop.inactive_channel) {
                            v3c->prop.inactive_channel = v3_channel_id(v3h, value);
                        } else if (values[mc->property] == &v3c->prop.srv_comment) {
                            strncpy(v3c->prop.srv_comment, value, sizeof(v3c->prop.srv_comment) - 1);
                        } else if (values[mc->property]) {
                            *(uint16_t *)values[mc->property] = atoi(value);
                        } else if ((filter = filters[mc->property])) {
                            sscanf(value, "%hu,%hu,%hu", &filter->action, &filter->interval, &filter->times);
                        }
                    }
                    if (mc->subtype == V3_PROP_CLIENT) {
                        if (values[mc->property] == &v3c->prop.chat_filter) {
                            ev.type = V3_EVENT_PROP_CHAT_FILTER;
                        } else if (values[mc->property] == &v3c->prop.chan_order) {
                            ev.type = V3_EVENT_PROP_CHAN_ORDER;
                        } else if (values[mc->property] == &v3c->prop.motd_always) {
                            ev.type = V3_EVENT_PROP_MOTD_ALWAYS;
                        }
                        if (ev.type) {
                            ev.data.prop = v3c->prop;
                            _v3_event_push(v3h, &ev);
                        }
                    }
                    ++mc->property;
                } else {
                    //TODO
                }
                break;
            }
        }
        break;
      case V3_MSG_AUDIO:
        {
            _v3_msg_audio *mc = m->data;
            const void *ptr;
            v3_user u = { .id = mc->user };
            float *volume[] = { &u.volume, &v3c->volume, &_volume };
            v3_event ev = { .type = 0 };

            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
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
                        _v3_coder_destroy(v3h, &u.next->decoder);
                        break;
                    }
                    ptr = _v3_msg_uint16_get(v3h, m->data + sizeof(_v3_msg_audio), NULL, 0, NULL);
                    ptr = _v3_msg_uint16_get(v3h, ptr, NULL, 0, NULL);
                    ev.data.pcm.length = sizeof(ev.data.pcm.sample);
                    ev.data.pcm.channels = (mc->pcmlen - 2000 == 2) ? 2 : 1;
                    if (_v3_audio_decode(
                            v3h,
                            mc->index,
                            mc->format,
                            &u.next->decoder,
                            ptr,
                            m->len - (ptr - m->data),
                            ev.data.pcm.sample,
                            &ev.data.pcm.length,
                            &ev.data.pcm.rate,
                            ev.data.pcm.channels) == V3_OK) {
                        _v3_audio_amplify(
                                v3h,
                                (void *)ev.data.pcm.sample,
                                ev.data.pcm.length,
                                volume,
                                sizeof(volume) / sizeof(*volume));
                        ev.type = V3_EVENT_AUDIO_RECV;
                    }
                    break;
                  case V3_AUDIO_AVAIL:
                  case V3_AUDIO_TAKEN:
                    break;
                  default:
                    _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
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
            if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
                ev.user = u;
                ev.user.next = NULL;
                u.next->channel = mc->channel;
                ev.user.channel = mc->channel;
                _v3_event_push(v3h, &ev);
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
            strncpy(v3c->name, mc->name, sizeof(v3c->name) - 1);
            strncpy(v3c->version, mc->version, sizeof(v3c->version) - 1);
        }
        break;
      case V3_MSG_PHANTOM:
        {
            _v3_msg_phantom *mc = m->data;
            v3_user u = { .id = 0 };
            char name[sizeof(u.name)];
            v3_event ev = { .type = 0 };

            _v3_mutex_lock(v3h);

            _v3_debug(v3h, V3_DBG_MESSAGE, "phantom: user: %u | phantom: %u | channel: %u", mc->user, mc->phantom, mc->channel);
            switch (mc->subtype) {
              case V3_PHANTOM_ADD:
                u.id = mc->user;
                break;
              case V3_PHANTOM_REMOVE:
                u.id = mc->phantom;
                break;
              default:
                _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                ret = V3_NOTIMPL;
                break;
            }
            if (_v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
                switch (mc->subtype) {
                  case V3_PHANTOM_ADD:
                    if (!ev.type) {
                        ev.type = (!v3c->logged_in) ? V3_EVENT_USER_LIST : V3_EVENT_USER_LOGIN;
                    }
                    strncpy(name, u.name, sizeof(name) - 1);
                    memset(&u, 0, sizeof(v3_user));
                    strncpy(u.name, name, sizeof(u.name) - 1);
                    u.id = mc->phantom;
                    u.channel = mc->channel;
                    u.phantom_owner = mc->user;
                  case V3_PHANTOM_REMOVE:
                    if (!ev.type) {
                        ev.type = V3_EVENT_USER_LOGOUT;
                    }
                    _v3_data(
                            v3h,
                            (mc->subtype == V3_PHANTOM_REMOVE)
                                ? V3_DATA_REMOVE
                                : V3_DATA_UPDATE,
                            V3_DATA_TYPE_USER,
                            &u,
                            (void *)&u._strings_ - (void *)&u);
                    ev.user = u;
                    ev.user.next = NULL;
                    break;
                }
                if (ev.type) {
                    _v3_event_push(v3h, &ev);
                }
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

            for (ptr = m->data + sizeof(_v3_msg_user_list), ctr = 0; ctr < mc->count; ++ctr) {
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
                    if (mc->subtype == V3_USER_LIST || mc->subtype == V3_USER_ADD) {
                        u.volume = 1.0;
                    }
                    _v3_data(
                            v3h,
                            (mc->subtype == V3_USER_REMOVE)
                                ? V3_DATA_REMOVE
                                : V3_DATA_UPDATE,
                            V3_DATA_TYPE_USER,
                            &u,
                            (void *)&u._strings_ - (void *)&u);
                    if ((!v3c->luser.id && ctr == 1) || (v3c->luser.id && u.id == v3c->luser.id)) {
                        memcpy(&v3c->luser, &u, (void *)&u._internal_ - (void *)&u);
                    }
                    ev.user = u;
                    ev.user.next = NULL;
                    break;
                  default:
                    _v3_error(v3h, "message not implemented: type 0x%02x unknown subtype 0x%02x", m->type, mc->subtype);
                    ret = V3_NOTIMPL;
                    ctr = mc->count;
                    break;
                }
                if (ev.type) {
                    _v3_event_push(v3h, &ev);
                }
                if (mc->subtype == V3_USER_REMOVE) {
                    v3_user *user = v3c->users, *last = NULL;

                    while (user) {
                        if (user->phantom_owner && user->phantom_owner == u.id) {
                            ev.type = V3_EVENT_USER_LOGOUT;
                            memcpy(&ev.user, user, sizeof(v3_user));
                            ev.user.next = NULL;
                            _v3_event_push(v3h, &ev);
                            u.next = user;
                            user = user->next;
                            free(u.next);
                            if (!last) {
                                v3c->users = user;
                            } else {
                                last->next = user;
                            }
                        } else {
                            last = user;
                            user = user->next;
                        }
                    }
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

            for (ptr = m->data + sizeof(_v3_msg_list_chan), ctr = 0; ctr < mc->count; ++ctr) {
                memset(&c, 0, sizeof(v3_channel));
                ptr = _v3_msg_channel_get(v3h, ptr, &c);
                _v3_debug(v3h, V3_DBG_MESSAGE, "channel: id: %u | name: '%s' | phonetic: '%s' | comment: '%s'",
                        c.id,
                        c.name,
                        c.phonetic,
                        c.comment);
                _v3_data(v3h, V3_DATA_UPDATE, V3_DATA_TYPE_CHANNEL, &c, (void *)&c._strings_ - (void *)&c);
                ev.channel = c;
                ev.channel.next = NULL;
                _v3_event_push(v3h, &ev);
            }

            _v3_mutex_unlock(v3h);
        }
        break;
      case V3_MSG_USER_PAGE:
        {
            _v3_msg_user_page *mc = m->data;
            v3_user u = { .id = mc->from };
            v3_event ev = { .type = V3_EVENT_USER_PAGE };

            _v3_debug(v3h, V3_DBG_MESSAGE, "user page: to: %u | from: %u", mc->to, mc->from);
            if (_v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
                ev.user = u;
                _v3_event_push(v3h, &ev);
            }
        }
        break;
      default:
        _v3_error(v3h, "unknown message type 0x%02x", m->type);
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

    _v3_leave(v3h, __func__);
    return ret;
}

