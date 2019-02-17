#!/usr/bin/env python3

import argparse
import serial
import time

DEFAULT_PORT = '/dev/ttyUSB0'
DEFAULT_BAUD = 9600

def communicate(cmd, port, baudrate):
    with serial.Serial(port, baudrate) as ser:
        ser.write(cmd + b'\n')
        ser.readline()  # Command we sent
        print(ser.readline().decode('utf-8').strip())

def main():
    parser = argparse.ArgumentParser(description='Control SimpleClock via UART')
    parser.add_argument('-p', '--port', nargs=1, default=DEFAULT_PORT)
    parser.add_argument('-b', '--baud', nargs=1, default=DEFAULT_BAUD)
    parser.add_argument('action', choices=('set-time', 'get-time', 'get-temp'))
    args = parser.parse_args()

    cmd = b''
    if args.action == 'set-time':
        curtime = time.strftime('%H:%M:%S')
        cmd = b'set ' + curtime.encode('utf-8')
    elif args.action == 'get-time':
        cmd = b'get'
    elif args.action == 'get-temp':
        cmd = b'temp'

    communicate(cmd, args.port, args.baud)


if __name__ == '__main__':
    main()
