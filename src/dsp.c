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

void
_v3_coder_destroy(v3_handle v3h, v3_coder *coder) {
    _v3_enter(v3h, __func__);

    if (coder->state) {
        _v3_debug(v3h, V3_DBG_MEMORY, "releasing %scoder state", (coder->encoder) ? "en" : "de");
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
            if (coder->encoder) {
                celt_encoder_destroy(coder->state);
            } else {
                celt_decoder_destroy(coder->state);
            }
            celt_mode_destroy(coder->mode);
            coder->mode = NULL;
            _v3_debug(v3h, V3_DBG_MEMORY, "released celt state");
            break;
#endif
#if HAVE_SPEEX
          case 3:
            if (coder->encoder) {
                speex_encoder_destroy(coder->state);
            } else {
                speex_decoder_destroy(coder->state);
            }
            _v3_debug(v3h, V3_DBG_MEMORY, "released speex state");
            break;
#endif
          default:
            break;
        }
        coder->state = NULL;
    }

    _v3_leave(v3h, __func__);
}

void
_v3_audio_amplify(v3_handle v3h, int16_t *pcm, uint32_t pcmlen, float **volume, size_t count) {
    static const int16_t samplemax = 0x7fff;
    static const int16_t samplemin = 0x7fff + 1;
    register float tmpsample;
    size_t pos;
    float mult;
    uint32_t ctr;

    _v3_enter(v3h, __func__);

    for (pos = 0; pos < count; ++pos) {
        if (volume[pos] && *volume[pos] >= 0.0 && *volume[pos] != 1.0) {
            mult = tan((*volume[pos] > 1.95 ? 1.95 : *volume[pos]) * M_PI * 25 / 100.0);
            _v3_debug(v3h, V3_DBG_INFO, "amplifying pcm to %.0f%% (multiplier: %f)", *volume[pos] * 100.0, mult);
            for (ctr = 0; ctr < pcmlen / sizeof(*pcm); ++ctr) {
                tmpsample = pcm[ctr];
                tmpsample *= mult;
                pcm[ctr] = (tmpsample > samplemax) ? samplemax : (tmpsample < samplemin ? samplemin : tmpsample);
            }
        }
    }

    _v3_leave(v3h, __func__);
}

int
_v3_audio_send(v3_handle v3h, uint32_t rate, uint8_t channels, const void *pcm, uint32_t pcmlen) {
    _v3_connection *v3c;
    const v3_codec *codec;
    uint8_t pcmbuf[1 << 16];
    uint32_t pcmbuflen = 0;
    uint8_t databuf[1 << 11];
    uint32_t databuflen = 0;
    uint32_t framesize;
    uint8_t *ptr;
    uint32_t *len;
    uint32_t rd;
    float *volume[1];
    uint16_t framecount;
    uint8_t celtfragsize;
    int ret = V3_OK;

    _v3_enter(v3h, __func__);

    channels = (channels == 2) ? 2 : 1;
    v3c = _v3_handles[v3h];
    codec = v3_codec_channel_get(v3h, v3c->luser.channel);
    framesize = codec->framesize * channels;

    if (!v3_codec_valid(codec)) {
        _v3_error(v3h, "invalid or unsupported codec");
        _v3_leave(v3h, __func__);
        return V3_FAILURE;
    }
    if (rate != codec->rate) {
#if HAVE_SPEEXDSP
        int err = 0;
        uint32_t in_len = pcmlen;
        uint32_t out_len = sizeof(pcmbuf);

        if (!v3c->resampler.state ||
            v3c->resampler.in_rate != rate ||
            v3c->resampler.out_rate != codec->rate ||
            v3c->resampler.channels != channels) {
            if (v3c->resampler.state) {
                speex_resampler_destroy(v3c->resampler.state);
            }
            v3c->resampler.state = speex_resampler_init(
                    (v3c->resampler.channels = channels),
                    (v3c->resampler.in_rate = rate),
                    (v3c->resampler.out_rate = codec->rate),
                    SPEEX_RESAMPLER_QUALITY_VOIP,
                    &err);
        }
        in_len  /= sizeof(int16_t) * channels;
        out_len /= sizeof(int16_t) * channels;
        if (err || (err = speex_resampler_process_interleaved_int(
                v3c->resampler.state,
                pcm,
                &in_len,
                (void *)pcmbuf,
                &out_len))) {
            _v3_error(v3h, "resampler error: %i: %s", err, speex_resampler_strerror(err));
            _v3_leave(v3h, __func__);
            return V3_FAILURE;
        }
        pcmbuflen = out_len * sizeof(int16_t) * channels;
#else
        _v3_error(v3h, "resampler needed for output rate (in: %uHz != out: %uHz)", rate, codec->rate);
        _v3_leave(v3h, __func__);
        return V3_FAILURE;
#endif
    } else {
        if (pcmlen > sizeof(pcmbuf)) {
            _v3_error(v3h, "pcm length larger than buffer size (%u > %lu)", pcmlen, sizeof(pcmbuf));
            _v3_leave(v3h, __func__);
            return V3_FAILURE;
        }
        memcpy(pcmbuf, pcm, pcmlen);
        pcmbuflen = pcmlen;
    }
    if (channels == 2) {
        int16_t *sample = (int16_t *)pcmbuf;
        uint32_t ctr;

        switch (codec->index) {
#if HAVE_CELT
          case 1:
          case 2:
            break;
#endif
          default:
            pcmbuflen /= 2;
            for (ctr = 0; ctr < pcmbuflen; ++ctr) {
                sample[ctr] = sample[ctr*2] / 2 + sample[ctr*2+1] / 2;
            }
            channels = 1;
            break;
        }
    }
    while (*(len = (v3c->pcmqueued) ? &v3c->pcmqueued : &pcmbuflen) / framesize >= codec->frames) {
        ptr = (v3c->pcmqueued) ? v3c->pcmq : pcmbuf;
        rd = framesize * codec->frames;
        *volume = &v3c->luser.volume;
        _v3_audio_amplify(v3h, (void *)ptr, rd, volume, sizeof(volume) / sizeof(*volume));
        databuflen = sizeof(databuf);
        framecount = 0;
        celtfragsize = 0;
        if ((ret = _v3_audio_encode(
                v3h,
                ptr,
                rd,
                codec->index,
                codec->format,
                &v3c->encoder,
                databuf,
                &databuflen,
                channels,
                &framecount,
                &celtfragsize)) == V3_OK) {
            switch (codec->index) {
#if HAVE_CELT
              case 1:
              case 2:
                {
                    uint8_t *celtdataptr = databuf;
                    uint16_t pktlen = (codec->index == 1) ? 198 : 108;
                    uint8_t pktframes = pktlen / celtfragsize;

                    pktlen = pktframes * celtfragsize;
                    while (framecount && pktframes) {
                        if (framecount < pktframes) {
                            pktframes = framecount;
                            pktlen = pktframes * celtfragsize;
                        }
                        ret = _v3_msg_audio_put(
                                v3h,
                                V3_AUDIO_DATA,
                                codec->index,
                                codec->format,
                                2000 + channels,
                                celtdataptr,
                                pktlen);
                        celtdataptr += pktlen;
                        framecount -= pktframes;
                    }
                }
                break;
#endif
              default:
                ret = _v3_msg_audio_put(
                        v3h,
                        V3_AUDIO_DATA,
                        codec->index,
                        codec->format,
                        pcmlen,
                        databuf,
                        databuflen);
                break;
            }
        }
        memmove(ptr, ptr + rd, (v3c->pcmqueued ? sizeof(v3c->pcmq) : sizeof(pcmbuf)) - rd);
        *len -= rd;
    }
    if (pcmbuflen) {
        rd = sizeof(v3c->pcmq) - v3c->pcmqueued;
        rd = (pcmbuflen > rd) ? rd : pcmbuflen;
        memcpy(v3c->pcmq + v3c->pcmqueued, pcmbuf, rd);
        v3c->pcmqueued += rd;
    }

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_audio_encode(
        v3_handle v3h,
        /* pcm input */
        const uint8_t *pcm,
        uint32_t pcmlen,
        /* encoded output */
        int16_t index,
        int16_t format,
        v3_coder *coder,
        uint8_t *data,
        uint32_t *datalen,
        /* optional args */
        uint8_t channels,
        uint16_t *framecount,
        uint8_t *celtfragsize) {
    uint32_t maxdatalen;
    size_t ctr;
    int ret = V3_OK;

    _v3_enter(v3h, __func__);

    if (!pcm || !pcmlen || !coder || !data || !datalen || (datalen && !*datalen)) {
        _v3_leave(v3h, __func__);
        return V3_FAILURE;
    }

    maxdatalen = *datalen;
    *datalen = 0;
    channels = (channels == 2) ? 2 : 1;

    if (coder->state && (coder->index != index || coder->format != format)) {
        _v3_coder_destroy(v3h, coder);
    }
    switch (index) {
#if HAVE_GSM
      case 0:
        {
            const v3_codec *codec = v3_codec_get(index, format);
            int opt = 1;

            _v3_debug(v3h, V3_DBG_INFO, "encoding %u bytes of pcm to gsm @ %u", pcmlen, codec->rate);
            if (!coder->state) {
                if (!(coder->state = gsm_create())) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to create gsm encoder");
                    ret = V3_FAILURE;
                    break;
                }
                gsm_option(coder->state, GSM_OPT_WAV49, &opt);
                coder->index = index;
                coder->format = format;
                coder->encoder = true;
            }
            for (ctr = 0; ctr < pcmlen / codec->framesize && *datalen + 65 <= maxdatalen; ++ctr) {
                gsm_encode(coder->state, (void *)pcm, (void *)data);
                gsm_encode(coder->state, (void *)pcm+(codec->framesize/2), (void *)data+32);
                pcm      += codec->framesize;
                data     += 65;
                *datalen += 65;
            }
            if (framecount) {
                *framecount = pcmlen / codec->framesize;
            }
        }
        break;
#endif
#if HAVE_CELT
      case 1:
      case 2:
        {
            static const int prediction = 2;
            static const int complexity = 10;
            const v3_codec *codec = v3_codec_get(index, format);
            uint8_t fragsize = (celtfragsize && *celtfragsize) ? *celtfragsize : (index == 1 ? 60 : 54);
            uint8_t framesize = fragsize - 1;

            _v3_debug(v3h, V3_DBG_INFO, "encoding %u bytes of pcm to celt @ %u", pcmlen, codec->rate);
            if (!coder->state || channels != coder->channels) {
                if (coder->state) {
                    celt_encoder_destroy(coder->state);
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
                if (!(coder->state = celt_encoder_create(coder->mode, channels, NULL))) {
                    if (coder->mode) {
                        celt_mode_destroy(coder->mode);
                        coder->mode = NULL;
                    }
                    _v3_debug(v3h, V3_DBG_INFO, "failed to create celt decoder");
                    ret = V3_FAILURE;
                    break;
                }
                if (celt_encoder_ctl(coder->state, CELT_SET_PREDICTION(prediction)) != CELT_OK ||
                    celt_encoder_ctl(coder->state, CELT_SET_COMPLEXITY(complexity)) != CELT_OK) {
                    _v3_debug(v3h, V3_DBG_INFO, "celt_encoder_ctl: prediction request failed or complexity 0-10 is only supported");
                    celt_encoder_destroy(coder->state);
                    coder->state = NULL;
                    celt_mode_destroy(coder->mode);
                    coder->mode = NULL;
                    ret = V3_FAILURE;
                    break;
                }
                coder->index = index;
                coder->format = format;
                coder->channels = channels;
                coder->encoder = true;
            }
            for (ctr = 0; ctr < pcmlen / (codec->framesize * channels) && *datalen + fragsize <= maxdatalen; ++ctr) {
                *data++ = framesize;
                celt_encode(coder->state, (void *)pcm, NULL, (void *)data, framesize);
                pcm      += codec->framesize * channels;
                data     += framesize;
                *datalen += fragsize;
            }
            if (framecount) {
                *framecount = pcmlen / (codec->framesize * channels);
            }
            if (celtfragsize) {
                *celtfragsize = fragsize;
            }
        }
        break;
#endif
#if HAVE_SPEEX
      case 3:
        {
            static const uint16_t maxspxbuf = 200;
            const v3_codec *codec = v3_codec_get(index, format);
            uint16_t framesize;
            SpeexBits bits;

            _v3_debug(v3h, V3_DBG_INFO, "encoding %u bytes of pcm to speex @ %u", pcmlen, codec->rate);
            if (!coder->state) {
                switch (codec->rate) {
                  case 8000:
                    coder->state = speex_encoder_init(&speex_nb_mode);
                    break;
                  case 16000:
                    coder->state = speex_encoder_init(&speex_wb_mode);
                    break;
                  case 32000:
                    coder->state = speex_encoder_init(&speex_uwb_mode);
                    break;
                }
                if (!coder->state) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to create speex encoder");
                    ret = V3_FAILURE;
                    break;
                }
                speex_encoder_ctl(coder->state, SPEEX_SET_QUALITY, (void *)&codec->quality);
                coder->index = index;
                coder->format = format;
                coder->encoder = true;
            }
            *(uint16_t *)data = htons(pcmlen / codec->framesize);
            data     += sizeof(uint16_t);
            *datalen += sizeof(uint16_t);
            *(uint16_t *)data = htons(codec->framesize / sizeof(int16_t));
            data     += sizeof(uint16_t);
            *datalen += sizeof(uint16_t);
            speex_bits_init(&bits);
            for (ctr = 0; ctr < pcmlen / codec->framesize && *datalen + maxspxbuf <= maxdatalen; ++ctr) {
                speex_encode_int(coder->state, (void *)pcm, &bits);
                framesize = speex_bits_write(&bits, (void *)data + sizeof(uint16_t), maxspxbuf);
                speex_bits_reset(&bits);
                *(uint16_t *)data = htons(framesize);
                pcm      += codec->framesize;
                data     += sizeof(uint16_t) + framesize;
                *datalen += sizeof(uint16_t) + framesize;
            }
            speex_bits_destroy(&bits);
            if (framecount) {
                *framecount = pcmlen / codec->framesize;
            }
        }
        break;
#endif
      default:
        _v3_debug(v3h, V3_DBG_INFO, "unsupported codec: index: %i | format: %i", index, format);
        ret = V3_FAILURE;
        break;
    }

    _v3_leave(v3h, __func__);
    return ret;
}

int
_v3_audio_decode(
        v3_handle v3h,
        /* encoded input */
        int16_t index,
        int16_t format,
        v3_coder *coder,
        const uint8_t *data,
        int32_t datalen,
        /* pcm output */
        uint8_t *pcm,
        uint32_t *pcmlen,
        /* optional args */
        uint32_t *rate,
        uint8_t channels) {
    uint32_t maxpcmlen;
    int ret = V3_OK;

    _v3_enter(v3h, __func__);

    if (!coder || !data || !datalen || !pcm || !pcmlen || (pcmlen && !*pcmlen)) {
        _v3_leave(v3h, __func__);
        return V3_FAILURE;
    }

    maxpcmlen = *pcmlen;
    *pcmlen = 0;
    channels = (channels == 2) ? 2 : 1;

    if (coder->state && (coder->index != index || coder->format != format)) {
        _v3_coder_destroy(v3h, coder);
    }
    switch (index) {
#if HAVE_GSM
      case 0:
        {
            const v3_codec *codec = v3_codec_get(index, format);
            int opt = 1;

            _v3_debug(v3h, V3_DBG_INFO, "decoding %i bytes of gsm to pcm @ %u", datalen, codec->rate);
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
                coder->encoder = false;
            }
            while ((datalen -= 65) >= 0 && *pcmlen + codec->framesize <= maxpcmlen) {
                if (gsm_decode(coder->state, (void *)data, (void *)pcm) ||
                    gsm_decode(coder->state, (void *)data+33, (void *)pcm+(codec->framesize/2))) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to decode gsm frame");
                }
                data    += 65;
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

            _v3_debug(v3h, V3_DBG_INFO, "decoding %i bytes of celt to pcm @ %u", datalen, codec->rate);
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
                coder->encoder = false;
            }
            while (datalen) {
                framesize = *data++;
                datalen -= framesize + 1;
                if (!framesize || datalen < 0 || *pcmlen + codec->framesize * channels > maxpcmlen) {
                    _v3_debug(v3h, V3_DBG_INFO, "received a malformed celt packet");
                    ret = V3_MALFORM;
                    break;
                }
                if (celt_decode(coder->state, (void *)data, framesize, (void *)pcm) != CELT_OK) {
                    _v3_debug(v3h, V3_DBG_INFO, "failed to decode celt frame");
                }
                data    += framesize;
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

            _v3_debug(v3h, V3_DBG_INFO, "decoding %i bytes of speex to pcm @ %u", datalen, codec->rate);
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
                coder->encoder = false;
            }
            datalen -= sizeof(uint16_t) * 2;
            if (datalen < 0) {
                _v3_debug(v3h, V3_DBG_INFO, "received a malformed speex packet");
                ret = V3_MALFORM;
                break;
            }
            data += sizeof(uint16_t) * 2;
            speex_bits_init(&bits);
            while (datalen) {
                framesize = ntohs(*(uint16_t *)data);
                datalen -= framesize + sizeof(uint16_t);
                if (!framesize || datalen < 0 || *pcmlen + codec->framesize > maxpcmlen) {
                    _v3_debug(v3h, V3_DBG_INFO, "received a malformed speex packet");
                    ret = V3_MALFORM;
                    break;
                }
                data += sizeof(uint16_t);
                speex_bits_read_from(&bits, (void *)data, framesize);
                speex_decode_int(coder->state, &bits, (void *)pcm);
                speex_bits_reset(&bits);
                data    += framesize;
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

    _v3_leave(v3h, __func__);
    return ret;
}

const v3_codec *
v3_codec_get(int16_t index, int16_t format) {
    int ctr;

    for (ctr = 0; v3_codecs[ctr].index >= 0 && (v3_codecs[ctr].index != index || v3_codecs[ctr].format != format); ++ctr);

    return &v3_codecs[ctr];
}

const v3_codec *
v3_codec_channel_get(v3_handle v3h, uint16_t id) {
    _v3_connection *v3c;
    const v3_codec *codec;
    v3_channel c = { .id = id, .codec_index = -1, .codec_format = -1 };

    if (_v3_handle_valid(v3h) != V3_OK || !v3_logged_in(v3h)) {
        return v3_codec_get(-1, -1);
    }
    _v3_enter(v3h, __func__);

    if (id) {
        _v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_CHANNEL, &c, 0);
    }
    if (id && c.codec_index >= 0 && c.codec_format >= 0) {
        codec = v3_codec_get(c.codec_index, c.codec_format);
    } else {
        v3c = _v3_handles[v3h];
        codec = v3_codec_get(v3c->codec_index, v3c->codec_format);
    }

    _v3_leave(v3h, __func__);
    return codec;
}

const v3_codec *
v3_codec_user_get(v3_handle v3h, uint16_t id) {
    const v3_codec *codec;
    v3_user u = { .id = id };

    if (_v3_handle_valid(v3h) != V3_OK || !v3_logged_in(v3h)) {
        return v3_codec_get(-1, -1);
    }
    _v3_enter(v3h, __func__);

    if (_v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
        codec = v3_codec_channel_get(v3h, u.channel);
        _v3_leave(v3h, __func__);
        return codec;
    }

    _v3_leave(v3h, __func__);
    return v3_codec_get(-1, -1);
}

uint32_t
v3_codec_rate_get(int16_t index, int16_t format) {
    return v3_codec_get(index, format)->rate;
}

int
v3_codec_valid(const v3_codec *codec) {
    return (codec && codec->index >= 0 && codec->format >= 0);
}

void
v3_volume_master_set(float level) {
    _volume = level;
}

float
v3_volume_master_get(void) {
    return _volume;
}

int
v3_volume_server_set(v3_handle v3h, float level) {
    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    _v3_handles[v3h]->volume = level;

    _v3_leave(v3h, __func__);
    return V3_OK;
}

float
v3_volume_server_get(v3_handle v3h) {
    float ret;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }
    _v3_enter(v3h, __func__);

    ret = _v3_handles[v3h]->volume;

    _v3_leave(v3h, __func__);
    return ret;
}

int
v3_volume_user_set(v3_handle v3h, uint16_t id, float level) {
    _v3_connection *v3c;
    v3_user u = { .id = id };
    int ret = V3_OK;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return V3_FAILURE;
    }
    _v3_enter(v3h, __func__);

    _v3_mutex_lock(v3h);

    v3c = _v3_handles[v3h];

    if (!id) {
        ret = v3_volume_server_set(v3h, level);
    } else if (id == v3c->luser.id) {
        v3c->luser.volume = level;
    } else if ((ret = _v3_data(v3h, V3_DATA_RETURN, V3_DATA_TYPE_USER, &u, 0)) == V3_OK) {
        u.next->volume = level;
    }

    _v3_mutex_unlock(v3h);

    _v3_leave(v3h, __func__);
    return ret;
}

float
v3_volume_user_get(v3_handle v3h, uint16_t id) {
    _v3_connection *v3c;
    v3_user u = { .id = id };
    float ret = 0;

    if (_v3_handle_valid(v3h) != V3_OK) {
        return 0;
    }
    _v3_enter(v3h, __func__);

    v3c = _v3_handles[v3h];

    if (!id) {
        ret = v3_volume_server_get(v3h);
    } else if (id == v3c->luser.id) {
        ret = v3c->luser.volume;
    } else if (_v3_data(v3h, V3_DATA_COPY, V3_DATA_TYPE_USER, &u, 0) == V3_OK) {
        ret = u.volume;
    }

    _v3_leave(v3h, __func__);
    return ret;
}

