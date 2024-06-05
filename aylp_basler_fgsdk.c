#include <stdio.h>
#include <stdlib.h>

#include "anyloop.h"
#include "logging.h"
#include "xalloc.h"
#include "aylp_basler_fgsdk.h"

#include "basler_fg.h"

int aylp_basler_fgsdk_init(struct aylp_device *self)
{
	int err;
	self->device_data = xcalloc(1, sizeof(struct aylp_basler_fgsdk_data));
	struct aylp_basler_fgsdk_data *data = self->device_data;
	// attach methods
	self->process = &aylp_basler_fgsdk_process;
	self->close = &aylp_basler_fgsdk_close;

	// set default params
	data->pitch = 0.0;
	data->cam = PORT_A;	// TODO: parametrize?
	data->timeout = 10;	// TODO: parametrize?

	// parse the params json into our data struct
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "width")) {
			data->width = json_object_get_uint64(val);
			log_trace("width = %llu", data->width);
		} else if (!strcmp(key, "height")) {
			data->height = json_object_get_uint64(val);
			log_trace("height = %llu", data->height);
		} else if (!strcmp(key, "pitch")) {
			data->pitch = json_object_get_double(val);
			log_trace("pitch = %E", data->pitch);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	// make sure we didn't miss any params
	if (!data->width || !data->height) {
		log_error("You must provide nonzero width and height params");
		return -1;
	}

	// set up our fb matrix
	data->fb.size1 = data->height;
	data->fb.size2 = data->width;
	data->fb.tda = data->width;
	// it does not own any blocks; all the data is right in fg.data
	data->fb.block = 0;
	data->fb.owner = 0;

	err = Fg_InitLibraries(0);
	if (err) {
		log_error("Fg_InitLibraries failed.");
		return -1;
	}

	// TODO: parametrize this
	int board_number = 0;
	char *applet = "Acq_SingleFullAreaGray";
	// initialize
	data->fg = Fg_Init(applet, board_number);
	if (!data->fg) {
		log_error("Fg_Init failed: %s", Fg_getLastErrorDescription(0));
		return -1;
	}

	// set width and height
	err = Fg_setParameter(data->fg, FG_WIDTH, &data->width, data->cam);
	if (err) {
		log_error("Fg_setParameter(FG_WIDTH) failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}
	err = Fg_setParameter(data->fg, FG_HEIGHT, &data->height, data->cam);
	if (err) {
		log_error("Fg_setParameter(FG_HEIGHT) failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}

	// set alignment (TODO: parametrize)
	int bit_align = FG_LEFT_ALIGNED;
	err = Fg_setParameter(data->fg, FG_BITALIGNMENT, &bit_align, data->cam);
	if (err) {
		log_error("Fg_setParameter(FG_BITALIGNMENT) failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}

	// find out how many bytes there are per pixel
	int format;
	err = Fg_getParameter(data->fg, FG_FORMAT, &format, data->cam);
	if (err) {
		log_error("Fg_getParameter(FG_FORMAT) failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}
	// TODO: support more bytes per pixel? but then returning a matrix_uchar
	// is weird and misleading....
	if (format != FG_GRAY) {
		log_error("Format %d is not supported!", format);
		return -1;
	}

	frameindex_t n_bufs = 1;	// TODO: parametrize?
	// calculate buffer size (TODO: what if overflow?)
	size_t buf_size = (size_t) data->width * data->height * n_bufs;
	// allocate memory
	data->dma = Fg_AllocMemEx(data->fg, buf_size, n_bufs);
	if (!data->dma) {
		log_error("Fg_AllocMemEx failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}

	// start acquisition (TODO: parametrize ACQ_ mode?)
	err = Fg_AcquireEx(
		data->fg, data->cam, GRAB_INFINITE, ACQ_BLOCK, data->dma
	);
	if (err) {
		log_error("Fg_AcquireEx() failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}

	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = AYLP_T_MATRIX_UCHAR;
	self->units_out = AYLP_U_COUNTS;

	return 0;
}


int aylp_basler_fgsdk_process(
	struct aylp_device *self, struct aylp_state *state
){
	int err;
	struct aylp_basler_fgsdk_data *data = self->device_data;

	frameindex_t buf = Fg_getImageEx(
		data->fg, SEL_NEXT_IMAGE, 0,
		data->cam, data->timeout, data->dma
	);
	if (UNLIKELY(buf <= 0)) {
		log_error("Fg_getImageEx failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}

	// take the pointer to the frame buffer for the newest image
	// and write it into the gsl_block_uchar that comprises fb
	data->fb.data = Fg_getImagePtrEx(data->fg, buf, data->cam, data->dma);
	// update the gsl_block_uchar with the number of bytes transferred
	size_t length = 0;
	err = Fg_getParameterEx(
		data->fg, FG_TRANSFER_LEN, &length, data->cam, data->dma, buf
	);
	if (UNLIKELY(err)) {
		log_error("Fg_getParameterEx(FG_TRANSFER_LEN) failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}
	if (UNLIKELY(length != data->width * data->height)) {
		log_error("Expected %llu by %llu image but got %llu bytes",
			data->width, data->height, length
		);
		return -1;
	}
	log_trace("Got image of length %llu bytes", length);

	// unblock frame buffer
	err = Fg_setStatusEx(data->fg, FG_UNBLOCK, buf, data->cam, data->dma);
	if (UNLIKELY(err)) {
		log_error("Fg_setStatusEx(FG_UNBLOCK) failed: %s",
			Fg_getLastErrorDescription(data->fg)
		);
		return -1;
	}

	// zero-copy update of pipeline state
	// TODO: fb is still not usable
	state->matrix_uchar = &data->fb;
	// housekeeping on the header
	state->header.type = self->type_out;
	state->header.units = self->units_out;
	state->header.log_dim.y = data->height;
	state->header.log_dim.x = data->width;
	state->header.pitch.y = data->pitch;
	state->header.pitch.x = data->pitch;
	return 0;
}


int aylp_basler_fgsdk_close(struct aylp_device *self)
{
	struct aylp_basler_fgsdk_data *data = self->device_data;
	Fg_stopAcquire(data->fg, data->cam);
	Fg_FreeMemEx(data->fg, data->dma);
	Fg_FreeGrabber(data->fg); data->fg = 0;
	Fg_FreeLibraries();
	xfree(data);
	return 0;
}

