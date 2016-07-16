# raspberry_pi_ft5406_capacitive_touch

Raspberry Pi 7 inch touchscreen controller display hacking (with Raspberry Pi 3 as host)


## Description

Scripts for communicating with the touch screen controller on the
official 7 inch Raspberry Pi touchscreen display.

If you just want to use the touch screen as a regular touch screen (controlled
by your Raspberry Pi), this repository is not for you.

If you want to talk to the touch controller from userspace
(or from another platform), this repository might be for you.

 - One application reading and displaying capacitance images live with a decent refresh rate

 - One script allow you to capture single capacitance images using the touch screen

 - Another script allow you to use the screen as a touch screen completely from
userspace.

More details can be found on www.optisimon.com/raspberrypi/touch/ft5406/2016/07/13/raspberry-pi-7-inch-touchscreen-hacking/


## Disclaimer
This has only been tested using my Raspberry Pi 3, and my touch screen.

It's more than likely that you would need to connect to the touch screen
differently for different versions of the Raspberry Pi.
And since I have a "Don't move FFC:s around more than absolutely needed" policy
(typically rated for just a few insertion cycles), I won't test the screen with
any older (or newer) Raspberry Pi.

Modifying the hardware to run this code may cause problems I havn't forseen, so
use it at your own risk (as you always should with things found on the internet).

And remember that the Raspberry Pi team have the freedom to change both pieces
of hardware as they see fit, so it may not work with future hardware.


## Copyright

Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)

Do whatever you like with this code, but please refer to me as the original author.
