# -----------------------------------------------------------------------------------
# ReadSerialAndPushToInfluxDB.py
# T. Barrett, Uni of Sussex, 2018
#
# A script which listens for a string of data arriving over the serial port. If the
# string received matches one of those in the list "Data_format", then construct a
# "payload" string and post it to an InfluxDB server via its HTTP API.
# -----------------------------------------------------------------------------------
DEBUG = True

def printDebug(string):
    if DEBUG:
        print string

import serial    # Used to connect and interact with USB serial object
import requests  # Used for making HTTP posts into remote InfluxDB database

IP = "***.***.***.***"               # The location of the machine hosting the influxdb instance
DB = "QSD_Sensor_Network_Database"   # The InfluxDB database name to write to (make sure exists)

# List of Acceptable Strings to be Received on Serial Port ####################################################
expected_data_format_list = ["GRLab,Dev01,A0,A1,A2,A3,A4,A5,T1,T2,T3,T4",
                             "GRLab,Dev02,Pressure",
                             "GRLab,Dev03,A0,A1,A2,D2,D3",
                             "LaserLab,Dev01,A0,A1,D2,D3,T1,T2,T3,T4",
                             "BLLab,Dev01,A0,A1,A2,A3,A4,A5,D2,D3,T1,T2,T3,T4,T5,Pressure",
                             "SYSTEM,Collector,healthVector1,healthVector2,healthVector3,healthVector4,healthVector5"]


# Dictionary to map between received values and measurements to be posted to InfluxDB #########################
# Format is {"Tag Name for Influx":["Value to take from received string","Scaling factor","Decimal places"]}

InfluxDB_Dictionary = {}
InfluxDB_Dictionary["GRLab,Dev01"] = {"A0":["A0",1,0],                 
                                      "A1":["A1",1,0],                 
                                      "A2":["A2",1,0],                 
                                      "A3":["A3",1,0],
                                      "A4":["A4",1,0],
                                      "A5":["A5",1,0],
                                      "T1":["T1",1,2],                 
                                      "T2":["T2",1,2],
                                      "T3":["T3",1,2],
                                      "T4":["T4",1,2]
                                     }

InfluxDB_Dictionary["GRLab,Dev02"] = {"Pressure":["Pressure",1,15]
                                      }

InfluxDB_Dictionary["GRLab,Dev03"] = {"A0":["A0",1,0],                 
                                      "A1":["A1",1,0],
                                      "A2":["A2",1,0],
                                      "Power_Seed":["A0",28.1/325,3],
                                      "Power_BoosTA":["A2",2.74/608,3],
                                      "Power_Repump":["A1",35.5/647,3],
                                      "ShutterState_BoosTA":["D2",1,0],                 
                                      "ShutterState_Repump":["D3",1,0]
                                      }

InfluxDB_Dictionary["LaserLab,Dev01"] = {"A0":["A0",1,0],                 
                                         "A1":["A1",1,0],                 
                                         "ShutterState_Cooler":["D2",1,0],                 
                                         "ShutterState_Repump":["D3",1,0], 
                                         "Power_Cooler":["A0",2.792/766,3],
                                         "Power_Repump":["A1",2.481/706,3],
                                         "T1":["T1",1,2],                 
                                         "T2":["T2",1,2],
                                         "T3":["T3",1,2],
                                         "T4":["T4",1,2]
                                         }


InfluxDB_Dictionary["BLLab,Dev01"] = {"A0":["A0",1,0],                 
                                      "A1":["A1",1,0],
                                      "A2":["A2",1,0],                 
                                      "A3":["A3",1,0], 
                                      "A4":["A4",1,0],                 
                                      "A5":["A5",1,0], 
                                      "ShutterState_BoosTA":["D2",1,0],                 
                                      "ShutterState_Repump":["D3",1,0], 
                                      "Power_3DRepump":["A0",3/750,3],
                                      "Power_2DRepump":["A1",3/750,3],
                                      "Power_BoosTAOutput":["A2",3/750,3],
                                      "Power_BoosTASeed":["A3",3/750,3],
                                      "Power_AOM1stArm":["A4",3/750,3],
                                      "Power_AOM2ndArm":["A5",3/750,3],
                                      "T1_LaserBox":["T1",1,2],                 
                                      "T2_Hood":["T2",1,2],
                                      "T3_Canopy":["T3",1,2],
                                      "T4_VentLabIn":["T4",1,2],
                                      "T5_VentLabOut":["T5",1,2],
                                      "Pressure":["Pressure",1,15]
                                         }

InfluxDB_Dictionary["SYSTEM,Collector"] = {"STATUS_LaserLab_Dev01":["healthVector1",1,0],                 
                                           "STATUS_GRLab_Dev02":["healthVector2",1,0],
                                           "STATUS_GRLab_Dev03":["healthVector3",1,0],
                                           "STATUS_GRLab_Dev01":["healthVector4",1,0],
                                           "STATUS_BLLab_Dev01":["healthVector5",1,0]
                                          }
###############################################################################################################


# Try to connect to serial port device ########################################################################
ser = serial.Serial()
ser.baudrate = 57600
ser.port = "/dev/ttyUSB0"
try:
    ser.open()
    print("Serial port opened sucessfully")
    print('\n')
    print("Data Collection Started...")
    proceed = 1
except:
    print("Could not open serial port")
    proceed = 0
###############################################################################################################

# Construct a table of expected values to receive #############################################################
expected_data_format_table = []
ID_list = []
for n in range(0, len(expected_data_format_list)):
    expected_data_format_table.append(expected_data_format_list[n].split(","))
    ID_list.append(expected_data_format_table[n][0] + "," + expected_data_format_table[n][1])
###############################################################################################################


# Main listener loop - runs indefinitely ######################################################################
errorCount = 0
while proceed:

    bytesAvailable = ser.in_waiting

    if bytesAvailable > 0:
        
        rawPacket = ser.readline()

        try:
            received_string = rawPacket.decode().strip()
            received_string_list = received_string.split(",")               # Separate received string into list at commas 
            received_string_list = list(filter(None, received_string_list)) # Remove blank entries from received string
        except:
            received_string = "a"
            errorCount = errorCount+1;

        # Check if string received is a valid one ###################################
        if len(received_string_list) >= 2:
            received_string_ID = received_string_list[0] + "," + received_string_list[1]

            if received_string_ID in ID_list:
                ID_index = ID_list.index(received_string_ID)
                
                if len(received_string_list) == len(expected_data_format_table[ID_index]):
                    printDebug("Received data packet with matched ID and correct length!")
    
                    # Construct and Print Payload String ##########
                    payload_string = "measurements"                                         + "," + \
                                     "Room_ID="               + received_string_list[0]     + "," + \
                                     "Device_ID="             + received_string_list[1]     + " "
                    key_list = list(InfluxDB_Dictionary[received_string_ID].keys())
                    
                    for m in range(0,len(key_list)):
                        ind = expected_data_format_table[ID_index].index(InfluxDB_Dictionary.get(received_string_ID).get(key_list[m])[0])
                        try:
                            value = float((InfluxDB_Dictionary.get(received_string_ID).get(key_list[m])[1]))*float(received_string_list[ind])
                        except:
                            value = 0;
                            errorCount = errorCount+1;                            
                            
                        payload_string += key_list[m] + "=" + '{0:.{prec}f}'.format(value, prec=(InfluxDB_Dictionary.get(received_string_ID).get(key_list[m])[2])) + "," 

                    payload_string = payload_string[:-1]
                    
                    printDebug("Payload String: " + payload_string)
                    printDebug("Error Count: ")
                    printDebug(errorCount)
                    ###############################################
                
                        
                    # Post to InfluxDB via HTTP API ###############
                    try:
                        r = requests.post("http://%s:8086/write?db=%s" % (IP, DB), data=payload_string, timeout=0.1)
                        printDebug("Post to InfluxDB Attempted...")
                        
                        if r.status_code == 204:
                            printDebug("Status Code: 204 (Data Point Sucessfully Inserted!)")
                            printDebug('\n')
                        else:
                            printDebug("{}{}".format("Error code from server: ", r.status_code))
                            printDebug("Response from server: ")
                            printDebug(r.json())
                            printDebug('\n')
                            
                    except requests.exceptions.Timeout:
                        printDebug("ERROR POSTING! (The request timed out).")
                        printDebug('\n')
                        
                    except requests.exceptions.ConnectionError:
                        printDebug("ERROR POSTING! (A connection error occurred).")
                        printDebug('\n')
                        
                    except requests.exceptions.RequestException as e:
                        printDebug("ERROR POSTING! (Unknown Error):")
                        printDebug('\n')
                        printDebug(e)
                    ###############################################

                           

                else:
                    printDebug("Correct ID, but incorrect length!")
                    printDebug(received_string)
                    printDebug('\n')
            else:
                printDebug("ID not matched")
                printDebug(received_string)
                printDebug('\n')
        else:
            printDebug("Received string wrong size!")
            printDebug(received_string)
            printDebug('\n')
    



