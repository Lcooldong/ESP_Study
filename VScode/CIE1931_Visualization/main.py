import sys
import time
import serial   # pip3 install pyserial
import threading
import queue
import numpy as np
import colour
from colour.plotting import *


PORT = 'COM10'
TIMEOUT = 100


def connect_uart(_ser, _port, _baudrate=115200, _timeout=10):
    _ser = serial.Serial(
        port=_port,
        baudrate=_baudrate,
        parity='N',
        stopbits=1,
        bytesize=8,
        timeout=_timeout
    )

    try:
        if _ser.isOpen():
            print(_ser.name)
            return _ser
    except serial.SerialException as e:
        print('Serial Not Connected : ' + e)
        return


if __name__ == '__main__':
    ser = serial.Serial()
    ser = connect_uart(ser, PORT, _timeout=TIMEOUT)
    RGB = colour.models.eotf_inverse_sRGB(np.array([[20, 2, 45], [87, 12, 67]]) / 255)

    plot_RGB_chromaticities_in_chromaticity_diagram_CIE1931(RGB)
    #plot_RGB_chromaticities_in_chromaticity_diagram_CIE1931(RGB, )
    while True:
        command = 'i'

        ser.write(command.encode())
        time.sleep(0.01)

        if ser.readable():
            response = ser.readline()
            print(response.decode())
