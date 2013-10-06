#!/usr/bin/env python

"""
Simple server for Ardomino
"""

from __future__ import print_function

from io import BytesIO
import os
import socket


class DataReader(object):
    def __init__(self):
        self._lines = []
        self._buf = BytesIO()

    def __iter__(self):
        return self

    def next(self):
        try:
            return self._lines.pop(0)
        except IndexError:
            raise StopIteration

    def feed(self, text):
        lines = text.splitlines()
        if len(lines) < 1:
            return  # nothing to do
        lines[0] = self._buf.getvalue() + lines[0]
        self._buf.seek(0)
        self._buf.truncate()
        if not text.endswith("\n"):
            self._buf.write(lines.pop())
        self._lines.extend(lines)


HOST = os.environ.get('HOST', '')
PORT = int(os.environ.get('PORT', 1234))

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

s.bind((HOST, PORT))

try:
    while True:
        print("Listening...")
        s.listen(1)
        conn, addr = s.accept()
        print("Connected: {0}:{1}".format(*addr))
        recv_data = DataReader()
        while True:
            data = conn.recv(1024)
            if not data:
                break  # EOT
            recv_data.feed(data)
            for line in recv_data:
                print("Data: " + line)
                conn.send("ACK")
        conn.close()

finally:
    s.close()
