# echo-server.py

from operator import truediv
from pickle import FALSE
import socket
from xml.dom.minidom import TypeInfo
import json

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
                    txStr = handshakeMsgStr+"\n"
                    client_socket.sendall(txStr.encode('utf-8'))

            else:
                msgJson = json.loads(dataStr)
                for key in msgJson["MountMessage"].keys():
                    # print(key)
                    ### Alt Az Request
                    if key == "RequestAltAz":
                        doPrint = False
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
                        txStr = altAzMsgStr+"\n"
                    ### Park Status Request
                    if key=="IsParked":
                        doPrint = True
                        IsParkedMsgJson = {
                        "KarbonMessage" : {
                            "IsParked": mountParked
                        }
                        }
                        isParkedStr = json.dumps(IsParkedMsgJson)
                        txStr = isParkedStr+"\n"
                    ### Park Command
                    if key=="ParkCommand":
                        doPrint = True
                        mountParked = True
                        ParkAckMsgJson = {
                        "KarbonMessage" : {
                            "ParkCommand": "ParkCommand=$OK^"
                        }
                        }
                        parkCmdStr = json.dumps(ParkAckMsgJson)
                        txStr = parkCmdStr+"\n"
                    ### Unpark Command
                    if key=="UnparkCommand":
                        doPrint = False
                        mountParked = False
                        UnparkAckMsgJson = {
                        "KarbonMessage" : {
                            "UnparkCommand": "UnparkCommand=$OK^"
                        }
                        }
                        unparkCmdStr = json.dumps(UnparkAckMsgJson)
                        txStr = unparkCmdStr+"\n"
                    ### Tracking Status Request
                    if key=="getTrackRate":
                        doPrint = False
                        TrackRateJson = {
                        "KarbonMessage" : {
                            "getTrackRate": 0.0
                        }
                        }
                        trackRateJsonStr = json.dumps(TrackRateJson)
                        txStr = trackRateJsonStr+"\n"
                    client_socket.sendall(txStr.encode('utf-8'))
                    if doPrint == True:
                        print("Received: "+dataStr)
                        print("Sent: "+txStr)
            # del data
            # print("\n")