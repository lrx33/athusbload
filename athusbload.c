/*
  Load firmware for AR9271 over USB
*/

#include<libusb.h>
#include<stdio.h>
#include<assert.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/mman.h>

#define TIMEOUT 2000
#define AR9271_FIRMWARE       0x501000U
#define AR9271_FIRMWARE_TEXT  0x903000U
#define FW_DOWNLOAD 0x30
#define FW_DOWNLOAD_DONE 0x31
#define FWMAXLEN 80000

int main(int argc, char *argv[]) {

    if(argc != 2) {
	printf("Usage: %s <firmware-file>\n", argv[0]);
	return 1;
    }

    libusb_context *libctx = NULL;
    int libinit = libusb_init(&libctx);
    assert(libinit == 0);


    /* mmap firmware file */
    int fwfd = open(argv[1], O_RDONLY);
    assert(fwfd != -1);

    struct stat fwinfo;
    assert(fstat(fwfd, &fwinfo) == 0);

    uint32_t fwlen = fwinfo.st_size;
    printf("fwlen = %u\n", fwlen);

    uint8_t *fwdata = mmap(NULL, fwlen, PROT_READ, MAP_PRIVATE, fwfd, 0);
    assert(fwdata != MAP_FAILED);


    /* Get USB handle */
    uint16_t idVendor = 0x0cf3;
    uint16_t idProduct = 0x9271;
    libusb_device_handle *usbdev = libusb_open_device_with_vid_pid(libctx, idVendor, idProduct);
    assert(usbdev != NULL);

    uint32_t memaddr = AR9271_FIRMWARE;
    uint32_t fwrem = fwlen;
    uint32_t fwdone = 0;

    /* Write out firmware in 4k chunks*/
    while(fwrem) {

	uint32_t chunksz = fwrem > 4096 ? 4096 : fwrem;
	int ret = libusb_control_transfer(usbdev, 0x40, FW_DOWNLOAD, memaddr >> 8, 0, fwdata + fwdone, chunksz, TIMEOUT);

	printf("Transferred ret=%d bytes of firmware.\n", ret);

	if(ret < 0) {
	    printf("ERROR: %s", libusb_strerror(ret));
	    return 2;
	}

	memaddr += ret;
	fwrem -= ret;
	fwdone += ret;
    }

    /* Download done message to USB device */
    int ret = libusb_control_transfer(usbdev, 0x40, FW_DOWNLOAD_DONE, AR9271_FIRMWARE_TEXT >> 8, 0, NULL, 0, TIMEOUT);
    printf("Firmware loaded: ret=%d\n", ret);

    munmap(fwdata, fwlen);
    close(fwfd);
    libusb_close(usbdev);
    libusb_exit(libctx);

    return 0;
}
