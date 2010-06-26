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
_v3_data_update(v3_handle v3h, int type, void *data) {
    const char func[] = "_v3_data_update";

    _v3_connection *v3c;
    void **list;
    size_t size;
    void *ptr, *last;
    int ctr = 0;

    _v3_enter(v3h, func);

    //_v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];
    switch (type) {
      case V3_DATA_CHANNEL:
        list = (void **)&v3c->channels;
        size = sizeof(v3_channel);
        _v3_debug(v3h, V3_DBG_INFO, "updating channel linked list");
        break;
      case V3_DATA_RANK:
        list = (void **)&v3c->ranks;
        size = sizeof(v3_rank);
        _v3_debug(v3h, V3_DBG_INFO, "updating rank linked list");
        break;
      case V3_DATA_USER:
        list = (void **)&v3c->users;
        size = sizeof(v3_user);
        _v3_debug(v3h, V3_DBG_INFO, "updating user linked list");
        break;
      case V3_DATA_ACCOUNT:
        list = (void **)&v3c->accounts;
        size = sizeof(v3_account);
        _v3_debug(v3h, V3_DBG_INFO, "updating account linked list");
        break;
      default:
        _v3_mutex_unlock(v3h);
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }
    if (!(ptr = *list)) {
        *list = calloc(1, size);
        memcpy(*list, data, size-sizeof(void *));
        _v3_debug(v3h, V3_DBG_INFO, "initialized linked list");
    } else {
        while (ptr) {
            if (*((uint16_t *)ptr) == *((uint16_t *)data)) {
                _v3_debug(v3h, V3_DBG_INFO, "updating linked list at item %i", ctr);
                break;
            }
            last = ptr;
            memcpy(&ptr, ptr+(size-sizeof(void *)), sizeof(void *));
            ctr++;
        }
        if (!ptr) {
            ptr = calloc(1, size);
            memcpy(last+(size-sizeof(void *)), &ptr, sizeof(void *));
            _v3_debug(v3h, V3_DBG_INFO, "appended linked list at item %i", ctr);
        }
        memcpy(ptr, data, size-sizeof(void *));
    }

    //_v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return V3_OK;
}

