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
	json_bool fast;	// whether or not to cheat God
	// internal framegrabber struct
	Fg_Struct *fg;
	// dma_mem area
	dma_mem *dma;
	// port number for camera (see PORT_* in fg_define.h)
	unsigned cam;
	// framebuffer matrix (put into the state every round)
	// this is NOT a pointer because it's just a container for the pointer
	// the kernel driver will spit at us
	gsl_matrix_uchar fb;
};

// initialize fgsdk device
// (does not handle loading and activating applets)
int aylp_basler_fgsdk_init(struct aylp_device *self);

// busy-wait for frame
int aylp_basler_fgsdk_process(
	struct aylp_device *self, struct aylp_state *state
);

// just keep the same image pointer
int aylp_basler_fgsdk_process_fast(
	struct aylp_device *self, struct aylp_state *state
);

// close fgsdk device when loop exits
int aylp_basler_fgsdk_close(struct aylp_device *self);

#endif

