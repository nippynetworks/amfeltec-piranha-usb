 /*
  *  amf_usb.h - 'driver types and definitions'
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

#ifndef __AMF_USB_H__
#define __AMF_USB_H__

#include "amf_usb_utils.h"
#include "dahdi/kernel.h"
#include <linux/list.h>

//=================================================================================

#define	AMF_USB_DRIVER_VERSION "4.0.4"

#define AMF_USB_NAME					"amf_usb"
#define AMF_USB_VENDORID				0x10C4
#define AMF_USB_PRODUCTID_FXO			0x88F4
#define AMF_USB_PRODUCTID_FXS			0x88F6
#define AMF_USB_PRODUCTID_PA			0x88F8

#define AMF_USB_MODE_MASTER
//#define AMF_USB_MODE_SLAVE

 /* settings for the 'adapter_type' */
enum {
         F100_ADAPTER = 0x01,	/* F100 */
         F120_ADAPTER			/* F120 */
};

#define AMF_USB_FXO						1
#define AMF_USB_FXS						2
#define AMF_USB_PA						3

#define AMF_USBFXO_SYNC_DELAY			10
#define AMF_USBFXO_SYNC_RX_RETRIES		100
#define AMF_USBFXO_SYNC_TX_RETRIES		400
#define AMF_USBFXO_READ_DELAY			20
#define AMF_USBFXO_READ_RETRIES			100
#define AMF_USBFXO_WRITE_DELAY			10

#define AMF_USBFXS_WRITE_INDIRECT_DELAY		110		//max. 110 ms + 4 retires
#define AMF_USBFXS_WRITE_INDIRECT_RETRIES	4
#define AMF_USBFXS_READ_INDIRECT_DELAY		20		//max. 1 sec
#define AMF_USBFXS_READ_INDIRECT_RETRIES	50

#define AMF_USB_STATUS_ATTACHED			1
#define AMF_USB_STATUS_READY			2
#define AMF_USB_STATUS_TX_READY			3
#define AMF_USB_STATUS_RX_EVENT1_READY	4
#define AMF_USB_STATUS_RX_EVENT2_READY	5
#define AMF_USB_STATUS_RX_DATA_READY	6
#define AMF_USB_STATUS_BH				7
#define AMF_USB_STATUS_TX_CMD			8

#define AMF_USB_LSTATUS_GETDEV			1
#define AMF_USB_LSTATUS_TASKLET			2
#define AMF_USB_LSTATUS_WRITEBUF		3
#define AMF_USB_LSTATUS_READBUF			4
#define AMF_USB_LSTATUS_RXCMD			5
#define AMF_USB_LSTATUS_TXCMD			6
#define AMF_USB_LSTATUS_INTF			7
#define AMF_USB_LSTATUS_URB				8
#define AMF_USB_LSTATUS_DAA				9
#define AMF_USB_LSTATUS_PROSLIC			10
#define AMF_USB_LSTATUS_DAHDI			11
#define AMF_USB_LSTATUS_DAHDI_MASTER	12
#define AMF_USB_LSTATUS_MISC			13


#define AMF_USB_STATUS_FE_RW			1

#define AMF_URB_STATUS_READY			1

#define AMF_USB_IDLE_PATTERN			0x7E
#define AMF_USB_CTRL_IDLE_PATTERN		0x7E

#define AMF_USB_MAX_RX_CMD_QLEN			20
#define AMF_USB_MAX_TX_CMD_QLEN			20


#define AMF_URB_AMOUNT				16
#define AMF_URB_BULK_BUF_SIZE			64
//#define AMF_URB_BULK_BUF_SIZE                   (MAX_USB_RX_LEN * 16)

#define	SYNC_SIZE				3

//=================================================================================

#define MAX_USB_DEVICES			128
#define MAX_USB_MODULES			1

#define AMF_USB_CHUNKSIZE		8
#define AMF_USB_CHUNKS_PER_PACKET	6
#define AMF_USB_DEFAULT_CHUNK_SIZE AMF_USB_CHUNKS_PER_PACKET

#define AMF_USB_FRAMESCALE		1
#define AMF_USB_RXTX_CHUNKSIZE	(AMF_USB_CHUNKSIZE*AMF_USB_FRAMESCALE)	// 8
#define AMF_USB_MAX_CHUNKSIZE	(AMF_USB_CHUNKSIZE*AMF_USB_FRAMESCALE)	// 8

#define MAX_READ_URB_COUNT		1										// 1
#define MAX_WRITE_URB_COUNT		2										// Setting to 1: transmitter error (both KERNEL/ATOMIC)!
#define MAX_USB_RX_LEN			64										// (4*AMF_USB_RXTX_CHUNKSIZE)              // 32
#define MAX_USB_TX_LEN			64										// (4*AMF_USB_RXTX_CHUNKSIZE)              // 32
#define MAX_READ_BUF_LEN		(MAX_USB_RX_LEN*100)					// 30*4
#define MAX_WRITE_BUF_LEN		(MAX_USB_TX_LEN*100)					// 30*4


//====================================================== Internal CPU registers

// Hardware ID: high nibble is Device type: FXO:1 FXS:2 PA:3, low is a Rotary Switch
#define AMF_USB_CPU_REG_DEVICEID				0x00

#define AMF_USB_CPU_REG_HARDWAREVER				0x01
#define AMF_USB_CPU_REG_FIRMWAREVER				0x02

#define AMF_USB_CPU_REG_CTRL					0x03
#define AMF_USB_CPU_BIT_CTRL_RESET				0x80
#define AMF_USB_CPU_BIT_CTRL_RESET_ALL			0x83		// Including SLIC framer
#define AMF_USB_CPU_BIT_CTRL_FWUPDATE			0x40
#define AMF_USB_CPU_BIT_CTRL_TS1_HWEC_EN		0x08
#define AMF_USB_CPU_BIT_CTRL_TS0_HWEC_EN		0x04
#define AMF_USB_CPU_BIT_CTRL_TS1_EVENT_EN		0x02
#define AMF_USB_CPU_BIT_CTRL_TS0_EVENT_EN		0x01

#define AMF_USB_CPU_REG_FIFO_STATUS		0x04
#define AMF_USB_CPU_BIT_FIFO_STATUS_TS1_TX_UF	0x80
#define AMF_USB_CPU_BIT_FIFO_STATUS_TS1_TX_OF	0x40
#define AMF_USB_CPU_BIT_FIFO_STATUS_TS0_TX_UF	0x20
#define AMF_USB_CPU_BIT_FIFO_STATUS_TS0_TX_OF	0x10
#define AMF_USB_CPU_BIT_FIFO_STATUS_TS1_RX_UF	0x08
#define AMF_USB_CPU_BIT_FIFO_STATUS_TS1_RX_OF	0x04
#define AMF_USB_CPU_BIT_FIFO_STATUS_TS0_RX_UF	0x02
#define AMF_USB_CPU_BIT_FIFO_STATUS_TS0_RX_OF	0x01

#define AMF_USB_CPU_REG_UART_STATUS		0x05
#define AMF_USB_CPU_BIT_UART_STATUS_LOST_SYNC	0x10
#define AMF_USB_CPU_BIT_UART_STATUS_CMD_UNKNOWN	0x10
#define AMF_USB_CPU_BIT_UART_STATUS_RX_UF		0x08
#define AMF_USB_CPU_BIT_UART_STATUS_RX_OF		0x04
#define AMF_USB_CPU_BIT_UART_STATUS_TX_UF		0x02
#define AMF_USB_CPU_BIT_UART_STATUS_TX_OF		0x01

#define AMF_USB_CPU_REG_HOSTIF_STATUS	0x06
#define AMF_USB_CPU_BIT_HOSTIF_STATUS_RX_UF		0x08
#define AMF_USB_CPU_BIT_HOSTIF_STATUS_RX_OF		0x04
#define AMF_USB_CPU_BIT_HOSTIF_STATUS_TX_UF		0x02
#define AMF_USB_CPU_BIT_HOSTIF_STATUS_TX_OF		0x01

#define AMF_USB_CPU_REG_LED_CONTROL		0x07
#define AMF_USB_CPU_BIT_LED_CONTROL_TS1_GRN		0x08
#define AMF_USB_CPU_BIT_LED_CONTROL_TS1_RED		0x04
#define AMF_USB_CPU_BIT_LED_CONTROL_TS0_GRN		0x02
#define AMF_USB_CPU_BIT_LED_CONTROL_TS0_RED		0x01

#define AMF_USB_CPU_REG_DEBUG					0x08
#define AMF_USB_CPU_BIT_DEBUG_WEN_ACK	0x08
#define AMF_USB_CPU_BIT_DEBUG_DTMF		0x04
#define AMF_USB_CPU_BIT_DEBUG_RINGTEST			0x02
#define AMF_USB_CPU_BIT_DEBUG_LOOPBACK			0x01

#define AMF_USB_CPU_REG_EC_NUM			0x09

#define AMF_USB_CPU_REG_WATCHDOG				0x0A
#define AMF_USB_CPU_WATCHDOG_RESTART			0x02

#define AMF_USB_CPU_REG_FAILOVER_RELAY			0x0B
#define AMF_USB_CPU_FAILOVER_RELAY_ON			0x02
#define AMF_USB_CPU_FAILOVER_WATCHDOG_ON		0x01

#define list_last_entry(ptr, type, member) \
         list_entry((ptr)->prev, type, member)

#define list_last_entry_or_null(ptr, type, member) \
         (!list_empty(ptr) ? list_last_entry(ptr, type, member) : NULL)

#define WR_CPU_COMMAND AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_WRITE_CPU)
#define RD_CPU_COMMAND AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_READ_CPU)
#define WR_FXO_COMMAND AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_WRITE_FXO)
#define RD_FXO_COMMAND AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_READ_FXO)
#define WR_FXS_COMMAND AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_WRITE_FXS_INDIRECT)
#define RD_FXS_COMMAND AMF_USB_CMD_TYPE_ENCODE(AMF_USB_CMD_TYPE_READ_FXS_INDIRECT)

#define NO_COMMAND 	AMF_USB_CTRL_IDLE_PATTERN


#define STATUS_BITS	8

//====================================================== FXO

#define RING_DEBOUNCE           16     /* Ringer Debounce (64 ms) */
#define DEFAULT_BATT_DEBOUNCE   16     /* Battery debounce (64 ms) */
#define POLARITY_DEBOUNCE       16     /* Polarity debounce (64 ms) */
#define DEFAULT_BATT_THRESH     3      /* Anything under this is "no battery" */
#define DEFAULT_OH_THRESH       10      /* Anything under this is "off-hook" */
#define OHT_TIMER               6000    /* How long after RING to retain OHT */

#define FXO_LINK_DEBOUNCE       400		//200 (as for Polarity Reverse Issue)

#define MAX_ALARMS              10
#define FXO_LINK_THRESH         1      /* fxo link threshold */

#define FXO_DISCONNECTED		0x01
#define FXO_CONNECTED			0x02


#define packed_data __attribute__((packed, aligned(1)))

typedef enum{
	INIT_STATE,
	OPERATION_STATE,
	OPERATION_STOP
}main_state_t;


struct amf_urb {
	void			*pvt;
	int			id;
	int			indx;					// To maintain compatibility, proide reverce indexing
	int			next_off;
	unsigned int		ready;
	struct urb		urb;
};


typedef struct sk_buff_head	amf_skb_queue_t;
/*
176 struct sk_buff_head {
177         / * These two members must be first. * /
178         struct sk_buff  *next;
179         struct sk_buff  *prev;
180
181         __u32           qlen;
182         spinlock_t      lock;
183 };
*/



//====================================================  STATS
typedef struct {
	unsigned long		core_notready_cnt;
	unsigned long		cmd_overrun;
	unsigned long		cmd_timeout;
	unsigned long		cmd_invalid;
	unsigned long		rx_sync_err_cnt;
	unsigned long		rx_start_fr_err_cnt;
	unsigned long		rx_start_err_cnt;
	unsigned long		rx_cmd_reset_cnt;
	unsigned long		rx_cmd_drop_cnt;
	unsigned long		rx_cmd_unknown;
	unsigned long		rx_overrun_cnt;
	unsigned long		rx_underrun_cnt;
	unsigned long		tx_overrun_cnt;
	unsigned long		tx_underrun_cnt;
	unsigned long		tx_notready_cnt;
	unsigned long		tx_endpointstall_cnt;
	unsigned long		rx_endpointstall_cnt;
	unsigned char		dev_fifo_status;
	unsigned char		dev_uart_status;
	unsigned char		dev_hostif_status;
	unsigned long		dev_sync_err_cnt;
	unsigned long		mcpu_status_err_cnt;
	unsigned long		mcpu_queue_threshold_err_cnt;
	unsigned long		slic_indirect_access_err_cnt;
} amf_usb_comm_err_stats_t;

//====================================================
struct amf_usb_desc {
        char	*name;
        int	adptr_type;
};


typedef struct fxo_
{
	int     ready;
	u8      status;                 /* line status (connected/disconnected) */
	int	statusdebounce;         /* line status debounce */
    	int	ring_detect;
    	int	offhook;                /* Xswitch */
    	int	onhook;					// ZERO_BATT_RING special
    	int     battery;                /* Xswitch */
    	int     battdebounce;           /* Xswitch */
    	int	i_debounce;
    	int	ringdebounce;
    	int     wasringing;
    	int     lastpol;
    	int	polarity;
    	int    	polaritydebounce;
    /* Variables required for tapper mode */
	int     ohdebounce;
	int     going_offhook;  /* current ohdebounce is for off-hk or on-hk */
	unsigned char   imask;          /* interrupt mask */
	int                             readcid;
	unsigned int                    cidtimer;
    /*Additional for Zaptel mode*/
	int                             echotune;       /* echo tune */
	//struct wan_rm_echo_coefs        echoregs;       /* echo tune */
} amf_fxo_t;

//============================== indirect registers
typedef struct {
	unsigned char address;
	unsigned char altaddr;
	char *name;
	unsigned short initial;
} alpha;

enum proslic_power_warn {
	PROSLIC_POWER_UNKNOWN = 0,
	PROSLIC_POWER_ON,
	PROSLIC_POWER_WARNED,
};

// Number of Calibration registers
#define NUM_CAL_REGS            12

typedef struct {
        unsigned char vals[NUM_CAL_REGS];
} callregs_t;

typedef struct fxs_
{
        int     ready;
        int     lasttxhook;
        int     lasttxhook_update;
        int     lastrxhook;
        int     oldrxhook;
        int     debouncehook;
        int     debounce;
        int     palarms;
        int     ohttimer;
		int		idletxhookstate;
		enum proslic_power_warn		proslic_power;
        callregs_t      			callregs;
} amf_fxs_t;

typedef struct packed_data cpu_reg_cmd
{
        u8 ctrl;
        u8 addr;
        u8 low;
        u8 high;
} cpu_reg_cmd_t;


typedef struct packed_data packet_64
{
        u8              sync[SYNC_SIZE];
        u8              mcpu_status;
        cpu_reg_cmd_t   cmd;
        u8              data[AMF_USB_CHUNKS_PER_PACKET][AMF_USB_CHUNKSIZE];
        u8              status[6];
        u8              mcpu_status_2;
        u8              chsum_lo;
//      u8              chsum_hi;

}packet_64_t;

typedef struct reg_access_item
{
	struct list_head	list;	
	struct cpu_reg_cmd	cmd;
}reg_access_item_t;

typedef struct reg_access_head
{
	struct list_head	head;
	spinlock_t		list_lock;
}reg_access_head_t;

typedef struct queue_head
{
	struct list_head	head;
	spinlock_t		list_lock;
}queue_head_t;

typedef struct audio_chunk
{
	struct list_head list;
	u8 chunk[AMF_USB_CHUNKSIZE];
}audio_chunk_t;



//===============================================================================================  amf_usb_t

typedef struct amf_usb
{

	struct usb_device		*udev;
	struct usb_interface 		*uintf;
	unsigned int 			tx_pipe;
	int				amf_device_type;				// FXO/FXS...
	u8				partnum;					// CP210X_PARTNUM
	unsigned int			num;						// amf_usb device number
	int				order;						// by rotary switch
	int				firmwareupdate;					// if not 0: firmware update mode
	int				failover_enabled;				// hardware version with fail-over relay
	char				bus_id[20];
	unsigned long			lstatus;					// load status
	unsigned int			status;
	unsigned int			mod_cnt;
	unsigned int			status_hist[STATUS_BITS];
	unsigned int			status_cnt;

	int						time_to_reset_fifo_pointers;

	struct dahdi_span 		span;
	struct dahdi_span 		span_master;
	int				master_span_on;             		        // 0: works as hardware-master, 1: Master span is on, data transfer in Slave mode
	struct dahdi_chan 		chans[MAX_USB_MODULES];
	struct dahdi_chan 		*chan_ptrs[MAX_USB_MODULES];

	char				master_data[4];

#if DAHDI_VERSION >= 26
	struct dahdi_device		*ddev;
	struct dahdi_device		*ddev_master;
#endif
	char   				device_name[20];
	char   				device_name_master[20];

	unsigned char 			lasttx[MAX_USB_MODULES][DAHDI_CHUNKSIZE];
	// FXO/FXS...
	int 				mod_type[MAX_USB_MODULES];
	union {
		amf_fxo_t		fxo;
		amf_fxs_t		fxs;
	} fx[MAX_USB_MODULES];

	int				alawoverride;
	int				opermode;
	int				battdebounce;
	int				battthresh;
	int				battalarm;
	int				reversepolarity;
	int		 		ohthresh;
	u_int8_t			relaxcfg;
												// global for FXO
	int				fxo_txgain;
	int				fxo_rxgain;
												// global for FXS
	u_int8_t			fastringer;
	u_int8_t			lowpower;
	u_int8_t			ringampl;
	u_int8_t			pulsedialing;
	int				fxs_rxgain;
	int				fxs_txgain;

	int				dead;
	int				usecount;
	int				chunk_size;

	struct amf_urb			urbread[MAX_READ_URB_COUNT][AMF_URB_AMOUNT];		// RX URB
//	struct amf_urb			urbwrite[MAX_WRITE_URB_COUNT][AMF_URB_AMOUNT];		// TX URB
	#define	urbread_to_usb(urb) container_of(urb, struct amf_usb, urbread)
	#define	urbwrite_to_usb(urb) container_of(urb, struct amf_usb, urbread)


	char				*readbuf;						// RX buffer:
	int				rx_in;					//				in:		USB to Buffer
	int				rx_out;					//				out:	Buffer to DAHDI

	char				*writebuf;						// TX buffer:
	int				tx_in;							//				in:		DAHDI to Buffer
	int				tx_out;							//				out:	Buffer to USB

	int				rx_sync;
	int				rx_len;
	int				tx_sync;

	char				*readbuf_block[AMF_URB_AMOUNT];
//        dma_addr_t                      readbuf_block_dma[AMF_URB_AMOUNT];

//	char				writebuf_block[MAX_USB_TX_LEN][AMF_URB_BULK_BUF_SIZE];

	unsigned char			hw_rev;			// hardware (pcb) revision

	unsigned char			reg_cpu_ctrl;		// Reg 0x03

	unsigned char			core_id;	/* SubSystem ID [0..7] */
	unsigned char			core_rev;	/* SubSystem ID [8..15] */

	int				urbcount_read;
	int				urb_read_ind;

	int				urbcount_write;
	int				urb_write_ind;

	char				readchunk[MAX_USB_MODULES][AMF_USB_MAX_CHUNKSIZE * 2+1];
	char				writechunk[MAX_USB_MODULES][AMF_USB_MAX_CHUNKSIZE * 2+1];

	u8				regs[MAX_USB_MODULES][130];			// registers shadow, 119+ reserved for MSCPU Status

	spinlock_t			cmd_lock;
	spinlock_t			lock;
	spinlock_t			tx_lock;

	struct tasklet_struct		bh_task;



	unsigned char			ctrl_idle_pattern;
	unsigned long			tx_cmd_start;

	amf_skb_queue_t		tx_cmd_list;
	amf_skb_queue_t		tx_cmd_free_list;
	amf_skb_queue_t		rx_cmd_list;
	amf_skb_queue_t		rx_cmd_free_list;

	void		(*isr_func)(void*);
	void		*isr_arg;

	unsigned long		bhcount;
	unsigned long 		isrcount;
	unsigned long 		txcount;
	unsigned long 		rxcount;
	unsigned long 		rxbytes;
	unsigned long 		dahdi_ticks;
	unsigned long 		tasklet_sc;
	unsigned long 		usb_master;
	unsigned long 		usb_masterspan;
	unsigned long 		temperature;
	int					temperature_max;

	amf_usb_comm_err_stats_t	stats;

 	queue_head_t		dahdi_rx_queue;
	queue_head_t		dahdi_tx_queue;
	queue_head_t		usb_tx_queue;

	void* context;
	u32		chunk_amount;

	struct reg_access_head reg_access_tx_head;
	struct reg_access_head reg_access_rx_head;
	wait_queue_head_t wait_read;


        u32 incoming;
        u32 handled;
	main_state_t main_state;
//	u8 test_cnt;
} amf_usb_t;


typedef struct usb_tx_urb
{
	struct list_head list;
	struct urb	urb;
	u8		data[MAX_USB_TX_LEN];
	amf_usb_t	*ausb;

}usb_tx_urb_t;


void wait_just_a_bit(int ms, int fast);
int amf_usb_init(void);
void amf_usb_exit(void);
void amf_usb_bh(unsigned long data);
int __amf_usb_cpu_write(amf_usb_t *ausb, unsigned char off, unsigned char data);
int __amf_usb_cpu_read(amf_usb_t *ausb, unsigned char off, unsigned char *data);
int __amf_usb_cpu_init_proslic(amf_usb_t *ausb, unsigned char tx_data, unsigned char *rx_data);
int __amf_usb_fxo_write(amf_usb_t *ausb, int mod_no, unsigned char off, unsigned char data);
u_int8_t __amf_usb_fxo_read(amf_usb_t *ausb, int mod_no, unsigned char off);
int amf_usb_start_transfer(struct amf_urb *aurb);
int amf_usb_rxtx_buffers_alloc(amf_usb_t *ausb);
int amf_usb_start_uart(amf_usb_t *ausb);
int amf_usb_stop_uart(amf_usb_t *ausb);
void amf_usb_set_urb_status_ready(amf_usb_t *ausb);
int amf_usb_set_config(amf_usb_t *usb, u8 request, int value, unsigned int *data, int size);
int amf_usb_set_config_single(amf_usb_t *usb, u8 request, int value, unsigned int data);
int amf_usb_get_config(amf_usb_t *usb, u8 request, int value, unsigned int *data, int size);
void amf_usb_set_transfer_urbs(struct usb_interface *interface);
int amf_usb_search_rxsync(amf_usb_t *ausb);
int amf_usb_search_txsync(amf_usb_t *ausb);
int amf_usb_set_cpu(amf_usb_t *ausb);
int amf_usb_create(struct usb_interface*, int);
int amf_usb_destroy(struct usb_interface*);

int amf_usb_dahdi_register(amf_usb_t *ausb);
int amf_usb_dahdi_unregister(amf_usb_t *ausb);
int amf_usb_dahdi_register_master(amf_usb_t *ausb);
int amf_usb_dahdi_unregister_master(amf_usb_t *ausb);

int amf_usb_module_detect(amf_usb_t *);
void amf_voicedaa_check_hook(amf_usb_t *ausb, int mod_no);
void amf_proslic_check_hook(amf_usb_t *ausb, int mod_no);
void amf_proslic_check_ohttimer(amf_usb_t *ausb, int mod_no);
void amf_voicedaa_recheck_sanity(amf_usb_t *ausb, int mod_no);
void amf_proslic_recheck_sanity(amf_usb_t *ausb, int mod_no);
int amf_usb_init_daa(amf_usb_t *ausb, int mod_no, int fast, int sane);
int amf_usb_init_proslic(amf_usb_t *ausb, int mod_no, int fast, int sane);
int	amf_usb_set_opermode(amf_usb_t *ausb, char *opermode);

int amf_usb_txdata_raw(amf_usb_t *ausb, unsigned char *data, int max_len);
int amf_usb_txdata_raw_ready(amf_usb_t *ausb);
int amf_usb_rxdata_raw(amf_usb_t *ausb, unsigned char *data, int max_len);
int amf_usb_rxurb_reset(amf_usb_t *ausb);
int amf_usb_txurb_reset(amf_usb_t *ausb);

void amf_usb_isr(void *arg);
int amf_usb_rxdata_enable(amf_usb_t *ausb, int enable);
int amf_usb_rxevent_enable(amf_usb_t *ausb, int mod_no, int enable);
int amf_usb_rxevent(amf_usb_t *ausb, int mod_no, int force);

int amf_set_driver_started(void);
int amf_get_driver_started(void);

void amf_dahdi_sync_tick(struct dahdi_span *span, int is_master);
int amf_usb_syncverify(amf_usb_t *ausb, int len);

int amf_usb_txdata_prepare(amf_usb_t *ausb, char *writebuf, int len);
void wait_for_sync(amf_usb_t *ausb, int n);
int create_packet(cpu_reg_cmd_t* cmd, u8 (*data)[AMF_USB_CHUNKS_PER_PACKET]);
int amf_usb_txdata_send_64(amf_usb_t *ausb);
int amf_usb_create_packet_64(amf_usb_t *ausb);
void amf_usb_init_read_write(amf_usb_t *ausb);
//struct reg_access_item*  amf_usb_create_command(u8 command, u8 addr, u8 low, u8 high);
int amf_usb_read_value(u8 cmd, amf_usb_t *ausb, u8 address, int timeout);
int amf_usb_write_value(u8 cmd, amf_usb_t *ausb, u8 address, u8 low, u8 high, int timeout);
int amf_usb_cmd_xtactor(amf_usb_t *ausb, struct cpu_reg_cmd* cmd);

struct reg_access_item*  amf_usb_create_command(u8 command, u8 addr, u8 low, u8 high);


void add_cmd_queue(struct reg_access_item* ra_item, struct reg_access_head *head);
void add_cmd_fill_data(struct reg_access_item* ra_item, struct reg_access_head *head);
struct reg_access_item *get_cmd_from_queue(struct reg_access_head* head);
int is_list_empty(queue_head_t *queue);
int count_queue_elements(queue_head_t *queue);


void print_dahdi_call(struct amf_usb* ausb, char* method);




//===================================== ENCODE / DECODE ===========================================

#define AMF_USB_CMD_TYPE_MASK				0x3C           	//1C
#define AMF_USB_CMD_TYPE_SHIFT				2
#define AMF_USB_CMD_TYPE_READ_FXO			0x01
#define AMF_USB_CMD_TYPE_WRITE_FXO			0x02
#define AMF_USB_CMD_TYPE_EVENT				0x03
#define AMF_USB_CMD_TYPE_READ_CPU			0x04
#define AMF_USB_CMD_TYPE_WRITE_CPU			0x05
#define AMF_USB_CMD_TYPE_WRITE_FXS_INDIRECT	0x06
#define AMF_USB_CMD_TYPE_READ_FXS_INDIRECT	0x07
#define AMF_USB_CMD_TYPE_CPU_INIT_PROSLIC	0x08

#define AMF_USB_CMD_TYPE_DECODE(ctrl)	(((ctrl) & AMF_USB_CMD_TYPE_MASK) >> AMF_USB_CMD_TYPE_SHIFT)
#define AMF_USB_CMD_TYPE_ENCODE(cmd)	((cmd) << AMF_USB_CMD_TYPE_SHIFT)

#define AMF_USB_CTRL_DECODE(buf)														\
						((buf)[0] & 0xC0) | (((buf)[1] & 0xC0) >> 2) |					\
						(((buf)[2] & 0xC0) >> 4) | (((buf)[3] & 0xC0) >> 6)

#define AMF_USB_CTRL_ENCODE(buf, ind)													\
	if (ind % 4 == 0 && ind % 32 == 0) *((buf)+ind) = (*((buf)+ind) & 0xCF) | 0x30;		\
	else if (ind % 4 == 0)  *((buf)+ind) = (*((buf)+ind) & 0xCF) | 0x10;

#define AMF_USB_CTRL_ENCODE1(buf, ctrl)													\
						*(buf)     = (*(buf) & 0xCF)     | (ctrl);

#define AMF_USB_DATA_ENCODE(buf, data)													\
						*(buf)     = (*(buf) & 0xF0)     | ((data) & 0x0F);				\
						*((buf)+2) = (*((buf)+2) & 0xF0) | (((data) >> 4) & 0x0F);

#define AMF_USB_CMD_ENCODE(buf, cmd)													\
						*(buf)     = (*(buf) & 0x3F)     | ((cmd) & 0xC0);				\
						*((buf)+1) = (*((buf)+1) & 0x3F) | (((cmd) & 0x30) << 2);		\
						*((buf)+2) = (*((buf)+2) & 0x3F) | (((cmd) & 0x0C) << 4);		\
						*((buf)+3) = (*((buf)+3) & 0x3F) | (((cmd) & 0x03) << 6);


//===================================== ENCODE / DECODE. ===========================================

extern int kdeb_arr[];

#if 0
#define KMALLOC(p1, p2, p3) kmalloc(p1,p2); kdeb_arr[p3]++
#define KFREE(p1, p2) kfree(p1);  kdeb_arr[p2]--
#define MARK_UP(x)  kdeb_arr[x]++
#define MARK_DN(x)  kdeb_arr[x]--
#else
#define KMALLOC(p1, p2, p3) kmalloc(p1,p2)
#define KFREE(p1, p2) kfree(p1) 
#define MARK_UP(x)   
#define MARK_DN(x)   

#endif

#endif  //__AMF_USB_H__
