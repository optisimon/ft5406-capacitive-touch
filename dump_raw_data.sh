#!/bin/bash
#
# This script dumps raw capacitance values from the touch controller to stdout.
#
# See optisimon.com/raspberrypi/touch/ft5406/2016/07/13/raspberry-pi-7-inch-touchscreen-hacking/
# for more details.
#
#
# Original author Simon Gustafsson (www.optisimon.com)
#
# Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
#
# Do whatever you like with this code, but please refer to me as the original author.
#

I2C_BUS=1
I2C_ADDR=0x38


# Set test mode
i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00 0x40 b || echo "ERROR-1"
sleep 0.1

# Start scan (by setting high bit while in test mode)
i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00 0xc0 b || echo "ERROR-2"

# Wait for scan to complete (wait for high bit to be cleared)
while true ; do
  VAL=$( i2cget -y ${I2C_BUS} ${I2C_ADDR} 0x00 b )
  [ $VAL == "0x40" ] && break;
  #echo "VAL=$VAL"
done

# Read out raw data
for ROW_ADDR in $(seq 0 23) ; do
 i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x01 $(printf 0x%02x ${ROW_ADDR}) b || echo "ERROR-3"
 # Wait at least 100us
 sleep 0.001 ;
 for COL in $(seq 0 11) ; do
  VAL=$(i2cget -y ${I2C_BUS} ${I2C_ADDR} $(printf 0x%02x $(( 16 + ( 2 * ${COL} ) )) ) w)
  VAL2=0x${VAL:4:2}${VAL:2:2}
  echo -n "${VAL2} "
 done
 echo
done
