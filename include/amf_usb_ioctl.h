 /*
  *  amf_usb_ioctl.h - 'firmware update driver API definitions'
  *
  *  Maintained by:  <support@amfeltec.com>
  *
  *  Copyright (C) 2012-2015 Amfeltec Corp.
  *
  *
  *  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 2, or (at your option)
  *  any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; see the file COPYING.  If not, write to
  *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
  */

#ifndef AMF_USB_IOCTL_H
#define AMF_USB_IOCTL_H
#include <linux/ioctl.h>


#define AMF_MAX_CMD_DATA_SIZE 1000

typedef struct
{
	unsigned short	len;
	unsigned int	offset;
	unsigned char	bar;
	int				ret;
	int 			dev_num;
	int 			order;
	int 			amf_device_type;
	unsigned char	data[AMF_MAX_CMD_DATA_SIZE];
} amf_arg_t;


#define AMF_GET_STATUS _IOR('q', 1, amf_arg_t *)
#define AMF_GET_CPU_ID _IOR('q', 2, amf_arg_t *)
#define AMF_CPU_READ_REG _IOR('q', 3, amf_arg_t *)
#define AMF_CPU_WRITE_REG _IOWR('q', 4, amf_arg_t *)

#define AMF_FW_UPDATE_ENABLE _IOW('q', 5, amf_arg_t *)
#define AMF_FW_DATA_READ _IOR('q', 6, amf_arg_t *)
#define AMF_FW_DATA_WRITE _IOWR('q', 7, amf_arg_t *)

int exec_usb_cpu_read(int fd, int off, unsigned char *data, unsigned short len);
int exec_usb_fwupdate_enable(int fd);
int exec_usb_fw_data_write(int fd, unsigned char *data, unsigned short len);
int exec_usb_fw_data_read(int fd, unsigned char *data, unsigned short len);

#endif
