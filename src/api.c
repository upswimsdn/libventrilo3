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

int
v3_logged_in(v3_handle v3h) {
    if (_v3_handle_valid(v3h) == V3_OK) {
        return _v3_handles[v3h]->logged_in;
    }

    return false;
}

uint16_t
v3_luser_id(v3_handle v3h) {
    if (v3_logged_in(v3h)) {
        return _v3_handles[v3h]->luser.id;
    }

    return 0;
}

uint16_t
v3_luser_channel(v3_handle v3h) {
    if (v3_logged_in(v3h)) {
        return _v3_handles[v3h]->luser.channel;
    }

    return 0;
}

uint16_t
v3_licensed(v3_handle v3h) {
    if (v3_logged_in(v3h)) {
        return _v3_handles[v3h]->licensed;
    }

    return 0;
}

uint16_t
v3_slot_count(v3_handle v3h) {
    if (v3_logged_in(v3h)) {
        return _v3_handles[v3h]->slots;
    }

    return 0;
}

uint64_t
v3_sent_bytes(v3_handle v3h) {
    if (v3_logged_in(v3h)) {
        return _v3_handles[v3h]->sent_byte_ctr;
    }

    return 0;
}

uint64_t
v3_recv_bytes(v3_handle v3h) {
    if (v3_logged_in(v3h)) {
        return _v3_handles[v3h]->recv_byte_ctr;
    }

    return 0;
}

uint32_t
v3_sent_packets(v3_handle v3h) {
    if (v3_logged_in(v3h)) {
        return _v3_handles[v3h]->sent_pkt_ctr;
    }

    return 0;
}

uint32_t
v3_recv_packets(v3_handle v3h) {
    if (v3_logged_in(v3h)) {
        return _v3_handles[v3h]->recv_pkt_ctr;
    }

    return 0;
}

int
v3_luser_option(v3_handle v3h, int type, uint8_t value) {
    _v3_connection *v3c;
    int ret = V3_OK;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

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

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_luser_text(v3_handle v3h, const char *comment, const char *url, const char *integration, uint8_t silent) {
    _v3_connection *v3c;
    v3_user u;
    int ret = V3_OK;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

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

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_channel_change(v3_handle v3h, uint16_t id, const char *password) {
    v3_channel c = { .id = id };
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_chan_list_put(v3h, V3_CHAN_CHANGE, v3_luser_id(v3h), password, &c);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_user_mute(v3_handle v3h, uint16_t id, uint8_t mute) {
    v3_user u = { .id = id };
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    _v3_mutex_lock(v3h);

    if ((ret = _v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u, 0)) == V3_OK) {
        u.next->muted_local = mute;
    }

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_user_page(v3_handle v3h, uint16_t id) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    _v3_mutex_lock(v3h);

    ret = _v3_msg_user_page_put(v3h, id, v3_luser_id(v3h));

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_chat_join(v3_handle v3h) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_chat_put(v3h, V3_CHAT_JOIN, NULL);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_chat_leave(v3_handle v3h) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_chat_put(v3h, V3_CHAT_LEAVE, NULL);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_chat_message(v3_handle v3h, const char *message) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_chat_put(v3h, V3_CHAT_MESSAGE, message);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_audio_start(v3_handle v3h) {
    _v3_connection *v3c;
    const v3_codec *codec;
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];
    codec = v3_codec_channel_get(v3h, v3c->luser.channel);
    ret = _v3_msg_audio_put(v3h, V3_AUDIO_START, codec->index, codec->format, 0, NULL, 0);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_audio_stop(v3_handle v3h) {
    _v3_connection *v3c;
    const v3_codec *codec;
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];
    codec = v3_codec_channel_get(v3h, v3c->luser.channel);
    ret = _v3_msg_audio_put(v3h, V3_AUDIO_STOP, codec->index, codec->format, 0, NULL, 0);

    v3c->pcmqueued = 0;
#if HAVE_SPEEXDSP
    if (v3c->resampler.state) {
        speex_resampler_destroy(v3c->resampler.state);
        v3c->resampler.state = NULL;
    }
#endif
    _v3_coder_destroy(v3h, &v3c->encoder);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_audio_send(v3_handle v3h, uint32_t rate, uint8_t channels, const void *pcm, uint32_t pcmlen) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_audio_send(v3h, rate, channels, pcm, pcmlen);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_phantom_add(v3_handle v3h, uint16_t id) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_phantom_put(v3h, V3_PHANTOM_ADD, 0, id);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_phantom_remove(v3_handle v3h, uint16_t id) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_phantom_put(v3h, V3_PHANTOM_REMOVE, id, 0);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_login(v3_handle v3h, const char *password) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_admin_put(v3h, V3_ADMIN_LOGIN, 0, password);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_logout(v3_handle v3h) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_admin_put(v3h, V3_ADMIN_LOGOUT, 0, NULL);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_channel_kick(v3_handle v3h, uint16_t id) {
    v3_user u = { .id = id };
    v3_channel c = { .id = 0 };
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    if ((ret = _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_USER, &u, 0)) == V3_OK) {
        c.id = u.channel;
        ret = _v3_msg_chan_list_put(v3h, V3_CHAN_KICK, id, NULL, &c);
    }

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_channel_ban(v3_handle v3h, uint16_t id, const char *reason) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_admin_put(v3h, V3_ADMIN_CHAN_BAN, id, reason);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_kick(v3_handle v3h, uint16_t id, const char *reason) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_admin_put(v3h, V3_ADMIN_KICK, id, reason);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_ban(v3_handle v3h, uint16_t id, const char *reason) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_admin_put(v3h, V3_ADMIN_BAN, id, reason);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_move(v3_handle v3h, uint16_t src, uint16_t dest) {
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_msg_move_put(v3h, src, dest);

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_mute_channel(v3_handle v3h, uint16_t id) {
    v3_user u = { .id = id };
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    if ((ret = _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_USER, &u, 0)) == V3_OK) {
        ret = _v3_msg_user_option_put(v3h, id, V3_USER_CHANNEL_MUTE, !u.muted_channel);
    }

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_admin_mute_global(v3_handle v3h, uint16_t id) {
    v3_user u = { .id = id };
    int ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    if ((ret = _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_USER, &u, 0)) == V3_OK) {
        ret = _v3_msg_user_option_put(v3h, id, V3_USER_GLOBAL_MUTE, !u.muted_global);
    }

    _v3_leave(v3h, __func__);
    return ret;
}

