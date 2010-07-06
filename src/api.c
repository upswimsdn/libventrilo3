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

int
v3_logged_in(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK) {
        return _v3_handles[v3h]->logged_in;
    }

    return false;
}

uint16_t
v3_luser_id(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK && v3_logged_in(v3h)) {
        return _v3_handles[v3h]->luser.id;
    }

    return 0;
}

uint16_t
v3_luser_channel(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK && v3_logged_in(v3h)) {
        return _v3_handles[v3h]->luser.channel;
    }

    return 0;
}

uint16_t
v3_licensed(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK && v3_logged_in(v3h)) {
        return _v3_handles[v3h]->licensed;
    }

    return 0;
}

uint16_t
v3_slot_count(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK && v3_logged_in(v3h)) {
        return _v3_handles[v3h]->slots;
    }

    return 0;
}

uint64_t
v3_sent_bytes(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK && v3_logged_in(v3h)) {
        return _v3_handles[v3h]->sent_byte_ctr;
    }

    return 0;
}

uint64_t
v3_recv_bytes(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK && v3_logged_in(v3h)) {
        return _v3_handles[v3h]->recv_byte_ctr;
    }

    return 0;
}

uint32_t
v3_sent_packets(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK && v3_logged_in(v3h)) {
        return _v3_handles[v3h]->sent_pkt_ctr;
    }

    return 0;
}

uint32_t
v3_recv_packets(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK && v3_logged_in(v3h)) {
        return _v3_handles[v3h]->recv_pkt_ctr;
    }

    return 0;
}

int
v3_luser_option(v3_handle v3h, int type, uint8_t value) {
    const char func[] = "v3_luser_option";

    _v3_connection *v3c;
    int ret = V3_OK;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (v3c->logged_in) {
        ret = _v3_msg_user_option_put(v3h, v3c->luser.id, type, value);
    } else {
        switch (type) {
          case V3_USER_ACCEPT_PAGES:
            v3c->luser.accept_pages = value;
            break;
          case V3_USER_ACCEPT_U2U:
            v3c->luser.accept_u2u = value;
            break;
          case V3_USER_ALLOW_RECORD:
            v3c->luser.allow_record = value;
            break;
          case V3_USER_ACCEPT_CHAT:
            v3c->luser.accept_chat = value;
            break;
        }
    }

    _v3_leave(v3h, func);
    return ret;
}

int
v3_luser_text(v3_handle v3h, const char *comment, const char *url, const char *integration, uint8_t silent) {
    const char func[] = "v3_luser_text";

    _v3_connection *v3c;
    v3_user u;
    int ret = V3_OK;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (v3c->logged_in) {
        memset(&u, 0, sizeof(v3_user));
        u.id = v3c->luser.id;
        u.flags = (silent) ? 1 << 8 : 0;
        _v3_strncpy(u.comment, comment, sizeof(u.comment) - 1);
        _v3_strncpy(u.url, url, sizeof(u.url) - 1);
        _v3_strncpy(u.integration, integration, sizeof(u.integration) - 1);
        ret = _v3_msg_user_list_put(v3h, V3_USER_UPDATE, &u);
    } else {
        _v3_strncpy(v3c->luser.comment, comment, sizeof(v3c->luser.comment) - 1);
        _v3_strncpy(v3c->luser.url, url, sizeof(v3c->luser.url) - 1);
        _v3_strncpy(v3c->luser.integration, integration, sizeof(v3c->luser.integration) - 1);
    }

    _v3_leave(v3h, func);
    return ret;
}

int
v3_channel_change(v3_handle v3h, uint16_t id, const char *password) {
    const char func[] = "v3_channel_change";

    v3_channel c = { .id = id };
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    ret = _v3_msg_chan_list_put(v3h, V3_CHAN_CHANGE, v3_luser_id(v3h), password, &c);

    _v3_leave(v3h, func);
    return ret;
}

int
v3_user_mute(v3_handle v3h, uint16_t id, uint8_t mute) {
    const char func[] = "v3_user_mute";

    v3_user u = { .id = id };
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    if ((ret = _v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u)) == V3_OK) {
        u.next->muted_local = mute;
    }

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return ret;
}

int
v3_chat_join(v3_handle v3h) {
    const char func[] = "v3_chat_join";

    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    ret = _v3_msg_chat_put(v3h, V3_CHAT_JOIN, NULL);

    _v3_leave(v3h, func);
    return ret;
}

int
v3_chat_leave(v3_handle v3h) {
    const char func[] = "v3_chat_leave";

    int ret = V3_OK;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    ret = _v3_msg_chat_put(v3h, V3_CHAT_LEAVE, NULL);

    _v3_leave(v3h, func);
    return ret;
}

int
v3_chat_message(v3_handle v3h, const char *message) {
    const char func[] = "v3_chat_message";

    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    ret = _v3_msg_chat_put(v3h, V3_CHAT_MESSAGE, message);

    _v3_leave(v3h, func);
    return ret;
}


















