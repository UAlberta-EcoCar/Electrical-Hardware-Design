import struct
import serial
import threading
    
class SerialData():
    lucy_data = {
                'cap_voltage': -99,
                'cap_current': -99,
                'fc_voltage': -99,
                'fc_current': -99,
                'internal_stack_pressure': -99,
                'internal_stack_temp': -99,
                'motor_voltage': -99,
                'motor_current': -99
            }
    
    def __init__(self, connection="COM5", baud=115200):
        port = (connection)
        self.ser = serial.Serial(port, baud, timeout=5)
        
        # self.lock = threading.Lock()
        # self.condition = threading.Condition(self.lock)
        # self.readThread = threading.Thread(target=self.read_values)
        # self.readThread.start()
    
    def read_values(self):
        try:
            self.buffer = str(self.ser.readline().decode('utf-8').strip()).split(";")
            list = []
            for item in self.buffer:
                list.append(item.split()[1])
            print(list)
            if len(list) == 7:
                self.lucy_data["cap_voltage"] = list[0]
                self.lucy_data["cap_current"] = list[1]
                self.lucy_data["fc_voltage"] = list[2]
                self.lucy_data["fc_current"] = list[3]
                self.lucy_data["internal_stack_pressure"] = list[4]
                self.lucy_data["internal_stack_temp"] = list[5]
                self.lucy_data["motor_voltage"] = list[6]
                self.lucy_data["motor_current"] = list[7]

        except UnicodeDecodeError as e:
            list = []
    
        print(self.lucy_data)
            
    def get_values(self):
        return self.lucy_data
    
if __name__ == "__main__":
    testing = SerialData()
    while 1:
        testing.read_values()


# ser = serial.Serial('/dev/ttyUSB0', 9600)

# data_bytes = ser.read(sizeof(RecivedStructure))


# data_c_struct = RecivedStructure.from_buffer_copy(data_bytes)

# lucy_data = {
#     'h2_alarm': data_c_struct.h2_alarm,
#     'shell_stop': data_c_struct.shell_stop,
#     'relay_conf': data_c_struct.relay_conf,
#     'cap_voltage': data_c_struct.cap_voltage,
#     'cap_current': data_c_struct.cap_current,
#     'mtr_voltage': data_c_struct.mtr_voltage,
#     'mtr_current': data_c_struct.mtr_current,
#     'fc_voltage': data_c_struct.fc_voltage,
#     'fc_current': data_c_struct.fc_current,
#     'internal_stack_pressure': data_c_struct.internal_stack_pressure,
#     'internal_stack_temp': data_c_struct.internal_stack_temp,
#     'x_accel': data_c_struct.x_accel,
#     'y_accel': data_c_struct.y_accel,
#     'z_accel': data_c_struct.z_accel,
#     'speed_magnitude': data_c_struct.speed_magnitude,
#     'h2_voltage': data_c_struct.h2_voltage,
#     'h2_temp': data_c_struct.h2_temp,
#     'h2_pressure': data_c_struct.h2_pressure,
#     'h2_humidity': data_c_struct.h2_humidity
# }


# print(lucy_data)

# ser.close()

# class FuelCellSerial():
#     def __init__(self, connection="ttyACM0", baud=9600):
#         port = ("/dev/"+connection)
#         self.ser = serial.Serial(port, baud)
#         self.ser.bytesize = serial.EIGHTBITS 
#         self.ser.parity = serial.PARITY_NONE
#         self.ser.stopbits = serial.STOPBITS_TWO
#         self.ser.write('1'.encode('utf-8'))
#         self.fuelcell = {"voltage": 0, "current": 0, "pressure": 0, "temp": 0, "state": 'Standby'}
#         self.fuelCellStates = {'0': 'Standby', '1': 'Pressurize', '2': 'Start Up Purge',
#                                '3': 'Fuel Cell Charge', '4': 'Capacitor Charge', '5': 'Run',
#                                '6': 'Purge', '7': 'Shutdown', '8': 'Alarm', '9': 'Test'}       

#         self.lock = threading.Lock()
#         self.condition = threading.Condition(self.lock)
#         self.readThread = threading.Thread(target=self.readValues)
#         self.readThread.start()

#     def readValues(self):
#         while not(self.ser.in_waiting):
#                 self.readInBuffer()
#                 templist = self.buffer.decode('utf8').strip('\n').split('-')
#                 self.addValuesTofuelcell(templist, self.fuelCellStates)
#                 with self.condition:
#                     self.condition.notifyAll()
            
#     def readInBuffer(self):
#         self.buffer = self.ser.readline()   
#         self.ser.flush()
#         self.ser.write('1'.encode('utf-8'))

#     def addValuesTofuelcell(self, x, dictionary):
#         self.fuelcell['voltage'] = x[0]
#         self.fuelcell['current'] = x[1]
#         self.fuelcell['pressure'] = x[2]
#         self.fuelcell['temp'] = x[3]
#         self.fuelcell['state'] = dictionary[x[4]]

#     def getValue(self, Value):
#         with self.lock:
#             # Wait for new data to be available
#             self.condition.wait()
#             output = self.fuelcell[Value]
#         return output
 