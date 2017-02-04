#!/bin/bash
#
# Script communicating directly with the Raspberry Pi 7 inch touchscreen display, and
# forwarding touch events to the X server
#
# See optisimon.com/raspberrypi/touch/ft5406/2016/07/13/raspberry-pi-7-inch-touchscreen-hacking/
# for more details.
#
# WARNING: THIS IS NOT FOR REGULAR USERS. It's for low level hackers who want to talk
#          directly with the hardware over I2C, using unsupported commands which may
#          change between display batches. I also have no clue about proper timing or
#          how to ensure that a touch events coordinates (16 bits) are read out in a
#          latched manner, and I have no control over hardware reset of the display,
#          so I have no way to recover from certain error conditions.
#            Additionally, it might be possible to brick the display controller in
#          mysterious ways by changing default values (if they are stored
#          permanently), so use this at your own risk.
#
# HW configuration:
#          The display is attached to a Raspberry Pi 3 using the standard display cable.
#          Since there currently is no way to talk to it directly on a raspberry pi 3
#          (except if you are the GPU), a few changes was needed:
#          * The I2C lines from the display is additionally attached to the i2c-1 bus.
#          * The I2C-1 bus is enabled in /boot/config.txt by adding this line:
#            dtparam=i2c_arm=on
#          * The touch panel is disabled in /boot/config.txt (only way to get the GPU to
#            stop talking constantly to it without cutting traces on the board). This
#            line was added to /boot/config.txt:
#            disable_touchscreen=1
#
# Misc notes:
#          During system bootup, there is still quite a lot of traffic on that I2C bus,
#          but once the system is up and running, the bus seems to be quiet.
#
#          Helpful resources:
#          * Forum discussing "Capacitive touch problems under linux (FT5x06)":
#              http://www.newhavendisplay.com/NHD_forum/index.php/topic,82.0.html
#          * Link to touch controller registers in that forum post (WARNING they are not
#            identical to the registers in the raspberry pi (for instance going into the
#            system information device mode locks up the touch controller until the
#            raspberry pi is rebooted
#              http://www.newhavendisplay.com/app_notes/FT5x06_registers.pdf
#          * Link to some kernel driver for an FT5x06 chip based touch screen (WARNING
#            they are doing several things very from the raspberry pi. (They seem
#            to have a firmware in their touch screen which requires parity bytes to
#            be used, and certain registers don't match the registers in the appnote
#            by New Haven Display)
#              http://lxr.free-electrons.com/source/drivers/input/touchscreen/edt-ft5x06.c?v=4.4
#
# This script requires xdotool and i2c-tools (sudo apt-get install xdotool i2c-tools)
#
#
# Original author: Simon Gustafsson (www.optisimon.com)
#
# Copyright (c) 2016 Simon Gustafsson (www.optisimon.com).
#
# Do whatever you like with this code, but please refer to me as the original author.
#


I2C_BUS=1
I2C_ADDR=0x38


#
# Verify some dependencies
#
which xdotool > /dev/null || {
  echo "ERROR: You need to have xdotool installed (to forward clicks to X)"
  echo "       Install it by running"
  echo "       sudo apt-get install xdotool"
  exit 2;
}

which i2cget > /dev/null && \
which i2cdump > /dev/null && \
which i2cset > /dev/null || {
  echo "ERROR: You need to have i2c-tools installed"
  echo "       Install it by running"
  echo "       sudo apt-get install i2c-tools"
  exit 2;
}


OPERATING_MODE_OPERATING=0x00

# Usage: set_operating_mode hex_for_mode
set_operating_mode()
{
  i2cset -y ${I2C_BUS} ${I2C_ADDR} 0x00 $1 && sleep 0.1
}


# Outputs a decimal number with current number of concurrent touches
read_num_touches()
{
  local HEX_WITH_0x_PREFIX=$(i2cget -y ${I2C_BUS} ${I2C_ADDR} 0x02 b)
  echo $((${HEX_WITH_0x_PREFIX}))
}



#
# Start of the real script
#


# Set device mode to "Operating"
set_operating_mode $OPERATING_MODE_OPERATING || {
  echo "ERROR-1"
  exit 1;
}


# The main touch reading and forwarding loop
RELEASED=1
while true ; do
  NUM_TOUCHES=$(read_num_touches) || {
    echo "ERROR-2"
    exit 1;
  }

  if [ "${NUM_TOUCHES}" == "0" ] ; then
    [ $RELEASED == 0 ] && {
      xdotool mouseup 1
    }
    RELEASED=1
  fi

  if [ "${NUM_TOUCHES}" == "1" ] ; then
    STR=$(i2cdump -y -r 3-6 ${I2C_BUS} ${I2C_ADDR} bc \
      | tail -n 1 \
      | sed -e 's/00: *//') || {
        echo "ERROR-3"
        exit 1;
      }
    #echo "STR=$STR"
    IFS=' ' read -r -a BYTES <<< "$STR" || {
        echo "ERROR-4"
        exit 1;
      }

    # For some reason, I sometimes gets an invalid X coordinate larger than 256*128.
    # so let's disregard those
    [ $(( 0x${BYTES[0]} )) -ge $((0x10)) ] && {
      #echo "Invalid X data - zeroing high nibble and running anyway..."
      BYTES[0]=$(echo ${BYTES[0]} | sed -e 's/^./0/')
      #echo "Repaired first byte: ${BYTES[0]}"
    }

    X=$(( 256 * 0x${BYTES[0]} + 0x${BYTES[1]} ))
    Y=$(( 256 * 0x${BYTES[2]} + 0x${BYTES[3]} ))
    if [ $RELEASED == "1" ] ; then
      echo "GOING TO FORWARD TOUCH AT (${X}, ${Y})"
      xdotool mousemove --sync ${X} ${Y} mousedown 1
    else
      echo "GOING TO FORWARD DRAG TO (${X}, ${Y})"
      xdotool mousemove --sync ${X} ${Y}
    fi
    sleep 0.05
    RELEASED=0
  fi
done
