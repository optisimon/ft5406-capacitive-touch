#!/bin/bash
#
# Script dumping as many registers as we can from the FT5406 touch controller
# used by the official 7 inch Raspberry Pi touchscreen display.
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


OPERATING_MODE_OPERATING=0x00
OPERATING_MODE_TEST=0x40
OPERATING_MODE_SYSINFO=0x10


# Usage: set_operating_mode hex_for_mode
set_operating_mode()
{
  i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00 $1 && sleep 0.1
}


# Set device mode to Operating
set_operating_mode $OPERATING_MODE_OPERATING || {
  echo "ERROR-1"
  exit 1;
}

# Dump all registers:
echo -e "\n\nRegister dump (Operating):"
i2cdump -r 0-0xff -y ${I2C_BUS} ${I2C_ADDR} b || {
  echo "ERROR-2"
  exit 1;
}

# Set device mode to Test
set_operating_mode $OPERATING_MODE_TEST || {
  echo "ERROR-3"
  exit 1;
}

# Dump all registers:
echo -e "\n\nRegister dump (Test):"
i2cdump -r 0-0xff -y ${I2C_BUS} ${I2C_ADDR} b || {
  echo "ERROR-4"
  exit 1;
}


#
# Warning: Whenever I've been trying to read out the SYSINFO fields,
#          the touch controller have hanged, and the only values I
#          read back is the number of the register I'm trying to read.
#            That error condition have persisted until rebooting (I
#          currently don't have GPIO lines for resetting the controller)
#            It's actually not even possible to enter that mode and
#          exit it without locking up the controller.
#

## Set device mode to System Information
#set_operating_mode $OPERATING_MODE_SYSINFO || {
#  echo "ERROR-5"
#  exit 1;
#}
#
## Dump all registers:
#echo -e "\n\nRegister dumps (System Information):"
#i2cdump -r 0-0 -y ${I2C_BUS} ${I2C_ADDR} b && echo && \
#i2cdump -r 2-2 -y ${I2C_BUS} ${I2C_ADDR} b && echo && \
#i2cdump -r 0x07-0x16 -y ${I2C_BUS} ${I2C_ADDR} b && echo && \
#i2cdump -r 0x1b-0x1f -y ${I2C_BUS} ${I2C_ADDR} b || {
#  echo "ERROR-6"
#  exit 1;
#}
