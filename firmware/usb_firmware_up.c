 /*
  *  usb_firmware_up.c - 'firmware update application'
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>

#include "../include/amf_usb_ioctl.h"

#include "mem.h"

extern int amf_usb_firmware_update(int fd, char *firmware_file);
extern int amf_usb_status_reg(int fd);
extern int exec_usb_fwupdate_enable(int fd);
extern int exec_usb_fw_data_write(int fd, unsigned char *data, unsigned short len);
extern int exec_usb_fw_data_read(int fd, unsigned char *data, unsigned short len);
extern int exec_usb_cpu_read(int fd, int off, unsigned char *data, unsigned short len);
extern int WriteCommBlock(int fd, unsigned char *pBuffer , int BytesToWrite);
extern int ReceiveData (int fd, unsigned char *pBuffer, int BytesToRead);

static int amf_update(void);
static int print_help(void);
static int print_version(void);
int get_file(char *filename, char *prefix);

static amf_arg_t q;

typedef struct {
	int		version;
	char 	file[21];
} firmware_files_t; 



int main(int argc, char *argv[])
{
	int err = 0;
	int i;

	for(i = 0; i < argc; i++){

		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "help") || !strcmp(argv[i], "-help")){
			print_help();
			exit(0);
		} else if (!strcmp(argv[i],"-version")){
			print_version();
			exit(0);
		} else if (!strcmp(argv[i], "-fwupdate")){
			err = amf_update();						
			return(err);
		} else if (!strcmp(argv[i], "-readstatusreg")){
			return(err);
		} else {
			if (i != 0) {
				print_help();
				exit(0);
			}
		}
	}
	return 0;

}


static int amf_update()
{
	const char *driver_file = "/dev/amf_usb";
	int fd;
	int ret;

	FILE	*fin = NULL;

	char firmware_file[21];
	char prefix[14];
	char post_prefix[5];
	int  device_num = 0;
	char device_type[8];
	int dev_type = 0;
	char bootloader_version = 0;
	int  hardware_version = 0;
	int  firmware_version = 0;
	char response;
	int retries = 0;

	print_version();
	sleep(2);
	
	fd = open(driver_file, O_RDWR);
	if (fd == -1) {
		printf("Error opening Amfeltec Device File %s\n", driver_file);
		goto m_error_file;
	} 		

	memset(q.data, 0, AMF_MAX_CMD_DATA_SIZE);
	memset(firmware_file, 0, sizeof(firmware_file));
	memset(prefix, 0, sizeof(prefix));
	memset(post_prefix, 0, sizeof(post_prefix));
	memset(device_type, 0, sizeof(device_type));

	exec_usb_fwupdate_enable(fd);
	sleep(1);



	while (retries < 3) {
	
		if (ioctl(fd, AMF_GET_STATUS, &q) == -1) {
			perror("error in ioctl\n");
			goto m_error;
		} else {
		}
	
		device_num =  q.dev_num;
		if (device_num > 1) {	
			printf("More then one devices: %d, stop!\n", device_num);
			goto m_error;
		}
	
		//printf("...retries: %d\n", retries + 1);
		//printf("... data0: %2X\n", q.data[0]);		// Bootloader version
		//printf("... data1: %2X\n", q.data[1]);		// Firmware version
		//printf("... data2: %2X\n", q.data[2]);		// Hardware version
		//printf("... data3: %2X\n", q.data[3]);		// Rotary switch
		//printf("... data4: %2X\n", q.data[4]);		// ACK: 0x55
	
		// if we are not in a Bootloader, we've got garbage here
		q.order = 0xF - q.data[3];
		if ((q.order != 0) || (q.data[4] != 0x55)) {	
			retries++;
		} else {
			break;
		}
	}	

	if (retries == 3) {
		printf("Please check Rotary Switch setting, switch to 0 and reconnect device!\n\n");
		goto m_error;
	}

	dev_type = q.amf_device_type;
	switch (dev_type) {
	case 1:
		strncpy(prefix, "USB_FXO_", 8);
		strncpy(device_type, "USB_FXO", 7);		
		break;
	case 2:
		strncpy(prefix, "USB_FXS_", 8);
		strncpy(device_type, "USB_FXS", 7);		
		break;
	case 3:
		strncpy(prefix, "USB_PA_", 7);
		strncpy(device_type, "USB_PA", 6);		
		break;
	default:
		printf("Device type mismatch (%d)\n\n", q.amf_device_type);
		goto m_error;
	}
	bootloader_version = q.data[0];
	firmware_version = q.data[1];
	hardware_version = q.data[2];

	if (ioctl(fd, AMF_GET_CPU_ID, &q) == -1) {
		perror("error in ioctl\n");
		goto m_error;
	} else {
	}

	//bootloader_version = 0x02/0xA2;

	if ((bootloader_version & 0xF0) == 0xA0) {

		if ((q.data[1] == 0x00) && (q.data[0] == 0xC1)) {
			strncpy(post_prefix, "064AV", 6);
		} else if ((q.data[1] == 0x00) && (q.data[0] == 0x65)) {
			strncpy(post_prefix, "128AV", 6);
		} else if ((q.data[1] == 0x00) && (q.data[0] == 0xF5)) {
			strncpy(post_prefix, "256AV", 6);
		} else {
			printf("CPU: unknown Device ID: %X-%X, stop!\n", q.data[1], q.data[0]);
			goto m_error;
		}

	} else {

		if ((q.data[1] == 0x00) && (q.data[0] == 0xC1)) {
			strncpy(post_prefix, "064V", 5);
		} else if ((q.data[1] == 0x00) && (q.data[0] == 0x65)) {
			strncpy(post_prefix, "128V", 5);
		} else if ((q.data[1] == 0x00) && (q.data[0] == 0xF5)) {
			strncpy(post_prefix, "256V", 5);
		} else {
			printf("CPU: unknown Device ID: %X-%X, stop!\n", q.data[1], q.data[0]);
			goto m_error;
		}
	}

	strcat(prefix, post_prefix);

	if ((hardware_version == 3) && (dev_type == 1)) { 
		printf("Devices: %d, Type: %s (with Fail-over), Boot version: %X, Hardware version: %X, Firmware version: %X\n", device_num, device_type, (bootloader_version & 0xFF), hardware_version, firmware_version);
	} else {
		printf("Devices: %d, Type: %s, Boot version: %X, Hardware version: %X, Firmware version: %X\n", device_num, device_type, (bootloader_version & 0xFF), hardware_version, firmware_version);
	}

	if (get_file(firmware_file, prefix)) {
		printf("  Cancelled! \n\n");
		goto m_error;
	}	

	if ((fin = fopen(firmware_file, "r")) == NULL){
		printf("Failed to find firmware file %s, stop!\n\n",firmware_file);
		printf("\n");
		goto m_error;
	}
	fclose(fin);

//=========================================================================   Ready to update:	

	printf("\nATTENTION: Starting Firmware Update. Are you sure? (y/n)");
	scanf(" %c", &response); 
	if (response != 'y') {
		printf("  Cancelled! \n\n");
		goto m_error_file;
	}

	ret = amf_usb_firmware_update(fd, firmware_file);
	if(ret){
		printf("\n  Firmware update failed, stop\n");
		printf("  Please reconnect and reprogram device\n\n");
		goto m_error;
	} else {
		printf("\n  Firmware successfully updated!\n\n");
	}

//=========================================================================   Final check:	

	sleep(1);

	if (ioctl(fd, AMF_GET_STATUS, &q) == -1) {
		perror("error in ioctl\n");
		goto m_error;
	} else {
	}

	// if we are not in a Bootloader, we've got garbage here
	q.order = 0xF - q.data[3];
	if ((q.order != 0) || (q.data[4] != 0x55)) {	
		printf("  Final check error: Please reconnect and reprogram device\n\n");
		goto m_error;
	}
	firmware_version = q.data[1];
	if (firmware_version == 0xFF) {
		printf("  Final check error: Wrong Firmware version: %d\n", firmware_version);
		printf("  Please reconnect and reprogram device!\n");
	} else {
		printf("  New Firmware version: %X\n\n", firmware_version);
	}

		
m_error:
	close (fd);
m_error_file:
	return 0;
}





//=========================================================================   Read / Write

int exec_usb_fwupdate_enable(int fd)
{
	if (ioctl(fd, AMF_FW_UPDATE_ENABLE, &q) == -1) {
		perror("error in ioctl\n");
	} else {
	}
	return 0;
}

int exec_usb_fw_data_read(int fd, unsigned char *data, unsigned short len)
{
	int err, cnt = 0;

	if (!len) return -EINVAL;
	memset(q.data, 0, AMF_MAX_CMD_DATA_SIZE);

try_read_again:
	q.ret	= 0;			// Original call uses ret as mod_no
	q.bar	= 0;
	q.len	= len;
	q.offset	= 0;
	err = ioctl(fd, AMF_FW_DATA_READ, &q);
	if (err){
		printf("\namf_usb: exec_usb_fw_data_read...err=%X\n", err);
		return -EINVAL;
	}

	if (q.ret || q.len != len){
		usleep(100);
		if (++cnt < 100) goto try_read_again;
		printf("\namf_usb: Failed (rx timeout:%d:%d cnt:%d)\n", q.len, len, cnt);
		// exit(1);
		return -EINVAL;
	}

	memcpy(data, &q.data[0], q.len);
	return 0;

}

int exec_usb_fw_data_write(int fd, unsigned char *data, unsigned short len)
{
	int	err, cnt;

	memset(q.data, 0, AMF_MAX_CMD_DATA_SIZE);

	cnt = 0;
	//write_data_again:
	q.len	= len;
	memcpy(&q.data[0], data, len);
	err = ioctl(fd, AMF_FW_DATA_WRITE, &q);
	if (err){
		printf("\namf_usb: fw_data_write...err=%X\n", err);
		return -EINVAL;
	}
	if (q.len != len){
		usleep(1000);
		printf("\namf_usb: Failed (Write %d:%d bytes)\n", q.len, len);
		//if (++cnt < 10) goto write_data_again;
		return -EINVAL;
	}

	/* poll for ready */
	cnt = 0;
write_data_poll_again:
	q.len	= 0;					/* Ready? - wait for tx to complete */
	q.ret	= 0;
	err = ioctl(fd, AMF_FW_DATA_WRITE, &q);
	if (err){
		printf("write poll: err=%X)\n", err);
		return -EINVAL;
	}

	if (q.ret){
		usleep(10);
		if (++cnt < 1000) goto write_data_poll_again;
		printf("\namf_usb: Failed: tx timeout\n");
		return -EINVAL;
	}

	return 0;
}

int exec_usb_cpu_read(int fd, int off, unsigned char *data, unsigned short len)
{

	int err;
	int cnt = 0;

	if (!len) return -EINVAL;
	memset(q.data, 0, AMF_MAX_CMD_DATA_SIZE);

	q.len = len;
	q.offset = off;

try_read_again:
	q.ret	= 0;			// Original call uses ret as mod_no
	q.bar	= 0;
	q.len	= len;
	q.offset	= off;
	err = ioctl(fd, AMF_CPU_READ_REG, &q);
	if (err){
		printf("\namf_usb: cpu_read...err=%X\n", err);
		return -EINVAL;
	}

	if (q.ret || q.len != len){
		usleep(100);
		if (++cnt < 100) goto try_read_again;
		printf("\namf_usb: Failed (rx timeout:%d:%d) ...exit now!\n", q.len, len);
		exit(1);
		return -EINVAL;
	}

	memcpy(data, &q.data[0], q.len);
	return 0;

}


int exec_usb_cpu_write(int fd, int off, unsigned char *data, unsigned short len)
{
	int err, cnt;

	memset(q.data, 0, AMF_MAX_CMD_DATA_SIZE);
	cnt = 0;

	q.ret	= 0;
	q.len	= len;
	q.offset	= off;
	memcpy(&q.data[0], data, len);
	err = ioctl(fd, AMF_CPU_WRITE_REG, &q);
	if (err){
		printf("\namf_usb: cpu_write...err=%X\n", err);
		return -EINVAL;
	}
	if (q.ret){
		usleep(1000);
		printf("\namf_usb: Failed (Write %d:%d bytes)\n",  q.ret, len);
		return -EINVAL;
	}
	return 0;
}

static int print_help(void)
{
	print_version();
	printf("Usage:\n"); 
	printf("======\n"); 
	printf(" ausb -help          - Print help message\n");
	printf(" ausb -fwupdate      - Update Firmware from default file location\n");
	printf(" ausb -readstatusreg - Read status register from CPU\n");
	printf("\n");
	return 0;
}

static int print_version(void)
{
	printf("\n");
	printf("\n");
	printf("******************************************************\n");
	printf("***      Amfeltec USB Firmware Update utility      ***\n"); 
	printf("******************************************************\n");
	printf("\n");
	return 0;
}


int get_file(char *filename, char *prefix){

	char		ver[10];
	DIR		*dir;
	struct dirent	*ent;

	int  i = 0;
	int  version = 0;
	firmware_files_t		firmware_files[100];
	char ver_sel[100];

	memset(firmware_files, 0, sizeof(firmware_files));

	if (!(dir = opendir("."))){
		perror("opendir");
		return EINVAL;
	} else {
	}


	while((ent = readdir(dir))){

		if (strncmp(ent->d_name, prefix, strlen(prefix)) == 0){

			i = strlen(prefix);
			while(!isdigit(ent->d_name[i])) {
				 i++;
			}
			version = (ent->d_name[i]-'0') * 10 + (ent->d_name[i+1]-'0');

			firmware_files[version].version = version;
			strncpy(firmware_files[version].file, ent->d_name, strlen(ent->d_name));
		}
	}

	printf("\nAvailable firmware files:\n");
	for (i = 0; i < 100; i++) {
		if (firmware_files[i].version != 0) {
			printf("  file: %s, version: %d\n", firmware_files[i].file, firmware_files[i].version);
		}
	}

getfile_again:

	printf("\nPlease type version number or file name (q for exit) > ");
	if (scanf("%s", ver_sel)){

		if (strcmp(ver_sel, "q") == 0){
			return -EINVAL;
		}
		if (strlen(ver_sel) < 3){
			if (strlen(ver_sel) == 2){
				strncpy(ver, ver_sel, strlen(ver_sel));
				ver[2] = '\0';
				version = atoi(ver);
			}else if (strlen(ver_sel) == 1){
				ver[0] = '0';
				ver[1] = ver_sel[0];
				ver[2] = '\0';
				version = atoi(ver);
			}
			for (i = 0; i < 100; i++) {
				if ((version != 0) && (firmware_files[i].version == version)) {
					strncpy(filename, firmware_files[i].file, strlen(firmware_files[i].file));
					printf("\nSelected firmware file:\n");
					printf("  file: %s, version: %d\n", firmware_files[i].file, firmware_files[i].version);
					return 0;
				}
			}

		} else {

			if (!(dir = opendir("."))){
				perror("opendir");
				return EINVAL;
			} else {
				while((ent = readdir(dir))) {
					if (strncmp(ent->d_name, ver_sel, strlen(ver_sel)) == 0){
						strncpy(filename, ent->d_name, strlen(ent->d_name));
						printf("  selected file: %s\n", ent->d_name);									
						return 0;
					}
				}
			}
		}
		goto getfile_again;
	}
	return 0;
}

