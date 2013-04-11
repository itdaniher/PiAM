import ctypes
import timeit
r = ctypes.PyDLL("./radio.so")
r.setup_io()
r.setup_fm(ctypes.c_float(144.64))
r.sendByteAsk(ctypes.c_ubyte(0x55))
#print timeit.timeit(setup = 'import ctypes;r = ctypes.PyDLL("./radio.so")', stmt='r.usleep2(ctypes.c_uint(100))', number=1)
