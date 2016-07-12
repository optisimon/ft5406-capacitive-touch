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


#
# NOTE: Can't figure out how to start the scan manually
#       (if that is even possible with the provided firmware)
#
## Set test mode
#i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00 0x40 b || echo "ERROR-1"
#sleep 0.1
#
## Start scan (will be set to zero when scan finishes)
#i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x02 0x08 b || echo "ERROR-2"
#while true ; do
#  VAL=$( i2cget -y ${I2C_BUS} ${I2C_ADDR} 0x02 b )
#  [ $VAL == "0" ] && break;
#  echo "VAL=$VAL"
#done



#
# Workaround is to set the normal scanning mode, and pretend we are interested in touch events...
#

# Set normal mode
i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00 0x00 b || echo "ERROR-1"
sleep 0.1

# Outputs a decimal number with current number of concurrent touches
read_num_touches()
{
  local HEX_WITH_0x_PREFIX=$(i2cget -y ${I2C_BUS} ${I2C_ADDR} 0x02 b)
  echo $((${HEX_WITH_0x_PREFIX}))
}


# Fake caring about touch events for 10 frames
for N in $(seq 1 10) ; do
  NUM_TOUCHES=$(read_num_touches)
  [ "$NUM_TOUCHES" == 1 ] && {
    i2cdump -y -r 3-6 1 0x38 bc > /dev/null
  }
  sleep 0.05
done



# Set test mode
i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00 0x40 b || echo "ERROR-1"
sleep 0.1

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
