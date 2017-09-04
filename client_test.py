#!/usr/bin/python
# coding: utf-8

import struct
import json
import socket
import bz2
import time

# HOST/PORT: GServer IP/PORT
HOST = '127.0.0.1'
PORT = 5000

# multiple connect interval time
interval = 3

# times : test times
# cmd   : bus command
# pkg   : send package
times = 1
cmd = 67
pkg = {
	# "player_id"            : 73,
	# "app_version"          : "3.23",
	# "unique_code_logining" : "1418635357",
	"timestamp" : 1419483951,
	"player_id" : 66,
	"signin_super_gift" : 0,
	"signin_days" : 22,
	"unique_code_logining" : 1419483936,
	"item_box_list" : {
		"object_id" : 0,
		"object_num" : 100,
		"verify_num" : 8543,
		"object_type" : 3,
	},
	"signin_today" : 0,
	"app_version" : "3.28",
}

KEY = 'b5cd81794a627789b3c03df969e97241'
_DELTA = 0x9E3779B9

def _long2str(v, w):
	n = (len(v) - 1) << 2
	if w:
		m = v[-1]
		if (m < n - 3) or (m > n): return ''
		n = m
	s = struct.pack('<%iL' % len(v), *v)
	return s[0:n] if w else s

def _str2long(s, w):
	n = len(s)
	m = (4 - (n & 3) & 3) + n
	s = s.ljust(m, "\0")
	v = list(struct.unpack('<%iL' % (m >> 2), s))
	if w: v.append(n)
	return v

def encrypt(str, key):
	if str == '': return str
	v = _str2long(str, True)
	k = _str2long(key.ljust(16, "\0"), False)
	n = len(v) - 1
	z = v[n]
	y = v[0]
	sum = 0
	q = 6 + 52 // (n + 1)
	while q > 0:
		sum = (sum + _DELTA) & 0xffffffff
		e = sum >> 2 & 3
		for p in xrange(n):
			y = v[p + 1]
			v[p] = (v[p] + ((z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (k[p & 3 ^ e] ^ z))) & 0xffffffff
			z = v[p]
		y = v[0]
		v[n] = (v[n] + ((z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (k[n & 3 ^ e] ^ z))) & 0xffffffff
		z = v[n]
		q -= 1
	return _long2str(v, False)

def decrypt(str, key):
	if str == '': return str
	v = _str2long(str, False)
	k = _str2long(key.ljust(16, "\0"), False)
	n = len(v) - 1
	z = v[n]
	y = v[0]
	q = 6 + 52 // (n + 1)
	sum = (q * _DELTA) & 0xffffffff
	while (sum != 0):
		e = sum >> 2 & 3
		for p in xrange(n, 0, -1):
			z = v[p - 1]
			v[p] = (v[p] - ((z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (k[p & 3 ^ e] ^ z))) & 0xffffffff
			y = v[p]
		z = v[n]
		v[0] = (v[0] - ((z >> 5 ^ y << 2) + (y >> 3 ^ z << 4) ^ (sum ^ y) + (k[0 & 3 ^ e] ^ z))) & 0xffffffff
		y = v[0]
		sum = (sum - _DELTA) & 0xffffffff
	return _long2str(v, True)

def json_encode(val):
	return json.dumps(val)

def json_decode(val):
	return json.loads(val)

def deal(p, t, c):

	times = t

	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((HOST,PORT))

		for i in range(times):
			player_id = p + i
			pkg['player_id'] = player_id
			encode_pkg = json_encode(pkg)
			compress_pkg = bz2.compress(encode_pkg)
			encrypt_pkg = encrypt(compress_pkg, KEY)

			pkg_len = len(encrypt_pkg);
			head = struct.pack('>II', pkg_len, cmd)

			s.send(head)
			s.send(encrypt_pkg)
			
			data_len = struct.unpack('>I', s.recv(4))
			ret_cmd = struct.unpack('>I', s.recv(4))
			data = s.recv(data_len[0])
			decrpyt_data = decrypt(data, KEY)
			decompress_data = bz2.decompress(decrpyt_data)
			print 'times', i, '->', decompress_data
			print 'times', i, '->', ret_cmd[0]
			if i != times-1:
				time.sleep(interval)
		if c == 1:
			s.close()
	except:
		s.close()

if __name__ == "__main__":
	deal(66, 4, 1)
