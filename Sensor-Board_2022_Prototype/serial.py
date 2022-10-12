with serial.Serial() as ser:
    ser.baudrate = 125000
    ser.port = 'COM4'
    ser.open()
    ser.write('SUB EVT +ALL\n'.encode())
    for x in range(0,10000):
        string = 'GET COM_LOOP/DS\n'
        ser.write(string.encode())
        ser.flush()
        hello = ser.readline()
        print(hello)