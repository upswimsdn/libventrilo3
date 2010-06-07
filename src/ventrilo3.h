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

#ifndef _VENTRILO3_H
#define _VENTRILO3_H

#include <stdint.h>
#include <sys/time.h> /* struct timeval */

#define V3_OK       0
#define V3_FAILURE  1
#define V3_MALFORM  2

#define V3_BLOCK    1
#define V3_NONBLOCK 0

#define V3_NONE     0
#define V3_CRITICAL 1
#define V3_WARNING  2
#define V3_NOTICE   3
#define V3_INFO     4
#define V3_DEBUG    5

#define V3_DBG_NONE     0
#define V3_DBG_ERROR    1
#define V3_DBG_STACK    1 << 1
#define V3_DBG_INFO     1 << 2
#define V3_DBG_STATUS   1 << 3
#define V3_DBG_EVENT    1 << 4
#define V3_DBG_MESSAGE  1 << 5
#define V3_DBG_PACKET   1 << 6
#define V3_DBG_MUTEX    1 << 7
#define V3_DBG_MEMORY   1 << 8
#define V3_DBG_ALL      0xffff

#define V3_HANDLE_NONE  (v3_handle)-1

typedef int16_t v3_handle;

typedef struct v3_codec {
    int8_t      id;
    int8_t      format;
    uint32_t    framesize;
    uint32_t    rate;
    uint8_t     quality;
    char        name[128];
} v3_codec;
extern const v3_codec v3_codecs[];

typedef struct v3_user {
    uint16_t    id;
    uint16_t    channel;
    uint16_t    bitfield;
    uint16_t    rank_id;

    char        name[32];
    char        phonetic[32];
    char        comment[128];
    char        url[128];
    char        integration[128];

    uint8_t     accept_pages;
    uint8_t     accept_u2u;
    uint8_t     accept_chat;
    uint8_t     allow_recording;

    uint8_t     is_global_mute;
    uint8_t     is_channel_mute;
    uint8_t     is_transmitting;
    uint8_t     is_guest;
    uint16_t    phantom_owner;

    void *      gsm_decoder;
    void *      speex_decoder;

    uint16_t    volume;

    void *      next;
} v3_user;

typedef struct v3_channel {
    uint16_t    id;
    uint16_t    parent;
    uint8_t     unknown_1;
    uint8_t     password_protected;
    uint16_t    unknown_2;
    uint16_t    allow_recording;
    uint16_t    allow_cross_channel_transmit;
    uint16_t    allow_paging;
    uint16_t    allow_wave_file_binds;
    uint16_t    allow_tts_binds;
    uint16_t    allow_u2u_transmit;
    uint16_t    disable_guest_transmit;
    uint16_t    disable_sound_events;
    uint16_t    voice_mode;
    uint16_t    transmit_time_limit;
    uint16_t    allow_phantoms;
    uint16_t    max_clients;
    uint16_t    allow_guests;
    uint16_t    inactive_exempt;
    uint16_t    protect_mode;
    uint16_t    transmit_rank_level;
    uint16_t    channel_codec;
    uint16_t    channel_format;
    uint16_t    allow_voice_target;
    uint16_t    allow_command_target;
    char        name[32];
    char        phonetic[32];
    char        comment[128];

    void *      next;
} v3_channel;

typedef struct v3_rank {
    uint16_t    id;
    uint16_t    level;
    char        name[16];
    char        description[64];

    void *      next;
} v3_rank;

typedef struct v3_perms {
    uint16_t account_id;
    uint16_t replace_owner_id;
    uint8_t hash_password[32];
    uint16_t rank_id;
    uint16_t unknown_perm_1;
    uint8_t lock_acct;
    uint8_t in_reserve_list;
    uint8_t dupe_ip;
    uint8_t switch_chan;
    uint16_t dfl_chan;
    uint8_t unknown_perm_2;
    uint8_t unknown_perm_3;
    uint8_t recv_bcast;
    uint8_t add_phantom;
    uint8_t record;
    uint8_t recv_complaint;
    uint8_t send_complaint;
    uint8_t inactive_exempt;
    uint8_t unknown_perm_4;
    uint8_t unknown_perm_5;
    uint8_t srv_admin;
    uint8_t add_user;
    uint8_t del_user;
    uint8_t ban_user;
    uint8_t kick_user;
    uint8_t move_user;
    uint8_t assign_chan_admin;
    uint8_t edit_rank;
    uint8_t edit_motd;
    uint8_t edit_guest_motd;
    uint8_t issue_rcon_cmd;
    uint8_t edit_voice_target;
    uint8_t edit_command_target;
    uint8_t assign_rank;
    uint8_t assign_reserved;
    uint8_t unknown_perm_6;
    uint8_t unknown_perm_7;
    uint8_t unknown_perm_8;
    uint8_t unknown_perm_9;
    uint8_t unknown_perm_10;
    uint8_t bcast;
    uint8_t bcast_lobby;
    uint8_t bcast_user;
    uint8_t bcast_x_chan;
    uint8_t send_tts_bind;
    uint8_t send_wav_bind;
    uint8_t send_page;
    uint8_t send_comment;
    uint8_t set_phon_name;
    uint8_t gen_comment_snds;
    uint8_t event_snds;
    uint8_t mute_glbl;
    uint8_t mute_other;
    uint8_t glbl_chat; 
    uint8_t start_priv_chat; 
    uint8_t unknown_perm_11; 
    uint8_t eq_out; 
    uint8_t unknown_perm_12; 
    uint8_t unknown_perm_13; 
    uint8_t unknown_perm_14; 
    uint8_t see_guest; 
    uint8_t see_nonguest; 
    uint8_t see_motd; 
    uint8_t see_srv_comment; 
    uint8_t see_chan_list; 
    uint8_t see_chan_comment; 
    uint8_t see_user_comment; 
    uint8_t unknown_perm_15; 
} v3_perms;

typedef struct v3_account {
    v3_perms    perms;
    char        username[32];
    char        owner[32];
    char        notes[256];
    char        lock_reason[128];
    int         chan_admin_count;
    uint16_t    chan_admin[32];
    int         chan_auth_count;
    uint16_t    chan_auth[32];

    void *      next;
} v3_account;

typedef struct v3_event {
} v3_event;

/*
 * Debug functions
 */
int             v3_debug(v3_handle v3h, int level);
const char *    v3_error(v3_handle v3h);

/*
 * Functions to initialize a connection and perform mainloop processing
 */
v3_handle       v3_init(const char *server, const char *username);
v3_handle       v3_find_handle(const char *server, const char *username);
int             v3_destroy(v3_handle v3h);

int             v3_set_password(v3_handle v3h, const char *password);
int             v3_set_phonetic(v3_handle v3h, const char *phonetic);
int32_t         v3_set_default_channel_path(v3_handle v3h, char *path);
int32_t         v3_set_default_channel_id(v3_handle v3h, int32_t id);

int             v3_login(v3_handle v3h);
int             v3_login_cancel(v3_handle v3h);
int32_t         v3_iterate(v3_handle v3h, int8_t block, struct timeval *tv);
int32_t         v3_is_loggedin(v3_handle v3h);

/*
 * Retreive server information
 */
int32_t         v3_get_luser_id(v3_handle v3h);
int32_t         v3_get_luser_channel(v3_handle v3h);
int32_t         v3_get_max_clients(v3_handle v3h);
uint64_t        v3_get_bytes_sent(v3_handle v3h);
uint64_t        v3_get_bytes_recv(v3_handle v3h);
uint32_t        v3_get_pkts_sent(v3_handle v3h);
uint32_t        v3_get_pkts_recv(v3_handle v3h);

v3_user *       get_user(v3_handle v3h, int32_t id);
int32_t         v3_free_user(v3_user *user);

v3_channel *    get_channel(v3_handle v3h, int32_t id);
char *          get_channel_path(v3_handle v3h, int32_t id, char *result, int size);
int32_t         v3_free_channel(v3_channel *channel);

v3_rank *       get_rank(v3_handle v3h, int32_t id);
int32_t         v3_free_rank(v3_rank *rank);

v3_account *    get_account(v3_handle v3h, int32_t id);
int32_t         v3_free_account(v3_account *account);

const v3_perms *v3_get_permissions(v3_handle v3h);
const v3_codec *v3_get_channel_codec(v3_handle v3h, int32_t id);

int32_t         v3_channel_requires_password(v3_handle v3h, int32_t id);

int32_t         v3_get_codec_rate(int16_t codec, int16_t format);
const v3_codec *v3_get_codec(int16_t codec, int16_t format);

int16_t         v3_user_count(v3_handle v3h);
int16_t         v3_channel_count(v3_handle v3h);
int16_t         v3_rank_count(v3_handle v3h);
int16_t         v3_account_count(v3_handle v3h);

/*
 * Event processing functions
 */
int32_t         v3_check_events(v3_handle v3h);
v3_event *      v3_get_event(v3_handle v3h);
int32_t         v3_free_event(v3_event *event);
int32_t         v3_clear_events(v3_handle v3h);

/*
 * Audio signal processing functions
 */
void            v3_set_volume_master(int16_t level);
int16_t         v3_get_volume_master(void);
void            v3_set_volume_server_master(v3_handle v3h, int16_t level);
int16_t         v3_get_volume_server_master(v3_handle v3h);
void            v3_set_volume_user(v3_handle v3h, int32_t id, int16_t level);
int16_t         v3_get_volume_user(v3_handle v3h, int32_t id);

/*
 * Normal user functions
 */
void            v3_set_text(v3_handle v3h, char *comment, char *url, char *integration_text, uint8_t silent);

void            v3_join_chat(v3_handle v3h);
void            v3_leave_chat(v3_handle v3h);
void            v3_send_chat_message(v3_handle v3h, char *message);

void            v3_start_privchat(v3_handle v3h, int32_t id);
void            v3_end_privchat(v3_handle v3h, int32_t id);
void            v3_send_privchat_message(v3_handle v3h, int32_t id, char *message);
void            v3_send_privchat_away(v3_handle v3h, int32_t id);
void            v3_send_privchat_back(v3_handle v3h, int32_t id);

void            v3_change_channel(v3_handle v3h, int32_t id, char *password);

void            v3_start_audio(v3_handle v3h, int32_t dest);
void            v3_send_audio(v3_handle v3h, int32_t dest, uint32_t rate, uint8_t *pcm, uint32_t length);
void            v3_stop_audio(v3_handle v3h);

void            v3_phantom_add(v3_handle v3h, int32_t id);
void            v3_phantom_remove(v3_handle v3h, int32_t id);

void            v3_set_luser_option(v3_handle v3h, uint8_t type, uint8_t value);

/*
 * Admin user functions
 */
void            v3_admin_login(v3_handle v3h, char *password);
void            v3_admin_logout(v3_handle v3h);

void            v3_admin_boot(v3_handle v3h, int8_t type, int32_t id, char *reason);
void            v3_force_move_user(v3_handle v3h, int32_t id, int32_t dest);

void            v3_account_open(v3_handle v3h);
void            v3_account_close(v3_handle v3h);
void            v3_account_add(v3_handle v3h, v3_account *account);
void            v3_account_update(v3_handle v3h, v3_account *account);
void            v3_account_remove(v3_handle v3h, int32_t id);
void            v3_account_change_owner(v3_handle v3h, int32_t accountid, int32_t ownerid);

void            v3_channel_add(v3_handle v3h, int32_t id, v3_channel *channel);
void            v3_channel_update(v3_handle v3h, int32_t id, v3_channel *channel);
void            v3_channel_remove(v3_handle v3h, int32_t id);

#endif // _VENTRILO3_H

