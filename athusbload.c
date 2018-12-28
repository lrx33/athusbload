#include<libusb.h>
#include<stdio.h>
#include<assert.h>
#include<unistd.h>
#include<fcntl.h>

#define TIMEOUT 2000
#define AR9271_FIRMWARE       0x501000U
#define AR9271_FIRMWARE_TEXT  0x903000U
#define FW_DOWNLOAD 0x30
#define FW_DOWNLOAD_DONE 0x31
#define FWLOC "/home/bob/open-ath9k-htc-firmware/target_firmware/htc_9271.fw"
#define FWMAXLEN 80000

int main(int argc, char *argv[]) {

    if(argc != 2) {
	printf("Usage: %s <firmware-file>\n", argv[0]);
	return 1;
    }

    uint16_t idVendor = 0x0cf3;
    uint16_t idProduct = 0x9271;
    libusb_context *libctx = NULL;

    int libinit = libusb_init(&libctx);
    assert(libinit == 0);

    /* Read firmware into buf from fw file */
    uint8_t fwdata[FWMAXLEN];
    uint32_t fwlen = 0;

    int fwfd = open(argv[1], O_RDONLY);
    assert(fwfd != -1);
    while(1) {
	ssize_t rcnt = read(fwfd, fwdata + fwlen, FWMAXLEN - fwlen);
	if(rcnt == 0)
	    break;

	fwlen += rcnt;
    }
    close(fwfd);

    printf("fwlen = %u\n", fwlen);

    /* Open USB Device and write out firmware */
    libusb_device_handle *usbdev = libusb_open_device_with_vid_pid(libctx, idVendor, idProduct);
    assert(usbdev != NULL);

    uint32_t memaddr = AR9271_FIRMWARE;
    uint32_t fwrem = fwlen;
    uint32_t fwdone = 0;

    while(fwrem) {

	uint32_t chunksz = fwrem > 4096 ? 4096 : fwrem;
	int ret = libusb_control_transfer(usbdev, 0x40, FW_DOWNLOAD, memaddr >> 8, 0, fwdata + fwdone, chunksz, TIMEOUT);

	printf("Transferred ret=%d bytes of firmware.\n", ret);

	assert(ret > 0);

	memaddr += chunksz;
	fwrem -= chunksz;
	fwdone += chunksz;
    }

    int ret = libusb_control_transfer(usbdev, 0x40, FW_DOWNLOAD_DONE, AR9271_FIRMWARE_TEXT >> 8, 0, NULL, 0, TIMEOUT);
    printf("Firmware loaded: ret=%d\n", ret);

    libusb_close(usbdev);
    libusb_exit(libctx);

    return 0;
}

    /* int libusb_control_transfer(libusb_device_handle *devh, uint8_t */
    /* bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, */
    /* unsigned char *data, uint16_t wLength, unsigned int timeout) Perform a */

    /* err = usb_control_msg(hif_dev->udev, usb_sndctrlpipe(hif_dev->udev, 0), */
    /* 		      FIRMWARE_DOWNLOAD_COMP, */
    /* 		      0x40 | USB_DIR_OUT, */
    /* 		      firm_offset >> 8, 0, NULL, 0, USB_MSG_TIMEOUT); */



/* Parameters */
/*     dev_handle	a handle for the device to communicate with */
/*     bmRequestType	the request type field for the setup packet */
/*     bRequest	the request field for the setup packet */
/*     wValue	the value field for the setup packet */
/*     wIndex	the index field for the setup packet */
/*     data	a suitably-sized data buffer for either input or output (depending on direction bits within bmRequestType) */
/*     wLength	the length field for the setup packet. The data buffer should be at least this size. */
/*     timeout	timeout (in millseconds) that this function should wait before giving up due to no response being received. For an unlimited timeout, use value 0. */


/* 	int usb_control_msg(struct usb_device * dev, unsigned int pipe, __u8 request, __u8 requesttype, __u16 value, __u16 index, void * data, __u16 size, int timeout); */

/*     err = usb_control_msg( */
/* 	hif_dev->udev, */
/* 	usb_sndctrlpipe(hif_dev->udev, 0), */
/* req	FIRMWARE_DOWNLOAD, */
/* reqt	0x40 | USB_DIR_OUT, */
/* 	addr >> 8, */
/* 	0, */
/* 	buf, */
/* 	transfer, */
/* 	USB_MSG_TIMEOUT); */

/* ARGUMENTS */
/* dev pointer to the usb device to send the message to */
/* pipeEndpoint “pipe” to send the message to */
/* request USB message request value */
/* requesttype USB message request type value */
/* value USB message value */
/* index USB message index value */
/* data pointer to the data to send */
/* size length in bytes of the data to send */
/* timeout time in msecs to wait for the message to complete before timing out (if 0 the wait is forever) */
