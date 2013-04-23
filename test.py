import ctypes
from ctypes import c_ubyte, c_int

import socket
import time
r = ctypes.PyDLL("./radio.so")
r.setup_io()

# from someone else
div14464 = 0x374F

div = lambda freq: c_int(int((1<<12)*500e6/freq))

r.setup_fm(div(144.64e6))


mySocket = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
mySocket.bind ( ( '10.33.91.11', 80) )
mySocket.listen ( 1 )


while True:
	r.askLow()
	channel, details = mySocket.accept()
	channel.recv(1)
	time.sleep(0.05)
	r.sendStringAsk("hello world", 1000)
	#for letter in "hello world":
	#	# byte, mark
	#	r.sendByteAsk(c_ubyte(ord(letter)), c_int(1000))
	#	print letter, hex(ord(letter))
	channel.close()
