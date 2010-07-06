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
_v3_data(v3_handle v3h, int oper, int type, void *data) {
    const char func[] = "_v3_data";

    _v3_connection *v3c;
    const char *tstr;
    void **list;
    size_t size;
    void *ptr, *last = NULL;
    int ctr = 0;

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
        _v3_debug(v3h, V3_DBG_INFO, "updating %s linked list", tstr);
        if (!(ptr = *list)) {
            *list = calloc(1, size);
            memcpy(*list, data, size-sizeof(void *));
            _v3_debug(v3h, V3_DBG_INFO, "initialized %s linked list", tstr);
            break;
        }
        while (ptr) {
            if (*(uint16_t *)ptr != *(uint16_t *)data) {
                last = ptr;
                memcpy(&ptr, ptr+(size-sizeof(void *)), sizeof(void *));
                ctr++;
                continue;
            }
            _v3_debug(v3h, V3_DBG_INFO, "updating %s linked list at item %i", tstr, ctr+1);
            if (type == V3_DATA_TYPE_USER) {
                v3_user *src = data, *dest = ptr;
                strncpy(src->name, dest->name, sizeof(src->name) - 1);
                strncpy(src->phonetic, dest->phonetic, sizeof(src->phonetic) - 1);
            }
            break;
        }
        if (!ptr) {
            ptr = calloc(1, size);
            memcpy(last+(size-sizeof(void *)), &ptr, sizeof(void *));
            _v3_debug(v3h, V3_DBG_INFO, "appended %s linked list at item %i", tstr, ctr);
        }
        memcpy(ptr, data, size-sizeof(void *));
        break;
      case V3_DATA_COPY:
      case V3_DATA_RETURN:
        _v3_debug(v3h, V3_DBG_INFO, "retrieving %s id %u from linked list", tstr, *(uint16_t *)data);
        if (!(ptr = *list)) {
            _v3_error(v3h, "%s linked list not initialized", tstr);
            _v3_mutex_unlock(v3h);
            _v3_leave(v3h, func);
            return V3_FAILURE;
        }
        while (ptr && *(uint16_t *)ptr != *(uint16_t *)data) {
            memcpy(&ptr, ptr+(size-sizeof(void *)), sizeof(void *));
        }
        if (!ptr) {
            _v3_error(v3h, "%s id %u not found", tstr, *(uint16_t *)data);
            _v3_mutex_unlock(v3h);
            _v3_leave(v3h, func);
            return V3_FAILURE;
        }
        memcpy(data, ptr, size-sizeof(void *));
        if (oper == V3_DATA_RETURN) {
            memcpy(data+(size-sizeof(void *)), &ptr, sizeof(void *));
        }
        break;
      case V3_DATA_REMOVE:
        _v3_debug(v3h, V3_DBG_INFO, "removing %s id %u from linked list", tstr, *(uint16_t *)data);
        if (!(ptr = *list)) {
            _v3_error(v3h, "%s linked list not initialized", tstr);
            _v3_mutex_unlock(v3h);
            _v3_leave(v3h, func);
            return V3_FAILURE;
        }
        while (ptr) {
            if (*(uint16_t *)ptr == *(uint16_t *)data) {
                memcpy((!last) ? list : last+(size-sizeof(void *)), ptr+(size-sizeof(void *)), sizeof(void *));
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
        _v3_debug(v3h, V3_DBG_INFO, "clearing %s linked list", tstr);
        if (!*list) {
            _v3_error(v3h, "%s linked list not initialized", tstr);
            _v3_mutex_unlock(v3h);
            _v3_leave(v3h, func);
            return V3_FAILURE;
        }
        while (*list) {
            memcpy(&ptr, *list+(size-sizeof(void *)), sizeof(void *));
            free(*list);
            _v3_debug(v3h, V3_DBG_MEMORY, "released %s item %i", tstr, ++ctr);
            *list = ptr;
        }
        break;
      default:
        _v3_error(v3h, "unknown data operation: %i", oper);
        _v3_mutex_unlock(v3h);
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return V3_OK;
}

int
_v3_data_count(v3_handle v3h, int type) {
    _v3_connection *v3c = _v3_handles[v3h];
    void *list;
    size_t size;
    int ctr;

    if (!v3c->mutex) {
        return 0;
    }
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
_v3_event_mutex_init(v3_handle v3h) {
    const char func[] = "_v3_event_mutex_init";

    _v3_connection *v3c;
    pthread_mutexattr_t mta;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (!v3c->event_mutex) {
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
        _v3_debug(v3h, V3_DBG_MUTEX, "initializing eventq mutex");
        v3c->event_mutex = malloc(sizeof(pthread_mutex_t));
        v3c->event_cond = malloc(sizeof(pthread_cond_t));
        pthread_mutex_init(v3c->event_mutex, &mta);
        pthread_cond_init(v3c->event_cond, (pthread_condattr_t *)&mta);
    }

    _v3_leave(v3h, func);
}

void
_v3_event_mutex_destroy(v3_handle v3h) {
    const char func[] = "_v3_event_mutex_destroy";

    _v3_connection *v3c;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (v3c->event_mutex) {
        _v3_debug(v3h, V3_DBG_MUTEX, "destroying eventq mutex");
        pthread_cond_broadcast(v3c->event_cond);
        pthread_cond_destroy(v3c->event_cond);
        pthread_mutex_destroy(v3c->event_mutex);
        free(v3c->event_cond);
        free(v3c->event_mutex);
        v3c->event_cond = NULL;
        v3c->event_mutex = NULL;
    }

    _v3_leave(v3h, func);
}

void
_v3_event_mutex_lock(v3_handle v3h) {
    const char func[] = "_v3_event_mutex_lock";

    _v3_connection *v3c = _v3_handles[v3h];

    if (!v3c->event_mutex) {
        _v3_enter(v3h, func);
        _v3_event_mutex_init(v3h);
        _v3_leave(v3h, func);
    }
    pthread_mutex_lock(v3c->event_mutex);
}

void
_v3_event_mutex_unlock(v3_handle v3h) {
    const char func[] = "_v3_event_mutex_unlock";

    _v3_connection *v3c = _v3_handles[v3h];

    if (!v3c->event_mutex) {
        _v3_enter(v3h, func);
        _v3_event_mutex_init(v3h);
        _v3_leave(v3h, func);
    } else {
        pthread_mutex_unlock(v3c->event_mutex);
    }
}

void
_v3_event_cond_signal(v3_handle v3h) {
    _v3_connection *v3c = _v3_handles[v3h];

    if (v3c->event_cond) {
        pthread_cond_signal(v3c->event_cond);
    }
}

void
_v3_event_cond_wait(v3_handle v3h) {
    _v3_connection *v3c = _v3_handles[v3h];

    if (v3c->event_cond) {
        pthread_cond_wait(v3c->event_cond, v3c->event_mutex);
    }
}

void
_v3_event_push(v3_handle v3h, v3_event *ev) {
    const char func[] = "_v3_event_push";

    _v3_connection *v3c;
    v3_event *last;
    int ctr;

    _v3_enter(v3h, func);

    _v3_event_mutex_lock(v3h);

    last = calloc(1, sizeof(v3_event));
    memcpy(last, ev, sizeof(v3_event) - sizeof(last->next));
    ev = last;
    v3c = _v3_handles[v3h];

    for (last = v3c->eventq, ctr = 0; last && ++ctr && last->next; last = last->next);

    if (!v3c->eventq) {
        v3c->eventq = ev;
    } else {
        last->next = ev;
    }
    _v3_debug(v3h, V3_DBG_EVENT, "%i events in queue", ++ctr);
    _v3_event_cond_signal(v3h);

    _v3_event_mutex_unlock(v3h);

    _v3_leave(v3h, func);
}

int
_v3_event_pop(v3_handle v3h, int block, v3_event *ev) {
    _v3_connection *v3c = _v3_handles[v3h];

    if (!block && !v3c->eventq) {
        return V3_FAILURE;
    }
    _v3_event_mutex_lock(v3h);

    if (!v3c->eventq) {
        _v3_debug(v3h, V3_DBG_EVENT, "waiting for an event...");
        _v3_event_cond_wait(v3h);
    }
    memcpy(ev, v3c->eventq, sizeof(v3_event) - sizeof(ev->next));
    ev = v3c->eventq;
    v3c->eventq = ev->next;
    free(ev);

    _v3_event_mutex_unlock(v3h);

    return V3_OK;
}

int
v3_event_count(v3_handle v3h) {
    v3_event *ev;
    int ctr;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }
    _v3_event_mutex_lock(v3h);

    for (ctr = 0, ev = _v3_handles[v3h]->eventq; ev; ctr++, ev = ev->next);

    _v3_event_mutex_unlock(v3h);

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
    v3_event *ev, *next;
    int ctr = 0;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return;
    }
    _v3_event_mutex_lock(v3h);

    ev = _v3_handles[v3h]->eventq;

    while (ev && ++ctr) {
        next = ev->next;
        free(ev);
        ev = next;
    }
    _v3_debug(v3h, V3_DBG_EVENT, "released %i events", ctr);

    _v3_event_mutex_unlock(v3h);
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

    return _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_CHANNEL, c);
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

    return _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_RANK, r);
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

    return _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_USER, u);
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

    return _v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_ACCOUNT, a);
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
            if (!c->next) {
                ctr = depth;
                id = 0;
            }
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

