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
_v3_connected(v3_handle v3h) {
    const char func[] = "_v3_connected";

    _v3_connection *v3c;
    int connected;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    connected = (v3c->sd >= 0);
    _v3_debug(v3h, V3_DBG_INFO, "%sconnected with socket descriptor: %i", (!connected) ? "not " : "", v3c->sd);

    _v3_leave(v3h, func);
    return connected;
}

int
_v3_canceling(v3_handle v3h) {
    const char func[] = "_v3_canceling";

    _v3_connection *v3c;
    int cancel;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    cancel = v3c->cancel;
    _v3_debug(v3h, V3_DBG_INFO, "login %scanceled", (!cancel) ? "not " : "");

    _v3_leave(v3h, func);
    return cancel;
}

void
_v3_close(v3_handle v3h) {
    const char func[] = "_v3_close";

    _v3_connection *v3c;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (_v3_connected(v3h)) {
        close(v3c->sd);
        v3c->sd = -1;
    }

    _v3_leave(v3h, func);
}

int
_v3_connect(v3_handle v3h, int udp) {
    const char func[] = "_v3_connect";

    _v3_connection *v3c;
    struct linger ling = { 1, 1 };
    struct sockaddr_in sa;

    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if ((v3c->sd = socket(AF_INET, SOCK_DGRAM, (udp) ? IPPROTO_UDP : IPPROTO_TCP)) < 0) {
        _v3_error(v3h, "failed to create %s socket: %s", (udp) ? "udp" : "tcp", strerror(errno));
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }
    setsockopt(v3c->sd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
    if (!udp) {
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = v3c->ip;
        sa.sin_port = htons(v3c->port);
        if (connect(v3c->sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            _v3_error(v3h, "failed to connect: %s", strerror(errno));
            _v3_close(v3h);
            _v3_leave(v3h, func);
            return V3_FAILURE;
        }
        _v3_debug(v3h, V3_DBG_INFO, "tcp connected: '%s:%hu'", inet_ntoa(sa.sin_addr), v3c->port);
    }

    _v3_leave(v3h, func);
    return V3_OK;
}

int
v3_login(v3_handle v3h) {
    const char func[] = "v3_login";

    _v3_connection *v3c;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];
    if (v3c->connecting) {
        _v3_mutex_unlock(v3h);
        _v3_error(v3h, "already connecting; try cancel first");
        _v3_leave(v3h, func);
        return V3_FAILURE;
    } else if (_v3_connected(v3h)) {
        _v3_mutex_unlock(v3h);
        _v3_error(v3h, "already connected; try logout first");
        _v3_leave(v3h, func);
        return V3_FAILURE;
    } else if (_v3_canceling(v3h)) {
        _v3_mutex_unlock(v3h);
        _v3_error(v3h, "cancel in progress");
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }
    v3c->connecting = true;
    if (_v3_handshake(v3h) != V3_OK) {
        v3c->connecting = false;
        v3c->cancel = false;
        _v3_mutex_unlock(v3h);
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }
    v3c->connecting = false;
    v3c->sd = 3;

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, func);
    return V3_OK;
}

int
v3_login_cancel(v3_handle v3h) {
    const char func[] = "v3_login_cancel";

    _v3_connection *v3c;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, func);

    v3c = _v3_handles[v3h];
    if (v3c->connecting && !_v3_canceling(v3h)) {
        _v3_debug(v3h, V3_DBG_INFO, "canceling login");
        v3c->cancel = true;
        _v3_close(v3h);
    } else if (!v3c->connecting && _v3_connected(v3h)) {
        _v3_error(v3h, "already connected; try logout");
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }

    _v3_leave(v3h, func);
    return V3_OK;
}





















