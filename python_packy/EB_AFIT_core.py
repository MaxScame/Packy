from ctypes import *
import os


# Init dll library (for Win32 only)
adder = CDLL(os.path.abspath(os.path.join(os.path.dirname(__file__), './3d_packer.dll')))
start = adder.run
start.argtypes = [c_char_p]


"""
Calculate data from file
(default filename: "boxlist.txt")
"""
def calc(filename='boxlist.txt'):
    data_filename = c_char_p(filename.encode())
    res = start(data_filename)
