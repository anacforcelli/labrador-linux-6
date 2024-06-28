#ifndef __CAM_SPI_H
#define __CAM_SPI_H

#include <linux/types.h>
#include <linux/timer.h>
#include <linux/spi/spi.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/wait.h>
#include <linux/kfifo.h>

static int devmajor;
static struct class *devclass;
static u8 UBXNAVPVT_POLL_MSG[8] = {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00};

#define UBXNAVPVT_MSG_LEN 92
#endif

