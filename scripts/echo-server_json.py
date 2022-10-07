# echo-server.py

from operator import truediv
import socket
from xml.dom.minidom import TypeInfo
import json

# HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
# PORT = 65432  # Port to listen on (non-privileged ports are > 1023)
raVal = 0.0
deVal = 0.0
# HOST = "192.168.190.101"
HOST = "localhost"
# HOST = "192.168.121.177"
PORT = 4400  # The port used by the server

print("Attempting to connect.")

mountParked = True

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    # server.close()
    # server.bind((HOST, PORT))
    server.bind((HOST, PORT))
    server.listen()
    client_socket, client_address = server.accept()
    with client_socket:
        print(f"Connected by {client_address}")
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            dataStr = data.decode()
            msgJson = json.loads(dataStr)
            
            print(msgJson["MountMessage"])
            
            handshakeMsgJson = {
                "ControllerMessage" : {
                    "handshake": 0xbeef
                }
            }
            handshakeMsgStr = json.dumps(handshakeMsgJson)
            print(handshakeMsgStr)
            client_socket.sendall(handshakeMsgStr.encode('utf-8'))
            
            del data
            print("\n")