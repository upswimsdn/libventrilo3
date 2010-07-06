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

void
_v3_decoder_destroy(v3_handle v3h, v3_coder *coder) {
    const char func[] = "_v3_decoder_destroy";

    _v3_enter(v3h, func);

    if (coder->state) {
        _v3_debug(v3h, V3_DBG_MEMORY, "releasing decoder state");
        switch (coder->index) {
#if HAVE_GSM
          case 0:
            gsm_destroy(coder->state);
            _v3_debug(v3h, V3_DBG_MEMORY, "released gsm state");
            break;
#endif
#if HAVE_CELT
          case 1:
          case 2:
            celt_decoder_destroy(coder->state);
            celt_mode_destroy(coder->mode);
            coder->mode = NULL;
            _v3_debug(v3h, V3_DBG_MEMORY, "released celt state");
            break;
#endif
#if HAVE_SPEEX
          case 3:
            speex_decoder_destroy(coder->state);
            _v3_debug(v3h, V3_DBG_MEMORY, "released speex state");
            break;
#endif
          default:
            break;
        }
        coder->state = NULL;
    }

    _v3_leave(v3h, func);
}

int
_v3_audio_decode(
        v3_handle v3h,
        /* encoded input */
        int16_t index,
        int16_t format,
        v3_coder *coder,
        uint8_t *data,
        int32_t datalen,
        /* pcm output */
        uint8_t *pcm,
        uint32_t *pcmlen,
        /* optional args */
        uint32_t *rate,
        uint8_t channels) {
    const char func[] = "_v3_audio_decode";

    uint32_t maxpcmlen;
    int ret = V3_OK;

    _v3_enter(v3h, func);

    if (!coder || !data || !datalen || !pcm || !pcmlen || (pcmlen && !*pcmlen)) {
        _v3_leave(v3h, func);
        return V3_FAILURE;
    }

    maxpcmlen = *pcmlen;
    *pcmlen = 0;
    channels = (channels == 2) ? 2 : 1;

    if (coder->state && (coder->index != index || coder->format != format)) {
        _v3_decoder_destroy(v3h, coder);
    }
    switch (index) {
#if HAVE_GSM
      case 0:
        {
            const v3_codec *codec = v3_codec_get(index, format);
            int opt = 1;

            if (!coder->state) {
                if (!(coder->state = gsm_create())) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to create gsm decoder");
                    ret = V3_FAILURE;
                    break;
                }
                _v3_debug(v3h, V3_DBG_MEMORY, "initialized gsm decoder state");
                gsm_option(coder->state, GSM_OPT_WAV49, &opt);
                coder->index = index;
                coder->format = format;
            }
            while (datalen - 65 >= 0 && *pcmlen + codec->framesize <= maxpcmlen) {
                if (gsm_decode(coder->state, data, (void *)pcm) ||
                    gsm_decode(coder->state, data+33, (void *)pcm+(codec->framesize/2))) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to decode gsm frame");
                }
                data    += 65;
                datalen -= 65;
                pcm     += codec->framesize;
                *pcmlen += codec->framesize;
            }
            if (rate) {
                *rate = codec->rate;
            }
        }
        break;
#endif
#if HAVE_CELT
      case 1:
      case 2:
        {
            const v3_codec *codec = v3_codec_get(index, format);
            uint8_t framesize;

            if (!coder->state || channels != coder->channels) {
                if (coder->state) {
                    celt_decoder_destroy(coder->state);
                    coder->state = NULL;
                    _v3_debug(v3h, V3_DBG_MEMORY, "released celt state");
                }
                if (coder->mode) {
                    celt_mode_destroy(coder->mode);
                    coder->mode = NULL;
                    _v3_debug(v3h, V3_DBG_MEMORY, "released celt mode");
                }
                if (!(coder->mode = celt_mode_create(44100, codec->framesize / sizeof(int16_t), NULL))) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to create celt mode");
                    ret = V3_FAILURE;
                    break;
                }
                if (!(coder->state = celt_decoder_create(coder->mode, channels, NULL))) {
                    if (coder->mode) {
                        celt_mode_destroy(coder->mode);
                        coder->mode = NULL;
                    }
                    _v3_debug(v3h, V3_DBG_INFO, "failed to create celt decoder");
                    ret = V3_FAILURE;
                    break;
                }
                _v3_debug(v3h, V3_DBG_MEMORY, "initialized celt decoder state");
                coder->index = index;
                coder->format = format;
                coder->channels = channels;
            }
            while (datalen) {
                framesize = *data++;
                if (!framesize || datalen - framesize - 1 < 0 || *pcmlen + codec->framesize * channels > maxpcmlen) {
                    _v3_debug(v3h, V3_DBG_INFO, "received a malformed celt packet");
                    ret = V3_MALFORM;
                    break;
                }
                if (celt_decode(coder->state, (void *)data, framesize, (void *)pcm) != CELT_OK) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to decode celt frame");
                }
                data    += framesize;
                datalen -= framesize + 1;
                pcm     += codec->framesize * channels;
                *pcmlen += codec->framesize * channels;
            }
            if (rate) {
                *rate = codec->rate;
            }
        }
        break;
#endif
#if HAVE_SPEEX
      case 3:
        {
            const v3_codec *codec = v3_codec_get(index, format);
            uint16_t framesize;
            SpeexBits bits;

            if (!coder->state) {
                switch (codec->rate) {
                  case 8000:
                    coder->state = speex_decoder_init(&speex_nb_mode);
                    break;
                  case 16000:
                    coder->state = speex_decoder_init(&speex_wb_mode);
                    break;
                  case 32000:
                    coder->state = speex_decoder_init(&speex_uwb_mode);
                    break;
                }
                if (!coder->state) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to create speex decoder");
                    ret = V3_FAILURE;
                    break;
                }
                _v3_debug(v3h, V3_DBG_MEMORY, "initialized speex decoder state");
                coder->index = index;
                coder->format = format;
            }
            if (datalen - 4 < 0) {
                _v3_debug(v3h, V3_DBG_INFO, "received a malformed speex packet");
                ret = V3_MALFORM;
                break;
            }
            data    += 4;
            datalen -= 4;
            speex_bits_init(&bits);
            while (datalen) {
                framesize = ntohs(*(uint16_t *)data);
                if (!framesize || datalen - framesize - 2 < 0 || *pcmlen + codec->framesize > maxpcmlen) {
                    _v3_debug(v3h, V3_DBG_INFO, "received a malformed speex packet");
                    ret = V3_MALFORM;
                    break;
                }
                data += 2;
                speex_bits_read_from(&bits, (void *)data, framesize);
                speex_decode_int(coder->state, &bits, (void *)pcm);
                speex_bits_reset(&bits);
                data    += framesize;
                datalen -= framesize + 2;
                pcm     += codec->framesize;
                *pcmlen += codec->framesize;
            }
            speex_bits_destroy(&bits);
            if (rate) {
                *rate = codec->rate;
            }
        }
        break;
#endif
      default:
        _v3_debug(v3h, V3_DBG_INFO, "unsupported codec: index: %i | format: %i", index, format);
        ret = V3_FAILURE;
        break;
    }

    _v3_leave(v3h, func);
    return ret;
}

const v3_codec *
v3_codec_get(int16_t index, int16_t format) {
    int ctr;

    for (ctr = 0; v3_codecs[ctr].index >= 0 && (v3_codecs[ctr].index != index || v3_codecs[ctr].format != format); ctr++);

    return &v3_codecs[ctr];
}

const v3_codec *
v3_codec_channel_get(v3_handle v3h, uint16_t id) {
    const char func[] = "v3_codec_channel_get";

    _v3_connection *v3c;
    const v3_codec *codec;
    v3_channel c = { .id = id, .codec_index = -1, .codec_format = -1 };

    if (_v3_handle_valid(v3h) != V3_OK || !v3_logged_in(v3h)) {
        return v3_codec_get(-1, -1);
    }
    _v3_enter(v3h, func);

    if (id) {
        _v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_CHANNEL, &c);
    }
    if (id && c.codec_index >= 0 && c.codec_format >= 0) {
        codec = v3_codec_get(c.codec_index, c.codec_format);
    } else {
        v3c = _v3_handles[v3h];
        codec = v3_codec_get(v3c->codec_index, v3c->codec_format);
    }

    _v3_leave(v3h, func);
    return codec;
}

const v3_codec *
v3_codec_user_get(v3_handle v3h, uint16_t id) {
    const char func[] = "v3_codec_user_get";

    const v3_codec *codec;
    v3_user u = { .id = id };

    if (_v3_handle_valid(v3h) != V3_OK || !v3_logged_in(v3h)) {
        return v3_codec_get(-1, -1);
    }
    _v3_enter(v3h, func);

    if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u) == V3_OK) {
        codec = v3_codec_channel_get(v3h, u.channel);
        _v3_leave(v3h, func);
        return codec;
    }

    _v3_leave(v3h, func);
    return v3_codec_get(-1, -1);
}

uint32_t
v3_codec_rate_get(int16_t index, int16_t format) {
    return v3_codec_get(index, format)->rate;
}

