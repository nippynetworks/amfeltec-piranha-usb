/*****************************************************************************
* amf_main.c  Amfeltec USB Device Driver
*
* Copyright (C) 2013-2014, Amfeltec Corp.
******************************************************************************
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#include "amf_usb.h"
#include "amf_usb_utils.h"

//===================================== GLOBALS ===========================================
static int amf_g_driver_started = 0;

//===================================== INIT ==============================================

static int __init amf_init(void)
{
	DEBUG_EVENT("Amfeltec Device Driver Start\n");
	amf_usb_init();
	return 0;
}

static void __exit amf_exit(void)
{
	DEBUG_EVENT("Amfeltec Device Driver Exit\n");
	amf_usb_exit();
}

int amf_set_driver_started(void) {
	amf_g_driver_started = 1;
	return 0;
}

int amf_get_driver_started(void) {
	return amf_g_driver_started;
}

module_init(amf_init);
module_exit(amf_exit);

MODULE_AUTHOR("Amfeltec");
MODULE_DESCRIPTION("Amfeltec USB Device Driver");
MODULE_LICENSE("GPL");
