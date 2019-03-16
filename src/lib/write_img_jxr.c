/* This file is part of ImPack2.
 *
 * ImPack2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ImPack2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ImPack2. If not, see <http://www.gnu.org/licenses/>. */

#include "config.h"

#ifdef IMPACK_WITH_JXR

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <JXRGlue.h>
#include "impack.h"
#include "impack_internal.h"
#include "img.h"

#define BUFSTEP 131072 // 128 KiB

typedef struct {
	uint8_t *buf;
	uint64_t bufsize;
	uint64_t filesize;
	uint64_t pos;
} jxr_io_state_t;

ERR jxr_io_close(struct WMPStream **strm) {
	
	jxr_io_state_t *state = (*strm)->state.pvObj;
	free(state->buf);
	free(state);
	free(*strm);
	return WMP_errSuccess;
	
}

Bool jxr_io_eos(struct WMPStream *strm) {
	
	jxr_io_state_t *state = strm->state.pvObj;
	return (state->pos == state->filesize);
	
}

ERR jxr_io_read(struct WMPStream *strm, void *buf, size_t count) {
	
	jxr_io_state_t *state = strm->state.pvObj;
	if (state->pos + count >= state->filesize) {
		return WMP_errFileIO;
	}
	memcpy(buf, state->buf, count);
	state->pos += count;
	return WMP_errSuccess;
	
}

ERR jxr_io_write(struct WMPStream *strm, const void *buf, size_t count) {
	
	jxr_io_state_t *state = strm->state.pvObj;
	if (state->pos + count >= state->bufsize) {
		uint64_t newsize = state->bufsize;
		while (state->pos + count >= newsize) {
			newsize += BUFSTEP;
		}
		uint8_t *newbuf = realloc(state->buf, newsize);
		if (newbuf == NULL) {
			return WMP_errFileIO;
		}
		state->buf = newbuf;
		memset(state->buf + state->bufsize, 0, newsize - state->bufsize);
		state->bufsize = newsize;
	}
	memcpy(state->buf + state->pos, buf, count);
	state->pos += count;
	if (state->pos > state->filesize) {
		state->filesize = state->pos;
	}
	return WMP_errSuccess;
	
}

ERR jxr_io_setpos(struct WMPStream *strm, size_t pos) {
	
	jxr_io_state_t *state = strm->state.pvObj;
	state->pos = pos;
	return WMP_errSuccess;
	
}

ERR jxr_io_getpos(struct WMPStream *strm, size_t *pos) {
	
	jxr_io_state_t *state = strm->state.pvObj;
	*pos = state->pos;
	return WMP_errSuccess;
	
}

impack_error_t impack_write_img_jxr(FILE *output_file, uint8_t *pixeldata, uint64_t pixeldata_size, uint64_t img_width, uint64_t img_height) {
	
	if (img_width > INT32_MAX || img_height > INT32_MAX) {
		return ERROR_IMG_SIZE;
	}
	
	impack_error_t ret = ERROR_MALLOC;
	PKImageEncode *encoder = NULL;
	struct WMPStream *strm = malloc(sizeof(struct WMPStream));
	if (strm == NULL) {
		goto cleanup;
	}
	jxr_io_state_t *io_state = malloc(sizeof(jxr_io_state_t));
	if (io_state == NULL) {
		free(strm);
		goto cleanup;
	}
	io_state->buf = malloc(BUFSTEP);
	if (io_state->buf == NULL) {
		free(strm);
		free(io_state);
		goto cleanup;
	}
	memset(io_state->buf, 0, BUFSTEP);
	io_state->bufsize = BUFSTEP;
	io_state->filesize = 0;
	io_state->pos = 0;
	strm->state.pvObj = io_state;
	strm->Close = jxr_io_close;
	strm->EOS = jxr_io_eos;
	strm->Read = jxr_io_read;
	strm->Write = jxr_io_write;
	strm->GetPos = jxr_io_getpos;
	strm->SetPos = jxr_io_setpos;
	
	if (PKImageEncode_Create_WMP(&encoder) != WMP_errSuccess) {
		free(((jxr_io_state_t*) strm->state.pvObj)->buf);
		free(strm->state.pvObj);
		free(strm);
		goto cleanup;
	}
	if (encoder->Initialize(encoder, strm, &encoder->WMP.wmiSCP, sizeof(CWMIStrCodecParam)) != WMP_errSuccess) {
		goto cleanup;
	}
	encoder->WMP.wmiSCP.cfColorFormat = YUV_444;
	encoder->WMP.wmiSCP.bdBitDepth = BD_LONG;
	encoder->WMP.wmiSCP.bfBitstreamFormat = SPATIAL;
	encoder->WMP.wmiSCP.bProgressiveMode = false;
	encoder->WMP.wmiSCP.olOverlap = OL_ONE;
	encoder->WMP.wmiSCP.cNumOfSliceMinus1H = 0;
	encoder->WMP.wmiSCP.cNumOfSliceMinus1V = 0;
	encoder->WMP.wmiSCP.sbSubband = SB_ALL;
	encoder->WMP.wmiSCP.uAlphaMode = 0;
	encoder->WMP.wmiSCP.uiDefaultQPIndex = 1;
	encoder->WMP.wmiSCP.uiDefaultQPIndexAlpha = 1;
	encoder->SetPixelFormat(encoder, GUID_PKPixelFormat24bppRGB);
	encoder->SetSize(encoder, img_width, img_height);
	if (encoder->WritePixels(encoder, img_height, pixeldata, img_width * 3) != WMP_errSuccess) {
		goto cleanup;
	}
	if (fwrite(io_state->buf, 1, io_state->filesize, output_file) != io_state->filesize) {
		ret = ERROR_OUTPUT_IO;
		goto cleanup;
	}
	encoder->Release(&encoder);
	return ERROR_OK;
	
cleanup:
	if (encoder != NULL) {
		encoder->Release(&encoder);
	}
	return ret;
	
}

#endif
