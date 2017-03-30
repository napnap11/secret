import ctypes
import time
start = time.time()
testlib = ctypes.CDLL('insertion.so')
testlib.myprint()
end = time.time()
print(end-start)
