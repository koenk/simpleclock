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
    subparsers = parser.add_subparsers(help='Command', dest='command')
    subparsers.required = True
    subparsers.add_parser('set-time')
    subparsers.add_parser('get-time')
    subparsers.add_parser('get-temp')
    subparsers.add_parser('get-version')
    subparsers.add_parser('set-brightness').add_argument('brightness', type=int)
    subparsers.add_parser('get-brightness')

    args = parser.parse_args()

    cmd = b''
    if args.command == 'set-time':
        curtime = time.strftime('%H:%M:%S')
        cmd = b'set ' + curtime.encode('utf-8')
    elif args.command == 'get-time':
        cmd = b'get'
    elif args.command == 'get-temp':
        cmd = b'temp'
    elif args.command == 'get-version':
        cmd = b'version'
    elif args.command == 'set-brightness':
        cmd = b'brightness %d' % args.brightness
    elif args.command == 'get-brightness':
        cmd = b'getbrightness'

    communicate(cmd, args.port, args.baud)


if __name__ == '__main__':
    main()
