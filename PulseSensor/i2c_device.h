/*
 * i2c_device.h
 *
 *  Created on: Nov 10, 2021
 *      Author: selik
 */

#ifndef I2C_DEVICE_H_
#define I2C_DEVICE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#define ARRAY_SIZE(array) \
    (sizeof(array) / sizeof(array[0]))
struct i2c_device{
    uint8_t address;
};

struct i2c_data{
    const void* tx_buf;
    size_t tx_len;
    void* rx_buf;
    size_t rx_len;
};

static int _check_ack(const struct i2c_device* dev){
    int err = 0;
    //IGNORE(dev);

    /* Check for Ack */
    if(UCB0IFG & UCNACKIFG){

        /*Send Stop Condition*/
        UCB0CTLW0 |= UCTXSTP;
        /*Clear Nack Flag*/
        UCB0IFG &= ~(UCNACKIFG);
        err = -1;
    }
    return err;
}

static int _transmit(const struct i2c_device* dev, const uint8_t* buf, size_t nbytes){
    int err = 0;
    //IGNORE(dev); //Don't know what this does!
    /* Send the start condition */
    UCB0CTLW0 |= UCTR+UCTXSTT;
    /* Wait Until Bus Ready */
    while((UCB0CTLW0 & UCTXSTT) && (UCB0IFG & UCTXIFG0)){};

    err = _check_ack(dev);

    /* If no errors, transmit data */
    while((err == 0) && (nbytes > 0)){
        UCB0TXBUF = *buf;
        while(!(UCB0IFG & UCTXIFG0)){
            err = _check_ack(dev);
            if(err < 0){
                break;
            }
        }
        buf++;
        nbytes--;
    }
    return err;
}

static int _receive(const struct i2c_device* dev, uint8_t* buf, size_t nbytes){
    int err = 0;
    //IGNORE(dev); //Don't know what this does!
    /* Send the start condition */
    UCB0CTLW0 &= ~(UCTR);
    UCB0CTLW0 |= UCTXSTT;

    /*Send Nack after last byte!*/
    if(nbytes == 1){
        UCB0CTLW0 |= UCTXSTP;
    }
    /*Check for ack for slave address*/
    err = _check_ack(dev);

    while((err == 0)&&(nbytes > 0)){
        /*Wait for Data to be received*/
        while(!(UCB0IFG & UCRXIFG0)){
        };

        (*buf) = (UCB0RXBUF);
        nbytes--;
        buf++;

        if(nbytes == 1){
            UCB0CTLW0 |= UCTXSTP;
        }
    }
    return err;
}



int i2c_transfer(const struct i2c_device* dev, struct i2c_data* data){

    int err = 0;
    /*Set Slave Address*/
    UCB0I2CSA = dev->address;

    /*Transfer any data*/
    if(data->tx_len > 0){
        err = _transmit(dev,(const uint8_t*)data->tx_buf,data->tx_len);
    }
    /*Receive Any Data*/
    if((err == 0) && (data->rx_len > 0)){
        err = _receive(dev,( uint8_t*)data->rx_buf,data->rx_len);
    }
    /* Send stop condition */
    UCB0CTLW0 |= UCTXSTP;

    return err;
}


#endif /* I2C_DEVICE_H_ */
