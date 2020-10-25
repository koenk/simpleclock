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

def datetime(s):
    try:
        time.strptime(s, "%d-%m-%Y")
        s += " 00:00:00"
    except ValueError as e:
        pass
    try:
        dt = time.strptime(s, "%d-%m-%Y %H:%M:%S")
    except ValueError as e:
        raise argparse.ArgumentTypeError(str(e))
    if dt.tm_year < 1900 or dt.tm_year > 2099:
        raise argparse.ArgumentTypeError('Year must be between 1900 and 2100.')
    return s

def main():
    parser = argparse.ArgumentParser(description='Control SimpleClock via UART')
    parser.add_argument('-p', '--port', nargs=1, default=DEFAULT_PORT)
    parser.add_argument('-b', '--baud', nargs=1, default=DEFAULT_BAUD)
    subparsers = parser.add_subparsers(help='Command', dest='command')
    subparsers.required = True
    subparsers.add_parser('set-time')
    subparsers.add_parser('get-time')
    subparsers.add_parser('set-date')
    subparsers.add_parser('get-date')
    subparsers.add_parser('enable-datediff')
    subparsers.add_parser('disable-datediff')
    subparsers.add_parser('set-datediff').add_argument('target', type=datetime)
    subparsers.add_parser('get-datediff')
    subparsers.add_parser('set-brightness').add_argument('brightness', type=int)
    subparsers.add_parser('get-brightness')
    subparsers.add_parser('get-temp')
    subparsers.add_parser('get-version')

    args = parser.parse_args()

    cmds = {
        'set-time': 'ts ' + time.strftime('%H:%M:%S'),
        'get-time': 'tg',
        'set-date': 'ds ' + time.strftime('%d:%m:%Y'),
        'get-date': 'dg',
        'enable-datediff': 'dde 1',
        'disable-datediff': 'dde 0',
        'set-datediff': 'dds ' + getattr(args, 'target', ''),
        'get-datediff': 'ddg',
        'set-brightness': 'bs %d' % getattr(args, 'brightness', 0),
        'get-brightness': 'bg',
        'get-temp': 'temp',
        'get-version': 'ver'
    }

    cmd = cmds[args.command].encode('utf-8')

    communicate(cmd, args.port, args.baud)


if __name__ == '__main__':
    main()
