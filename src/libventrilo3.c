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

#ifdef ANDROID
# include <android/log.h>
#else
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <netinet/in.h> /* struct in_addr */
#include <netdb.h> /* struct hostent */
#include <errno.h> /* ERANGE */
#include <arpa/inet.h> /* inet_aton / inet_ntoa */

#define __USE_UNIX98
#include <pthread.h>
#undef __USE_UNIX98

#include "libventrilo3.h"

#define false   0
#define true    1

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

    if (!username || !*username) {
        _v3_error(V3_HANDLE_NONE, "no username specified");
        return V3_HANDLE_NONE;
    }
    if (_v3_parse_server_info(server, &ip, &port) != V3_OK) {
        return V3_HANDLE_NONE;
    }

    pthread_mutex_lock(&_v3_handles_mutex);

    for (v3h = 0; v3h < V3_MAX_CONN && _v3_handles[v3h]; v3h++);

    if (v3h >= V3_MAX_CONN) {
        pthread_mutex_unlock(&_v3_handles_mutex);
        _v3_error(V3_HANDLE_NONE, "maximum number of open handles reached: %i", V3_MAX_CONN);
        return V3_HANDLE_NONE;
    }
    _v3_handles[v3h] = malloc(sizeof(_v3_connection));

    pthread_mutex_unlock(&_v3_handles_mutex);

    memset(_v3_handles[v3h], 0, sizeof(_v3_connection));
    v3c = _v3_handles[v3h];
    v3c->ip = ip;
    v3c->port = port;
    memcpy(v3c->luser.name, username, sizeof(v3c->luser.name) - 1);
    _v3_debug(V3_HANDLE_NONE, V3_DBG_INFO, "username: '%s'", v3c->luser.name);

    return v3h;
}

v3_handle
v3_find_handle(const char *server, const char *username) {
    v3_handle v3h;
    _v3_connection *v3c;
    uint32_t ip;
    uint16_t port;

    if (!username || !*username) {
        _v3_error(V3_HANDLE_NONE, "no username specified");
        return V3_HANDLE_NONE;
    }
    if (_v3_parse_server_info(server, &ip, &port) != V3_OK) {
        return V3_HANDLE_NONE;
    }

    pthread_mutex_lock(&_v3_handles_mutex);

    for (v3c = _v3_handles[v3h = 0]; v3h < V3_MAX_CONN && v3c; v3c = _v3_handles[++v3h]) {
        if (v3c->ip == ip && v3c->port == port && !strncmp(v3c->luser.name, username, sizeof(v3c->luser.name))) {
            pthread_mutex_unlock(&_v3_handles_mutex);
            return v3h;
        }
    }

    pthread_mutex_unlock(&_v3_handles_mutex);

    _v3_error(V3_HANDLE_NONE, "handle for server '%s' username '%s' not found", server, username);

    return V3_HANDLE_NONE;
}

/*
 * Free all resources allocated for the specified libventrilo3 handle and
 * disconnect if currently connected.
 * @param v3h A v3_handle created by v3_init()
 */
int
v3_destroy(v3_handle v3h) {
    if (_v3_check_handle(v3h) != V3_OK) {
        return V3_FAILURE;
    }

    pthread_mutex_lock(&_v3_handles_mutex);

    // TODO: safely clean up here
    free(_v3_handles[v3h]);
    _v3_handles[v3h] = NULL;

    pthread_mutex_unlock(&_v3_handles_mutex);

    return V3_OK;
}

/*
 *
 */

int
_v3_check_handle(v3_handle v3h) {
    if (v3h == V3_HANDLE_NONE) {
        _v3_error(V3_HANDLE_NONE, "invalid handle");
        return V3_FAILURE;
    }
    if (v3h < V3_HANDLE_NONE || v3h >= V3_MAX_CONN) {
        _v3_error(V3_HANDLE_NONE, "handle out of range");
        return V3_FAILURE;
    }
    if (!_v3_handles[v3h]) {
        _v3_error(V3_HANDLE_NONE, "handle not initialized");
        return V3_FAILURE;
    }

    return V3_OK;
}

void
_v3_debug(v3_handle v3h, int level, const char *format, ...) {
    va_list args;
    char str[1024] = "";
    char buf[1024] = "";
    char stamp[32] = "";
    struct timeval tv;
    struct tm *tm;

    if ((v3h == V3_HANDLE_NONE && !(_debug & level)) ||
        (v3h != V3_HANDLE_NONE && !(_v3_handles[v3h]->debug & level))) {
        return;
    }
    va_start(args, format);
    vsnprintf(str, sizeof(str) - 1, format, args);
    va_end(args);
    if (v3h != V3_HANDLE_NONE) {
        uint16_t stack = _v3_handles[v3h]->stack;
        int ctr;
        for (ctr = 0; ctr < stack; ctr++) {
            strncat(buf, "    ", sizeof(buf)-(strlen(buf)+1));
        }
    }
    strncat(buf, str, sizeof(buf)-(strlen(buf)+1));
    gettimeofday(&tv, NULL);
    if (!(tm = localtime(&tv.tv_sec)) || !strftime(stamp, sizeof(stamp), "%T", tm)) {
#ifndef ANDROID
        fprintf(stderr, "libventrilo3 (%i): %s\n", v3h, buf);
#else
        __android_log_write(ANDROID_LOG_VERBOSE, "libventrilo3", buf);
#endif
    } else {
#ifndef ANDROID
        fprintf(stderr, "libventrilo3 (%i): %s.%06lu: %s\n", v3h, stamp, tv.tv_usec, buf);
#else
        __android_log_print(ANDROID_LOG_VERBOSE, "libventrilo3", " %s.%06lu: %s", stamp, tv.tv_usec, buf);
#endif
    }
}

int
v3_debug(v3_handle v3h, int level) {
    if (v3h == V3_HANDLE_NONE) {
        _debug = level;
        return V3_OK;
    }
    if (_v3_check_handle(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_handles[v3h]->debug = level;

    return V3_OK;
}

void
_v3_error(v3_handle v3h, const char *format, ...) {
    va_list args;
    _v3_connection *v3c;

    va_start(args, format);
    if (v3h == V3_HANDLE_NONE) {
        vsnprintf(_error, sizeof(_error) - 1, format, args);
        _v3_debug(v3h, V3_DBG_ERROR, _error);
    } else if ((v3c = _v3_handles[v3h])) {
        vsnprintf(v3c->error, sizeof(v3c->error) - 1, format, args);
        _v3_debug(v3h, V3_DBG_ERROR, v3c->error);
    }
    va_end(args);
}

const char *
v3_error(v3_handle v3h) {
    if (v3h == V3_HANDLE_NONE || _v3_check_handle(v3h) != V3_OK) {
        return _error;
    }

    return _v3_handles[v3h]->error;
}

int
_v3_parse_server_info(const char *server, uint32_t *ip, uint16_t *port) {
    char *srv;
    char *sep;

    if (!server || !*server) {
        _v3_error(V3_HANDLE_NONE, "no server specified; expected: 'hostname:port'");
        return V3_FAILURE;
    }
    srv = strdup(server);
    if (!(sep = strchr(srv, ':'))) {
        _v3_error(V3_HANDLE_NONE, "invalid server name format of '%s'; expected: 'hostname:port'", srv);
        free(srv);
        return V3_FAILURE;
    }
    *sep++ = 0;
    if (!(*ip = _v3_resolv(srv))) {
        _v3_error(V3_HANDLE_NONE, "unable to resolve hostname: '%s'", srv);
        free(srv);
        return V3_FAILURE;
    }
    *port = atoi(sep);
    free(srv);
    _v3_debug(V3_HANDLE_NONE, V3_DBG_INFO, "parsed server: '%hu.%hu.%hu.%hu:%hu'",
            (*ip >> 24) & 0xff,
            (*ip >> 16) & 0xff,
            (*ip >>  8) & 0xff,
            *ip & 0xff,
            *port);

    return V3_OK;
}

uint32_t
_v3_resolv(const char *hostname) {
    struct in_addr ip;

    if (!hostname || !*hostname) {
        _v3_error(V3_HANDLE_NONE, "no hostname specified");
        return 0;
    }
    _v3_debug(V3_HANDLE_NONE, V3_DBG_INFO, "looking up hostname: '%s'", hostname);
    if (!inet_aton(hostname, &ip)) {
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
        /* if gethostbyname_r does not exist, assume that gethostbyname is re-entrant */
        hp = gethostbyname(hostname);
#endif
        if (res || !hp || hp->h_length < 1) {
            _v3_error(V3_HANDLE_NONE, "hostname lookup failed for: '%s'", hostname);
            return 0;
        }
        memcpy(&ip.s_addr, hp->h_addr_list[0], sizeof(ip.s_addr));
    }
    _v3_debug(V3_HANDLE_NONE, V3_DBG_INFO, "hostname resolved to ipv4 address: %s", inet_ntoa(ip));

    return ip.s_addr;
}


