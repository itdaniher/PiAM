import ctypes
from ctypes import c_ubyte, c_int

import socket
import time
import crc

r = ctypes.PyDLL("./radio.so")
r.setup_io()

div = lambda freq: c_int(int((1<<12)*500e6/freq))

r.setup_fm(div(144.64e6))

triggerSocket = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
triggerSocket.bind ( ( '10.33.91.11', 9000) )
triggerSocket.listen ( 1 )

def hexToByte( hexStr ):
    bytes = []
    hexStr = ''.join( hexStr.split(" ") )
    for i in range(0, len(hexStr), 2):
        bytes.append( chr( int (hexStr[i:i+2], 16 ) ) )
    return ''.join( bytes )

def sendMessage(message = "hello there, world!", sym = chr(104)):
	crcMsg = crc.crc32(message)|1<<33
	crcStr = hexToByte(hex(crcMsg)[3::][0:-1])
	r.sendStringAsk(message+crcStr, ord(sym))
	print [hex(ord(x))[2::] for x in message+crcStr]

while True:
	r.askLow()
	channel, details = triggerSocket.accept()
	sym = channel.recv(1)
	print ord(sym)
	time.sleep(0.05)
	sendMessage("hello world"*10, sym)
	channel.close()
