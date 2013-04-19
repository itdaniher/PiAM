import ctypes
from ctypes import c_ubyte, c_int
import time
r = ctypes.PyDLL("./radio.so")
r.setup_io()

# from someone else
div14464 = 0x374F

div = lambda freq: c_int(int((1<<12)*500e6/freq))

r.setup_fm(div(144.64e6))

while True:
	for letter in "hello world":
		# byte, mark
		r.sendByteAsk(c_ubyte(ord(letter)), c_int(1000))
		# byte, mark, base divider, spread
		#r.sendByteFsk(c_ubyte(ord(letter)), c_int(100000), div(144.5), c_int(10000))
		print letter, bin(ord(letter))
	time.sleep(1)
