// this plugin fetches frames from https://github.com/cdqp/menable-driver
#ifndef AYLP_BASLER_FGSDK_H
#define AYLP_BASLER_FGSDK_H

#include "anyloop.h"

#include "basler_fg.h"

struct aylp_basler_fgsdk_data {
	// TODO: docstring
	size_t width;
	size_t height;
	double pitch;
	size_t timeout;	// seconds
	// internal framegrabber struct
	Fg_Struct *fg;
	// dma_mem area
	dma_mem *dma;
	// port number for camera (see PORT_* in fg_define.h)
	unsigned cam;
	// framebuffer matrix (put into the state every round) and block
	// these are NOT pointers because they're just containers for the
	// pointer the kernel driver will spit at us
	gsl_matrix_uchar fb;
	gsl_block_uchar fb_block;
};

// initialize fgsdk device
// (does not handle loading and activating applets)
int aylp_basler_fgsdk_init(struct aylp_device *self);

// busy-wait for frame
int aylp_basler_fgsdk_process(struct aylp_device *self, struct aylp_state *state);

// close fgsdk device when loop exits
int aylp_basler_fgsdk_close(struct aylp_device *self);

#endif

