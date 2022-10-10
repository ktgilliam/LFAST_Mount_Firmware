# echo-server.py

# from operator import truediv
from pickle import FALSE
import socket
from xml.dom.minidom import TypeInfo
import json
import time

# HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
# PORT = 65432  # Port to listen on (non-privileged ports are > 1023)
altVal = 0.0
azVal = 0.0
# HOST = "192.168.190.101"
HOST = "localhost"
# HOST = "192.168.121.177"
PORT = 4400  # The port used by the server

print("Attempting to connect.")

mountParked = True
mountIsParking = False
parkCommandTime = time.time()


handshook = False


doPrint = False
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    # server.close()
    # server.bind((HOST, PORT))
    server.bind((HOST, PORT))
    server.listen()
    client_socket, client_address = server.accept()
    with client_socket:
        print(f"Connected by {client_address}")
        while True:
            txStr = ""
            doSend = False
            data = client_socket.recv(2048)
            if not data:
                break
            # print(data)
            dataStr = data.decode()
            # print(dataStr)
            # print(msgJson["MountMessage"])
            # print(msgJson["MountMessage"]["Handshake"])
            if handshook == False:
                doSend = True
                msgJson = json.loads(dataStr)
                handshakeStr = msgJson["MountMessage"]["Handshake"]
                handshakeVal = int(handshakeStr, 16)
                # print(hex(handshakeVal))
                if handshakeVal == 0xDEAD:
                    handshakeMsgJson = {
                        "KarbonMessage" : {
                            "Handshake": 0xbeef
                        }
                    }
                    handshook = True
                    handshakeMsgStr = json.dumps(handshakeMsgJson)
                    txStr = handshakeMsgStr+"\0"
                    client_socket.sendall(txStr.encode('utf-8'))

            else:
                now = time.time()
                msgJson = json.loads(dataStr)
                for key in msgJson["MountMessage"].keys():
                    # print(key)
                    ### Alt Az Request
                    if key == "RequestAltAz":
                        doPrint = True
                        doSend = True
                        altVal = altVal + 0.01
                        azVal = azVal + 0.02
                        altAzMsgJson = {
                        "KarbonMessage" : {
                            "AltPosition": altVal,
                            "AzPosition" : azVal
                        }
                        }
                        altAzMsgStr = json.dumps(altAzMsgJson)
                        txStr = altAzMsgStr
                    ### Park Status Request
                    elif key=="IsParked":
                        doPrint = True
                        doSend = True
                        timeSinceParkCommand = now-parkCommandTime
                        # print("###Time since park Command: "+str(timeSinceParkCommand))
                        if mountIsParking :
                            mountParked = (timeSinceParkCommand>3)
                            print("...Setting mount parked to "+str(timeSinceParkCommand>15))
                            mountIsParking = False
                            timeSinceParkCommand = 0
                        IsParkedMsgJson = {
                        "KarbonMessage" : {
                            "IsParked": mountParked
                        }
                        }
                        isParkedStr = json.dumps(IsParkedMsgJson)
                        txStr = isParkedStr
                    ### Park Command
                    elif key=="ParkCommand":
                        doPrint = True
                        doSend = True
                        mountIsParking = True
                        # mountParked = True
                        if not mountIsParking:
                            parkCommandTime = time.time()
                        ParkAckMsgJson = {
                        "KarbonMessage" : {
                            "ParkCommand": "$OK^",
                            "NoDisconnect": "$OK^"
                        }
                        }
                        parkCmdStr = json.dumps(ParkAckMsgJson)
                        txStr = parkCmdStr
                    ### Unpark Command
                    elif key=="UnparkCommand":
                        doPrint = True
                        doSend = True
                        mountParked = False
                        UnparkAckMsgJson = {
                        "KarbonMessage" : {
                            "UnparkCommand": "$OK^"
                        }
                        }
                        unparkCmdStr = json.dumps(UnparkAckMsgJson)
                        txStr = unparkCmdStr
                    ### Tracking Status Request
                    elif key=="getTrackRate":
                        doPrint = True
                        doSend = True
                        TrackRateJson = {
                        "KarbonMessage" : {
                            "TrackRate": 0.0
                        }
                        }
                        trackRateJsonStr = json.dumps(TrackRateJson)
                        txStr = trackRateJsonStr
                    else:
                        doPrint = True
                        doSend = False
                    if doSend:
                        txStr = txStr+"\0"
                        client_socket.sendall(txStr.encode('utf-8'))
                    if doPrint:
                        print("Received: "+dataStr)
                        print("Sent: "+txStr)
                        print("\n")
            # del data
            # print("\n")