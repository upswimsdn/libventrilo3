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
 * Mangler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mangler.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Functions to initialize a connection and perform mainloop processing
 */
v3_handle       v3_init(char *server, char *username);
v3_handle       v3_find_handle(char *server, char *username);

int32_t         v3_set_password(v3_handle v3h, char *password);
int32_t         v3_set_phonetic(v3_handle v3h, char *phonetic);
int32_t         v3_set_default_channel_path(v3_handle v3h, char *path);
int32_t         v3_set_default_channel_id(v3_handle v3h, int32_t id);

int32_t         v3_login(v3_handle v3h);
int32_t         v3_login_cancel(v3_handle v3h);
int32_t         v3_iterate(v3_handle v3h, int8_t block, struct timeval *tv);
int32_t         v3_is_loggedin(v3_handle v3h);


/*
 * Retreive server information
 */
int32_t         v3_get_luser_id(v3_handle v3h);
int32_t         v3_get_luser_channel(v3_handle v3h);
int32_t         v3_get_max_clients(v3_handle v3h);
uint32_t        v3_get_bytes_sent(v3_handle v3h);
uint32_t        v3_get_bytes_recv(v3_handle v3h);
uint32_t        v3_get_pkts_sent(v3_handle v3h);
uint32_t        v3_get_pkts_recv(v3_handle v3h);

v3_user *       get_user(v3_handle v3h, int32_t id);
int32_t         v3_free_user(v3_user *user);

v3_channel *    get_channel(v3_handle v3h, int32_t id);
char            *get_channel_path(v3_handle v3h, int32_t id, char *result, int size);
int32_t         v3_free_channel(v3_channel *channel);

v3_rank *       get_rank(v3_handle v3h, int32_t id);
int32_t         v3_free_rank(v3_rank *rank);

v3_account *    get_account(v3_handle v3h, int32_t id);
int32_t         v3_free_account(v3_account *account);

const v3_perms *v3_get_permissions(v3_handle v3h, void);
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

void            v3_start_audio(v3_handle v3h, int32_t send type, int32_t dest);
void            v3_send_audio(v3_handle v3h, int32_t send_type, int32_t dest, uint32_t rate, uint8_t *pcm, uint32_t length);
void            v3_stop_audio(v3_handle v3h);

void            v3_phantom_add(v3_handle v3h, int32_t id);
void            v3_phantom_remove(v3_handle v3h, int32_t id);

void            v3_set_server_option(v3_handle v3h, uint8_t type, uint8_t value);

/*
 * Admin user functions
 */

void            v3_admin_login(v3_handle v3h, char *password);
void            v3_admin_logout(v3_handle v3h);

void            v3_admin_boot(v3_handle v3h, int8_t type, int32_t id, char *reason);
void            v3_force_move_user(v3_handle v3h, int32_t id, int32_t dest);

void            v3_accountlist_open(v3_handle v3h);
void            v3_accountlist_close(v3_handle v3h);
void            v3_account_add(v3_handle v3h, v3_account *account);
void            v3_account_update(v3_handle v3h, v3_account *account);
void            v3_account_remove(v3_handle v3h, int32_t id);
void            v3_account_changeowner(v3_handle v3h, int32_t accountid, int32_t ownerid);

void            v3_channel_add(v3_handle v3h, int32_t id, v3_channel *channel);
void            v3_channel_update(v3_handle v3h, int32_t id, v3_channel *channel);
void            v3_channel_remove(v3_handle v3h, int32_t id);
