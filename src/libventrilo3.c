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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "libventrilo3.h"

pthread_mutex_t _v3_handles_mutex        = PTHREAD_MUTEX_INITIALIZER;
_v3_connection *_v3_handles[V3_MAX_CONN] = { NULL };

int16_t  _stack      = 0;
uint16_t _debug      = V3_DBG_NONE;
char     _error[256] = "";
float    _volume     = 1.0;

/*
 * Initialize a libventrilo3 handle for a server connection.
 * @return A pointer to a v3_handle for performing operations on a server.
 */
v3_handle
v3_init(const char *server, const char *username) {
    v3_handle v3h;
    _v3_connection *v3c;
    uint32_t ip;
    uint16_t port;

    _v3_enter(V3_HANDLE_NONE, __func__);

    if (!username || !*username) {
        _v3_error(V3_HANDLE_NONE, "no username specified");
        _v3_leave(V3_HANDLE_NONE, __func__);
        return V3_HANDLE_NONE;
    }
    if (_v3_parse_server(server, &ip, &port) != V3_OK) {
        _v3_leave(V3_HANDLE_NONE, __func__);
        return V3_HANDLE_NONE;
    }

    _v3_mutex_lock(V3_HANDLE_NONE);

    for (v3h = 0; v3h < V3_MAX_CONN && _v3_handles[v3h]; ++v3h);

    if (v3h >= V3_MAX_CONN) {
        _v3_mutex_unlock(V3_HANDLE_NONE);
        _v3_error(V3_HANDLE_NONE, "maximum number of open handles reached: %i", V3_MAX_CONN);
        _v3_leave(V3_HANDLE_NONE, __func__);
        return V3_HANDLE_NONE;
    }
    _v3_handles[v3h] = calloc(1, sizeof(**_v3_handles));

    _v3_mutex_unlock(V3_HANDLE_NONE);

    v3c = _v3_handles[v3h];
    v3c->ip = ip;
    v3c->port = port;
    v3c->sd = -1;
    v3c->luser.volume = 1.0;
    v3c->volume = 1.0;
    strncpy(v3c->luser.name, username, sizeof(v3c->luser.name) - 1);
    _v3_debug(V3_HANDLE_NONE, V3_DBG_INFO, "username: '%s'", v3c->luser.name);
    _v3_mutex_init(v3h);

    _v3_leave(V3_HANDLE_NONE, __func__);
    return v3h;
}

v3_handle
v3_find(const char *server, const char *username) {
    v3_handle v3h;
    _v3_connection *v3c;
    uint32_t ip;
    uint16_t port;

    _v3_enter(V3_HANDLE_NONE, __func__);

    if (!username || !*username) {
        _v3_error(V3_HANDLE_NONE, "no username specified");
        _v3_leave(V3_HANDLE_NONE, __func__);
        return V3_HANDLE_NONE;
    }
    if (_v3_parse_server(server, &ip, &port) != V3_OK) {
        _v3_leave(V3_HANDLE_NONE, __func__);
        return V3_HANDLE_NONE;
    }

    _v3_mutex_lock(V3_HANDLE_NONE);

    for (v3h = 0; v3h < V3_MAX_CONN && (v3c = _v3_handles[v3h]); ++v3h) {
        if (v3c->ip == ip && v3c->port == port && !strncmp(v3c->luser.name, username, sizeof(v3c->luser.name) - 1)) {
            _v3_mutex_unlock(V3_HANDLE_NONE);
            _v3_leave(V3_HANDLE_NONE, __func__);
            return v3h;
        }
    }

    _v3_mutex_unlock(V3_HANDLE_NONE);

    _v3_error(V3_HANDLE_NONE, "handle for server address '%s' username '%s' not found", server, username);

    _v3_leave(V3_HANDLE_NONE, __func__);
    return V3_HANDLE_NONE;
}

/*
 * Free all resources allocated for the specified libventrilo3 handle and
 * disconnect if currently connected.
 * @param v3h A v3_handle created by v3_init()
 */
void
v3_destroy(v3_handle v3h) {
    _v3_connection *v3c;

    _v3_enter(V3_HANDLE_NONE, __func__);

    if (_v3_handle_valid(v3h) != V3_OK) {
        _v3_leave(V3_HANDLE_NONE, __func__);
        return;
    }

    _v3_mutex_lock(V3_HANDLE_NONE);

    _v3_enter(v3h, __func__);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];

    _v3_close(v3h);
#ifdef HAVE_SPEEXDSP
    if (v3c->resampler.state) {
        speex_resampler_destroy(v3c->resampler.state);
    }
#endif
    _v3_coder_destroy(v3h, &v3c->encoder);
    _v3_data_destroy(v3h);
    _v3_event_clear(v3h);

    _v3_mutex_unlock(v3h);

    _v3_mutex_destroy(v3h);

    _v3_debug(v3h, V3_DBG_INFO, "destroyed handle: %i", v3h);

    _v3_leave(v3h, __func__);

    free(_v3_handles[v3h]);
    _v3_handles[v3h] = NULL;

    _v3_mutex_unlock(V3_HANDLE_NONE);

    _v3_leave(V3_HANDLE_NONE, __func__);
}

int
v3_password(v3_handle v3h, const char *password) {
    _v3_connection *v3c;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];
    strncpy(v3c->password, password, sizeof(v3c->password) - 1);

    _v3_leave(v3h, __func__);
    return V3_OK;
}

int
v3_phonetic(v3_handle v3h, const char *phonetic) {
    _v3_connection *v3c;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];
    strncpy(v3c->luser.phonetic, phonetic, sizeof(v3c->luser.phonetic) - 1);

    _v3_leave(v3h, __func__);
    return V3_OK;
}

int
v3_default_channel_path(v3_handle v3h, const char *path) {
    _v3_connection *v3c;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];
    strncpy(v3c->def_path, path, sizeof(v3c->def_path) - 1);

    _v3_leave(v3h, __func__);
    return V3_OK;
}

int
v3_default_channel_id(v3_handle v3h, uint16_t id) {
    _v3_connection *v3c;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];
    v3c->def_id = id;

    _v3_leave(v3h, __func__);
    return V3_OK;
}

int
_v3_handle_valid(v3_handle v3h) {
    int ret = V3_OK;

    pthread_mutex_lock(&_v3_handles_mutex);

    if (v3h == V3_HANDLE_NONE) {
        _v3_error(V3_HANDLE_NONE, "invalid handle: %i", v3h);
        ret = V3_FAILURE;
    } else if (v3h < V3_HANDLE_NONE || v3h >= V3_MAX_CONN) {
        _v3_error(V3_HANDLE_NONE, "handle out of range: %i", v3h);
        ret = V3_FAILURE;
    } else if (!_v3_handles[v3h]) {
        _v3_error(V3_HANDLE_NONE, "handle not initialized: %i", v3h);
        ret = V3_FAILURE;
    }

    pthread_mutex_unlock(&_v3_handles_mutex);

    return ret;
}

void
_v3_debug(v3_handle v3h, int level, const char *format, ...) {
    va_list args;
    char str[1024];
    char buf[1024];
    char time[32];
    struct timeval tv;
    struct tm *tm;
    size_t n;
    int ctr;

    if ((v3h == V3_HANDLE_NONE && !(_debug & level)) ||
        (v3h != V3_HANDLE_NONE && !(_v3_handles[v3h]->debug & level))) {
        return;
    }
    va_start(args, format);
    vsnprintf(str, sizeof(str), format, args);
    va_end(args);
    n = sizeof(buf) - 1;
    buf[n] = 0;
    *buf = 0;
    if ((v3h == V3_HANDLE_NONE && (_debug & V3_DBG_STACK)) ||
        (v3h != V3_HANDLE_NONE && (_v3_handles[v3h]->debug & V3_DBG_STACK))) {
        for (ctr = (v3h == V3_HANDLE_NONE) ? _stack : _v3_handles[v3h]->stack; ctr > 0; strncat(buf, "    ", n - strlen(buf)), --ctr);
    }
    strncat(buf, str, n - strlen(buf));
    gettimeofday(&tv, NULL);
    if (!(tm = localtime(&tv.tv_sec)) || !strftime(time, sizeof(time), "%T", tm)) {
#ifndef ANDROID
        fprintf(stderr, "lv3 (%2i): %s\n", v3h, buf);
#else
        __android_log_write(ANDROID_LOG_VERBOSE, "lv3", buf);
#endif
    } else {
#ifndef ANDROID
        fprintf(stderr, "lv3 (%2i): %s.%06lu: %s\n", v3h, time, tv.tv_usec, buf);
#else
        __android_log_print(ANDROID_LOG_VERBOSE, "lv3", " %s.%06lu: %s", time, tv.tv_usec, buf);
#endif
    }
}

int
v3_debug(v3_handle v3h, int level) {
    if (v3h == V3_HANDLE_NONE) {
        _debug = level;
        return V3_OK;
    }
    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_handles[v3h]->debug = level;

    return V3_OK;
}

void
_v3_error(v3_handle v3h, const char *format, ...) {
    _v3_connection *v3c;
    va_list args;

    va_start(args, format);
    if (v3h == V3_HANDLE_NONE) {
        vsnprintf(_error, sizeof(_error), format, args);
        _v3_debug(v3h, V3_DBG_ERROR, _error);
    } else if ((v3c = _v3_handles[v3h])) {
        vsnprintf(v3c->error, sizeof(v3c->error), format, args);
        _v3_debug(v3h, V3_DBG_ERROR, v3c->error);
    }
    va_end(args);
}

const char *
v3_error(v3_handle v3h) {
    if (v3h == V3_HANDLE_NONE || _v3_handle_valid(v3h) != V3_OK) {
        return _error;
    }

    return _v3_handles[v3h]->error;
}

void
_v3_enter(v3_handle v3h, const char *func) {
    _v3_connection *v3c;
    char buf[256];
    uint16_t debug = 0;

    if (v3h == V3_HANDLE_NONE) {
        debug = _debug;
    } else if ((v3c = _v3_handles[v3h])) {
        debug = v3c->debug;
    }
    if ((debug & V3_DBG_STACK)) {
        snprintf(buf, sizeof(buf), "---> %s()", func);
        _v3_debug(v3h, V3_DBG_STACK, buf);
    }
    if (v3h == V3_HANDLE_NONE) {
        ++_stack;
    } else if (v3c) {
        ++v3c->stack;
    }
}

void
_v3_leave(v3_handle v3h, const char *func) {
    _v3_connection *v3c;
    char buf[256];
    uint16_t debug = 0;

    if (v3h == V3_HANDLE_NONE) {
        if (--_stack < 0) {
            _stack = 0;
        }
        debug = _debug;
    } else if ((v3c = _v3_handles[v3h])) {
        if (--v3c->stack < 0) {
            v3c->stack = 0;
        }
        debug = v3c->debug;
    }
    if ((debug & V3_DBG_STACK)) {
        snprintf(buf, sizeof(buf), "<--- %s()", func);
        _v3_debug(v3h, V3_DBG_STACK, buf);
    }
}

void
_v3_packet(v3_handle v3h, const void *data, size_t len) {
    char buf[66];
    uint8_t chr, *p, *b;
    int ctr;

    if (!(_v3_handles[v3h]->debug & V3_DBG_PACKET)) {
        return;
    }
    memset(buf, ' ', sizeof(buf));

    _v3_debug(v3h, V3_DBG_PACKET, "PACKET: data length : %i", len);

    while (len) {
        for (ctr = 0, p = (uint8_t *)buf, b = p + 50; ctr < 16; ++ctr) {
            if (len) {
                --len;
                chr = *(typeof(chr) *)data++;
                *p++ = "0123456789abcdef"[chr >> 4];
                *p++ = "0123456789abcdef"[chr & 0xf];
                ++p;
                *b++ = (chr < ' ' || chr >= 0x7f) ? '.' : chr;
            } else {
                *p++ = ' ';
                *p++ = ' ';
                ++p;
                *b++ = 0;
            }
        }
        _v3_debug(v3h, V3_DBG_PACKET, "PACKET:     %.*s", sizeof(buf), buf);
    }
}

int
_v3_parse_server(const char *server, uint32_t *ip, uint16_t *port) {
    char *srv;
    char *sep;

    _v3_enter(V3_HANDLE_NONE, __func__);

    if (!server || !*server) {
        _v3_error(V3_HANDLE_NONE, "no server address specified; expected: 'hostname:port'");
        _v3_leave(V3_HANDLE_NONE, __func__);
        return V3_FAILURE;
    }
    srv = strdup(server);
    if (!(sep = strchr(srv, ':'))) {
        _v3_error(V3_HANDLE_NONE, "invalid server address format '%s'; expected: 'hostname:port'", srv);
        free(srv);
        _v3_leave(V3_HANDLE_NONE, __func__);
        return V3_FAILURE;
    }
    *sep++ = 0;
    if (!(*ip = _v3_resolv(srv))) {
        free(srv);
        _v3_leave(V3_HANDLE_NONE, __func__);
        return V3_FAILURE;
    }
    *port = atoi(sep);
    free(srv);
    _v3_debug(V3_HANDLE_NONE, V3_DBG_INFO, "server port: %hu", *port);

    _v3_leave(V3_HANDLE_NONE, __func__);
    return V3_OK;
}

uint32_t
_v3_resolv(const char *hostname) {
    struct in_addr ip;

    _v3_enter(V3_HANDLE_NONE, __func__);

    if (!hostname || !*hostname) {
        _v3_error(V3_HANDLE_NONE, "no hostname specified");
        _v3_leave(V3_HANDLE_NONE, __func__);
        return 0;
    }
    _v3_debug(V3_HANDLE_NONE, V3_DBG_INFO, "resolving hostname: '%s'", hostname);
#ifdef WIN32
    if ((ip.s_addr = inet_addr(hostname)) == INADDR_NONE)
#else
    if (!inet_aton(hostname, &ip))
#endif
    {
        struct hostent *hp;
        int res = 0;
#ifdef HAVE_GETHOSTBYNAME_R
        struct hostent hostbuf;
        size_t hstbuflen;
        char *tmphstbuf;
        int herr;

        hstbuflen = 1024;
        tmphstbuf = malloc(hstbuflen);

        while ((res = gethostbyname_r(hostname, &hostbuf, tmphstbuf, hstbuflen, &hp, &herr)) == ERANGE) {
            /* enlarge the buffer */
            hstbuflen *= 2;
            tmphstbuf = realloc(tmphstbuf, hstbuflen);
        }
        free(tmphstbuf);
#else
        /* if gethostbyname_r does not exist, assume gethostbyname is re-entrant */
        hp = gethostbyname(hostname);
#endif
        if (res || !hp || !hp->h_addr || hp->h_length <= 0) {
            _v3_error(V3_HANDLE_NONE, "hostname lookup failed for: '%s'", hostname);
            _v3_leave(V3_HANDLE_NONE, __func__);
            return 0;
        }
        ip.s_addr = *(typeof(ip.s_addr) *)*hp->h_addr_list;
    }
    _v3_debug(V3_HANDLE_NONE, V3_DBG_INFO, "resolved to ipv4 address: '%s'", inet_ntoa(ip));

    _v3_leave(V3_HANDLE_NONE, __func__);
    return ip.s_addr;
}

void
_v3_mutex_init(v3_handle v3h) {
    _v3_connection *v3c;
    pthread_mutexattr_t mta;

    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];

    if (!v3c->mutex) {
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
        _v3_debug(v3h, V3_DBG_MUTEX, "initializing connection mutex");
        v3c->mutex = malloc(sizeof(*v3c->mutex));
        pthread_mutex_init(v3c->mutex, &mta);
    }
    if (!v3c->event_mutex) {
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
        _v3_debug(v3h, V3_DBG_MUTEX, "initializing eventq mutex");
        v3c->event_mutex = malloc(sizeof(*v3c->event_mutex));
        v3c->event_cond = malloc(sizeof(*v3c->event_cond));
        pthread_mutex_init(v3c->event_mutex, &mta);
        pthread_cond_init(v3c->event_cond, (pthread_condattr_t *)&mta);
    }

    _v3_leave(v3h, __func__);
}

void
_v3_mutex_destroy(v3_handle v3h) {
    _v3_connection *v3c;

    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];

    if (v3c->event_mutex) {
        _v3_debug(v3h, V3_DBG_MUTEX, "destroying eventq mutex");
        pthread_cond_broadcast(v3c->event_cond);
        pthread_mutex_lock(v3c->event_mutex);
        pthread_mutex_unlock(v3c->event_mutex);
        pthread_cond_destroy(v3c->event_cond);
        pthread_mutex_destroy(v3c->event_mutex);
        free(v3c->event_cond);
        free(v3c->event_mutex);
        v3c->event_cond = NULL;
        v3c->event_mutex = NULL;
    }
    if (v3c->mutex) {
        _v3_debug(v3h, V3_DBG_MUTEX, "destroying connection mutex");
        pthread_mutex_destroy(v3c->mutex);
        free(v3c->mutex);
        v3c->mutex = NULL;
    }

    _v3_leave(v3h, __func__);
}

int
_v3_mutex_lock(v3_handle v3h) {
    _v3_connection *v3c;
    int ret = V3_FAILURE;

    _v3_enter(v3h, __func__);

    if (v3h == V3_HANDLE_NONE) {
        ret = pthread_mutex_lock(&_v3_handles_mutex);
        _v3_debug(v3h, V3_DBG_MUTEX, "locked handles mutex: %i: %s", ret, strerror(ret));
        ret = V3_OK;
    } else if ((v3c = _v3_handles[v3h])) {
        ret = pthread_mutex_lock(v3c->mutex);
        _v3_debug(v3h, V3_DBG_MUTEX, "locked connection mutex: %i: %s", ret, strerror(ret));
        ret = V3_OK;
    }

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_mutex_unlock(v3_handle v3h) {
    _v3_connection *v3c;
    int ret = V3_FAILURE;

    _v3_enter(v3h, __func__);

    if (v3h == V3_HANDLE_NONE) {
        ret = pthread_mutex_unlock(&_v3_handles_mutex);
        _v3_debug(v3h, V3_DBG_MUTEX, "unlocked handles mutex: %i: %s", ret, strerror(ret));
        ret = V3_OK;
    } else if ((v3c = _v3_handles[v3h])) {
        ret = pthread_mutex_unlock(v3c->mutex);
        _v3_debug(v3h, V3_DBG_MUTEX, "unlocked connection mutex: %i: %s", ret, strerror(ret));
        ret = V3_OK;
    }

    _v3_leave(v3h, __func__);
    return ret;
}

char *
_v3_strncpy(char *dest, const char *src, size_t n) {
    uint8_t *_dest = (uint8_t *)dest;
    uint8_t *_src = (uint8_t *)src;

    while (n--) {
        if ((*_dest++ = (*_src && *_src < ' ') ? ' ' : *_src)) {
            ++_src;
        }
    }

    return dest;
}

