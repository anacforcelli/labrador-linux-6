#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

#include "spi.h"

#define CAM_M8Q_DRIVERNAME "cam_m8q"



int create_checksum(u8 *msg, int len){
    int i;
    u8 ck_a = 0, ck_b = 0;
    for(i = 2; i < len - 2; i++){
        ck_a += msg[i];
        ck_b += ck_a;
    }
    msg[len - 2] = ck_a;
    msg[len - 1] = ck_b;
    return 0;
}
/*
int cam_spi_sync(struct spi_device *spi, u8 reg, u8 *val)
{
    int ret;
    u8 lenght = sizeof(val);
    for i in range(0, lenght){
        u8 buf[2] = {reg, val[i]};
        struct spi_transfer t = {
            .tx_buf = buf,
            .rx_buf = buf,
            .len = 1,
        };
        struct spi_message m;

        spi_message_init(&m);
        spi_message_add_tail(&t, &m);
        ret = spi_sync(spi, &m);
        *val = buf[1];
    }
    return ret;
}
*/
int cam_spi_write_then_read(struct spi_device *spi, int rx_len, int tx_len, u8 *msg, u8 *rx_msg){
    int ret;
    
    u8 received_msg[rx_len];
    int i;
    for (i=0 ; i <= 4 ; i++){// FIrst, send then receive the header, class, ID and lenght
        u8 tx_buf = msg[i];
        u8 rx_buf;
        ret = spi_write_then_read(spi, &tx_buf,1, &rx_buf,1);
        received_msg[i] = rx_buf;
        if (i == 4){
            rx_len = rx_buf;

        };
    };
    if (rx_len >= tx_len ){
        for (i=5 ; i <= tx_len ; i++){
            u8 tx_buf = msg[i];
            u8 rx_buf;
            ret = spi_write_then_read(spi, &tx_buf,1, &rx_buf,1);
            
            received_msg[i] = rx_buf;
        };
        for(i=tx_len ; i <= rx_len ; i++){
            u8 tx_buf = 0xFF;
            u8 rx_buf;
            ret = spi_write_then_read(spi, &tx_buf,1, &rx_buf,1);
            if(ret < 0){
                printk("failed to write/read to spi device\n");
                return ret;
            }
            received_msg[i] = rx_buf;
        };
    }
    else{
        for (i=5; i <= rx_len; i++){
            u8 tx_buf = msg[i];
            u8 rx_buf;
            ret = spi_write_then_read(spi, &tx_buf , 1 , &rx_buf, 1);
            if(ret < 0){
                printk("failed to write/read to spi device\n");
                return ret;
            }
            received_msg[i] = rx_buf;
        };
        for(i=rx_len; i <= tx_len; i++){
            u8 tx_buf = msg[i];
            u8 rx_buf;
            ret = spi_write_then_read(spi, &tx_buf,1, &rx_buf,1);
            if(ret < 0){
                printk("failed to write/read to spi device\n");
                return ret;
            }
        };
        rx_msg = received_msg;
    }
    return 0;
}
static int cam_probe(struct spi_device *spi){
    int ret;
    u8* probe_message = UBXNAVPVT_POLL_MSG;
    u8* received_msg;

    create_checksum(probe_message, sizeof(probe_message));

    ret = cam_spi_write_then_read(spi, UBXNAVPVT_MSG_LEN , sizeof(probe_message), probe_message, received_msg);
    if(ret < 0){
        printk("failed to write to spi device\n");
        return ret;
    }
    printk("spi device sync success\n");
    printk("%s", (char*) received_msg);
    return 0;

}

static const struct of_device_id cam_m8q_of_match[] = {
	{.compatible = "caninos,cam-m8q", },
    {},
};
MODULE_DEVICE_TABLE(of, cam_m8q_of_match);

static struct spi_driver cam_driver = {
    .probe = cam_probe,
    //.remove = cam_remove,
    //.id_table = cam_spi_id,
    .driver = {
        .name = CAM_M8Q_DRIVERNAME,
        .of_match_table = of_match_ptr(cam_m8q_of_match),
        .owner = THIS_MODULE,
    },
};

static int __init cam_init(void){
    int ret;
    /*
    ret = register_chrdev(0, CAM_M8Q_DRIVERNAME, &fops);
    if(ret < 0){
        printk(KERN_ERR "Failed to register char device\n");
        return ret;
    }

    devmajor = ret;
    devclass = class_create(THIS_MODULE, CAM_M8Q_DRIVERNAME);
    if(!devclass){
        printk("failed to register class\n");
		ret = -ENOMEM;
		goto out1;
    }
    */
    ret = spi_register_driver(&cam_driver);

    if(ret){
		printk("failed to register spi driver\n");
		goto out2;
	}

    goto out;

	out2:
	out1:
		class_destroy(devclass);
		devclass = NULL;
	out:

	printk("registered spi driver");
	return 0;
}
module_init(cam_init);

static void __exit cam_exit(void){
    spi_unregister_driver(&cam_driver);
    /*
    class_destroy(devclass);
    unregister_chrdev(devmajor, CAM_M8Q_DRIVERNAME);
    devclass = NULL;
    */
}

module_exit(cam_exit);

MODULE_ALIAS("platform:"CAM_M8Q_DRIVERNAME);
MODULE_ALIAS("spi:"CAM_M8Q_DRIVERNAME);
MODULE_AUTHOR("Fernando Asevedo Gentil <fegentil02@gmail.com>");
MODULE_DESCRIPTION("ubex cam m8q  SPI driver");
MODULE_LICENSE("GPL");
