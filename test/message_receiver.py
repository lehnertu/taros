#!/usr/bin/env python3

"""
This Python script uses a connected LoRa modem to receive messages
sent from a TAROS system. All system messages are printed to the console.

The serial device at which the modem is connected must be given
on the command line. (default is '/dev/ttyUSB0')

The receiver runs until interrupted with Ctrl-C
"""

import sys
import argparse
import serial

parser = argparse.ArgumentParser()
# Adding a positional argument with a default value
parser.add_argument('device', nargs='?',
    help='the name of the modem serial device, e.g. /dev/ttyUSB0', default='/dev/ttyUSB0')
args = parser.parse_args()

# open serial port
try:
    ser = serial.Serial(args.device, timeout=0.5)
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    sys.exit()
print('listening on '+ser.name)

# configuration mode requires M0 and M1 to be high
# for communication mode both are pulled low, so this is not available
# address = 00 00
# mode = E4 : 115200,8N1 air9.6k
# reg1 = 00
# freq = 12 : ch18=868.125 MHz
# enable RSSI
# ser.write(b'\xC0\x00\x06\x00\x00\xE4\x00\x12\x80')     # command
# read the response
# response = ser.read(size=10)
# print('configuration: '+response.hex())

try:
    while True:
        response = ser.read(size=200)
        if len(response) > 0:
            line = ""
            for c in response:
                line += (" %0.2X" % c)
            print(line)
            print(response)
            print()

except KeyboardInterrupt:
    print("\nTerminating the loop on CTRL-C.")
    ser.close()