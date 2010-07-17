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
_v3_data(v3_handle v3h, int oper, int type, void *data, size_t n) {
    const char func[] = "_v3_data";

    _v3_connection *v3c;
    const char *tstr;
    void **list;
    size_t size;
    void *ptr, *last = NULL;
    int ctr = 0;
    int ret = V3_OK;

    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];

    switch (type) {
      case V3_DATA_TYPE_CHANNEL:
        tstr = "channel";
        list = (void **)&v3c->channels;
        size = sizeof(v3_channel);
        break;
      case V3_DATA_TYPE_RANK:
        tstr = "rank";
        list = (void **)&v3c->ranks;
        size = sizeof(v3_rank);
        break;
      case V3_DATA_TYPE_USER:
        tstr = "user";
        list = (void **)&v3c->users;
        size = sizeof(v3_user);
        break;
      case V3_DATA_TYPE_ACCOUNT:
        tstr = "account";
        list = (void **)&v3c->accounts;
        size = sizeof(v3_account);
        break;
      default:
        _v3_error(v3h, "unknown data type: %i", type);
        _v3_mutex_unlock(v3h);
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }
    switch (oper) {
      case V3_DATA_UPDATE:
        _v3_debug(v3h, V3_DBG_INFO, "updating %s list", tstr);
        if (!(ptr = *list)) {
            *list = calloc(1, size);
            memcpy(*list, data, size-sizeof(void *));
            _v3_debug(v3h, V3_DBG_INFO, "initialized %s list", tstr);
            break;
        }
        while (ptr) {
            if (*(uint16_t *)ptr != *(uint16_t *)data) {
                last = ptr;
                memcpy(&ptr, ptr+(size-sizeof(void *)), sizeof(void *));
                ctr++;
                continue;
            }
            _v3_debug(v3h, V3_DBG_INFO, "updating %s list at item %i", tstr, ctr+1);
            if (type == V3_DATA_TYPE_USER) {
                v3_user *src = data, *dest = ptr;
                strncpy(src->name, dest->name, sizeof(src->name) - 1);
                strncpy(src->phonetic, dest->phonetic, sizeof(src->phonetic) - 1);
            }
            memcpy(ptr, data, (n) ? n : size-sizeof(void *));
            break;
        }
        if (!ptr) {
            ptr = calloc(1, size);
            memcpy(last+(size-sizeof(void *)), &ptr, sizeof(void *));
            memcpy(ptr, data, size-sizeof(void *));
            _v3_debug(v3h, V3_DBG_INFO, "appended %s list at item %i", tstr, ctr);
        }
        break;
      case V3_DATA_COPY:
      case V3_DATA_RETURN:
        _v3_debug(v3h, V3_DBG_INFO, "retrieving id %u from %s list", *(uint16_t *)data, tstr);
        if (!(ptr = *list)) {
            _v3_error(v3h, "%s list not initialized", tstr);
            ret = V3_FAILURE;
            break;
        }
        while (ptr && *(uint16_t *)ptr != *(uint16_t *)data) {
            memcpy(&ptr, ptr+(size-sizeof(void *)), sizeof(void *));
        }
        if (!ptr) {
            _v3_error(v3h, "%s id %u not found", tstr, *(uint16_t *)data);
            ret = V3_FAILURE;
            break;
        }
        memcpy(data, ptr, size-sizeof(void *));
        if (oper == V3_DATA_RETURN) {
            memcpy(data+(size-sizeof(void *)), &ptr, sizeof(void *));
        }
        break;
      case V3_DATA_REMOVE:
        _v3_debug(v3h, V3_DBG_INFO, "releasing id %u from %s list", *(uint16_t *)data, tstr);
        if (!(ptr = *list)) {
            _v3_error(v3h, "%s list not initialized", tstr);
            ret = V3_FAILURE;
            break;
        }
        while (ptr) {
            if (*(uint16_t *)ptr == *(uint16_t *)data) {
                memcpy((!last) ? list : last+(size-sizeof(void *)), ptr+(size-sizeof(void *)), sizeof(void *));
                if (type == V3_DATA_TYPE_USER) {
                    _v3_coder_destroy(v3h, &((v3_user *)ptr)->decoder);
                }
                free(ptr);
                _v3_debug(v3h, V3_DBG_MEMORY, "released %s id %u", tstr, *(uint16_t *)data);
                break;
            }
            last = ptr;
            memcpy(&ptr, ptr+(size-sizeof(void *)), sizeof(void *));
        }
        if (!ptr) {
            _v3_debug(v3h, V3_DBG_INFO, "%s id %u not found", tstr, *(uint16_t *)data);
        }
        break;
      case V3_DATA_CLEAR:
        _v3_debug(v3h, V3_DBG_INFO, "clearing %s list", tstr);
        if (!*list) {
            _v3_error(v3h, "%s list not initialized", tstr);
            ret = V3_FAILURE;
            break;
        }
        while (*list) {
            memcpy(&ptr, *list+(size-sizeof(void *)), sizeof(void *));
            if (type == V3_DATA_TYPE_USER) {
                _v3_coder_destroy(v3h, &((v3_user *)*list)->decoder);
            }
            free(*list);
            _v3_debug(v3h, V3_DBG_MEMORY, "released %s item %i", tstr, ++ctr);
            *list = ptr;
        }
        break;
      default:
        _v3_error(v3h, "unknown data operation: %i", oper);
        ret = V3_FAILURE;
        break;
    }

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return ret;
}

int
_v3_data_count(v3_handle v3h, int type) {
    _v3_connection *v3c = _v3_handles[v3h];
    void *list;
    size_t size;
    int ctr;

    pthread_mutex_lock(v3c->mutex);

    switch (type) {
      case V3_DATA_TYPE_CHANNEL:
        list = v3c->channels;
        size = sizeof(v3_channel);
        break;
      case V3_DATA_TYPE_RANK:
        list = v3c->ranks;
        size = sizeof(v3_rank);
        break;
      case V3_DATA_TYPE_USER:
        list = v3c->users;
        size = sizeof(v3_user);
        break;
      case V3_DATA_TYPE_ACCOUNT:
        list = v3c->accounts;
        size = sizeof(v3_account);
        break;
      default:
        list = NULL;
        break;
    }
    for (ctr = 0; list; ctr++) {
        memcpy(&list, list+(size-sizeof(void *)), sizeof(void *));
    }

    pthread_mutex_unlock(v3c->mutex);

    return ctr;
}

void
_v3_data_destroy(v3_handle v3h) {
    const char func[] = "_v3_data_destroy";

    _v3_enter(v3h, func);

    _v3_data(v3h, V3_DATA_CLEAR, V3_DATA_TYPE_CHANNEL, NULL, 0);
    _v3_data(v3h, V3_DATA_CLEAR, V3_DATA_TYPE_RANK, NULL, 0);
    _v3_data(v3h, V3_DATA_CLEAR, V3_DATA_TYPE_USER, NULL, 0);
    _v3_data(v3h, V3_DATA_CLEAR, V3_DATA_TYPE_ACCOUNT, NULL, 0);

    _v3_leave(v3h, func);
}

void
_v3_event_push(v3_handle v3h, v3_event *ev) {
    const char func[] = "_v3_event_push";

    _v3_connection *v3c;
    v3_event *last;
    int ctr;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];

    pthread_mutex_lock(v3c->event_mutex);

    last = calloc(1, sizeof(v3_event));
    memcpy(last, ev, sizeof(v3_event) - sizeof(last->next));
    ev = last;

    for (last = v3c->eventq, ctr = 0; last && ++ctr && last->next; last = last->next);

    if (!v3c->eventq) {
        v3c->eventq = ev;
    } else {
        last->next = ev;
    }
    _v3_debug(v3h, V3_DBG_EVENT, "%i events in queue", ++ctr);

    pthread_cond_signal(v3c->event_cond);

    pthread_mutex_unlock(v3c->event_mutex);

    _v3_leave(v3h, func);
}

int
_v3_event_pop(v3_handle v3h, int block, v3_event *ev) {
    _v3_connection *v3c = _v3_handles[v3h];

    if (!block && !v3c->eventq) {
        return V3_FAILURE;
    }

    pthread_mutex_lock(v3c->event_mutex);

    if (!v3c->eventq) {
        pthread_cond_wait(v3c->event_cond, v3c->event_mutex);
        if (!v3c->eventq) {
            pthread_mutex_unlock(v3c->event_mutex);
            return V3_FAILURE;
        }
    }
    memcpy(ev, v3c->eventq, sizeof(v3_event) - sizeof(ev->next));
    ev = v3c->eventq;
    v3c->eventq = ev->next;
    free(ev);

    pthread_mutex_unlock(v3c->event_mutex);

    return V3_OK;
}

void
_v3_event_clear(v3_handle v3h) {
    const char func[] = "_v3_event_clear";

    _v3_connection *v3c;
    v3_event *ev, *next;
    int ctr = 0;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];

    pthread_mutex_lock(v3c->event_mutex);

    ev = v3c->eventq;

    while (ev && ++ctr) {
        next = ev->next;
        free(ev);
        ev = next;
    }
    _v3_debug(v3h, V3_DBG_EVENT, "released %i events", ctr);

    pthread_mutex_unlock(v3c->event_mutex);

    _v3_leave(v3h, func);
}

int
v3_event_count(v3_handle v3h) {
    _v3_connection *v3c;
    v3_event *ev;
    int ctr;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }

    v3c = _v3_handles[v3h];

    pthread_mutex_lock(v3c->event_mutex);

    for (ctr = 0, ev = v3c->eventq; ev; ctr++, ev = ev->next);

    pthread_mutex_unlock(v3c->event_mutex);

    return ctr;
}

int
v3_event_get(v3_handle v3h, int block, v3_event *ev) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }

    return _v3_event_pop(v3h, block, ev);
}

void
v3_event_clear(v3_handle v3h) {
    const char func[] = "v3_event_clear";

    if (_v3_handle_valid(v3h) != V3_OK) {
        return;
    }
    _v3_enter(v3h, func);

    _v3_event_clear(v3h);

    _v3_leave(v3h, func);
}

int
v3_channel_count(v3_handle v3h) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }

    return _v3_data_count(v3h, V3_DATA_TYPE_CHANNEL);
}

int
v3_channel_get(v3_handle v3h, v3_channel *c) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }

    return _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_CHANNEL, c, 0);
}

int
v3_rank_count(v3_handle v3h) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }

    return _v3_data_count(v3h, V3_DATA_TYPE_RANK);
}

int
v3_rank_get(v3_handle v3h, v3_rank *r) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }

    return _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_RANK, r, 0);
}

int
v3_user_count(v3_handle v3h) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }

    return _v3_data_count(v3h, V3_DATA_TYPE_USER) - 1;
}

int
v3_user_get(v3_handle v3h, v3_user *u) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }

    return _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_USER, u, 0);
}

int
v3_account_count(v3_handle v3h) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }

    return _v3_data_count(v3h, V3_DATA_TYPE_ACCOUNT);
}

int
v3_account_get(v3_handle v3h, v3_account *a) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }

    return _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_ACCOUNT, a, 0);
}

int
v3_lacct(v3_handle v3h, v3_account *a) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }

    memcpy(a, &_v3_handles[v3h]->lacct, sizeof(v3_account));

    return V3_OK;
}

int
v3_channel_admin(v3_handle v3h, uint16_t id) {
    const char func[] = "v3_channel_admin";

    _v3_connection *v3c;
    uint16_t ctr;
    int ret = false;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }
    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];

    for (ctr = 0; id && ctr < v3c->lacct.chan_admin_count; ctr++) {
        if (v3c->lacct.chan_admin[ctr] == id) {
            ret = true;
            break;
        }
    }

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return ret;
}

uint16_t
_v3_channel_password(v3_handle v3h, uint16_t id) {
    v3_channel c = { .id = id };

    if (!id || v3_channel_get(v3h, &c) != V3_OK) {
        return 0;
    }
    if (c.protect_mode == 1) {
        return id;
    }

    return _v3_channel_password(v3h, c.parent);
}

uint16_t
v3_channel_password(v3_handle v3h, uint16_t id) {
    const char func[] = "v3_channel_password";

    uint16_t ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }
    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    ret = _v3_channel_password(v3h, id);

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return ret;
}

char *
v3_channel_path(v3_handle v3h, uint16_t id) {
    const char func[] = "v3_channel_path";

    _v3_connection *v3c;
    v3_channel *c;
    const char sep = '/';
    uint16_t parent = 0;
    char *path = NULL, *prepend;
    int chanlen, pathlen;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return NULL;
    }
    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];

    for (c = v3c->channels; c; c = (!c) ? v3c->channels : c->next) {
        if ((!parent && c->id == id) || (parent && c->id == parent)) {
            chanlen = strlen(c->name);
            if (!path) {
                pathlen = chanlen + 1;
                path = calloc(pathlen, 1);
                memcpy(path, c->name, chanlen);
            } else {
                prepend = calloc(chanlen + 1 + pathlen, 1);
                memcpy(prepend, c->name, chanlen);
                prepend[chanlen] = sep;
                memcpy(prepend + chanlen + 1, path, pathlen);
                pathlen += chanlen + 1;
                free(path);
                path = prepend;
            }
            if (!(parent = c->parent)) {
                break;
            }
            c = NULL;
        }
    }

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return path;
}

uint16_t
v3_channel_id(v3_handle v3h, const char *path) {
    const char func[] = "v3_channel_id";

    _v3_connection *v3c;
    v3_channel *c;
    const char sep = '/';
    const char *p = path;
    const char **names = NULL;
    char name[sizeof(c->name)];
    int depth = 0;
    size_t len = 0;
    int ctr;
    uint16_t id = 0;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }
    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];

    if (!path || !*path) {
        _v3_mutex_unlock(v3h);
        _v3_leave(v3h, func);
        return 0;
    }
    for (;;) {
        if (*p == sep || !*p) {
            names = realloc(names, sizeof(*names) * ++depth);
            if (!len) {
                free(names);
                _v3_mutex_unlock(v3h);
                _v3_leave(v3h, func);
                return 0;
            }
            names[depth-1] = p - len;
            len = 0;
            if (!*p) {
                break;
            }
        } else {
            len++;
        }
        p++;
    }
    for (ctr = 0; ctr < depth; ctr++) {
        memset(name, 0, sizeof(name));
        len = strlen(names[ctr]) - ((ctr + 1 < depth) ? strlen(names[ctr + 1]) + 1 : 0);
        if (len > sizeof(name)) {
            id = 0;
            break;
        }
        strncpy(name, names[ctr], len);
        for (c = v3c->channels; c; c = c->next) {
            if (c->parent == id && !strncmp(c->name, name, sizeof(name))) {
                id = c->id;
                break;
            }
        }
        if (!c) {
            id = 0;
            break;
        }
    }
    free(names);

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return id;
}

int
v3_channel_sort(v3_handle v3h, uint16_t left, uint16_t right) {
    const char func[] = "v3_channel_sort";

    _v3_connection *v3c;
    v3_channel *c;
    int sort = 0, lpos = -1, rpos = -1;
    int ctr;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }
    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];

    for (c = v3c->channels, ctr = 0; c; c = c->next, ctr++) {
        if (c->id == left) {
            lpos = ctr;
        }
        if (c->id == right) {
            rpos = ctr;
        }
    }
    if (lpos != -1 && rpos != -1 && lpos != rpos) {
        sort = (lpos < rpos) ? -1 : 1;
    }

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return sort;
}

