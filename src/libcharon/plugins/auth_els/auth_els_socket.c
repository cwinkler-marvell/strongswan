/*
 * Copyright (C) 2019-2020 Marvell 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <utils/chunk.h>
#include <config/child_cfg.h>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <linux/types.h>

#include <linux/bsg.h>
#include <scsi/scsi_bsg_fc.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

/* "readdir" etc. are defined here. */
#include <dirent.h>
/* limits.h defines "PATH_MAX". */
#include <limits.h>

#include <daemon.h>
#include <networking/host.h>
#include <processing/jobs/callback_job.h>

#include <pthread.h>

#include "auth_els_plugin.h"
#include "auth_els_socket.h"
#include "auth_els_utils.h"

/**
 * Data to pass to the send_message() callback job
 */
typedef struct {
	chunk_t chunk;
	int fd;
} job_data_t;

typedef struct private_auth_els_socket_t private_auth_els_socket_t;
/**
 * Private data of an auth_els_socket_t object.
 */
struct private_auth_els_socket_t {

	auth_els_socket_t public;

};

METHOD(socket_t, supported_families, socket_family_t,
	private_auth_els_socket_t *this)
{
	return SOCKET_FAMILY_FC;
}

METHOD(socket_t, destroy, void,
	private_auth_els_socket_t *this)
{
	free(this);
}

METHOD(socket_t, receiver, status_t,
		private_auth_els_socket_t *this, packet_t **packet)
{
	DBG_ENTER;

	// We don't use the receiver function, so just tell the core to stop trying.
	// Receives are done when the driver gives us an ELS_RECV AEN.
	return NOT_SUPPORTED;
}

METHOD(socket_t, sender, status_t,
		private_auth_els_socket_t *this, packet_t *packet)
{
	DBG_ENTER;

	return NOT_SUPPORTED;
}

METHOD(socket_t, get_port, uint16_t, 
	private_auth_els_socket_t *this, bool nat_t)
{
	return 77;  // dummy host number
}

auth_els_socket_t *auth_els_socket_create()
{
	private_auth_els_socket_t *this;

	INIT(this,
		.public = {
			.socket = {
				.send = _sender,
				.receive = _receiver,
				.supported_families = _supported_families,
				.get_port = _get_port,
				.destroy = _destroy,
			},
		},
	);
	
	return &this->public;
}