# echo-server.py

import socket
from xml.dom.minidom import TypeInfo

# HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
# PORT = 65432  # Port to listen on (non-privileged ports are > 1023)
raVal = 0.0
deVal = 0.0
# HOST = "192.168.190.101"
HOST = "localhost"
# HOST = "192.168.121.177"
PORT = 4400  # The port used by the server

print("Attempting to connect.")

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
            # client_socket.sendall(data)
            print(data)
            loc = data.find(b'#')
            # print(type(data))
            # id = int.from_bytes(data[0:loc], "little")
            idB = data[0:loc]
            # print("ID:")
            # print(idB)
            # print(type(idB))
            # print(idB.hex())
            # print(idB.isdigit())
            
            id = int(idB)
            # print(type(tmp))
            # print(tmp)
            # id = int(.decode('ascii'))
            # print("ID: ")
            # print(id)
            # id = 99
            
            match id:
                case 99:
                    print("Sending " + data.decode('utf-8'))
                    client_socket.sendall("99#Handshake^".encode('utf-8'))
                    # client_socket.sendall(b'abcd')
                case 2:
                    print("Sending RA/DEC")
                    raVal = raVal + 0.1
                    deVal = deVal + 0.05
                    raDecStr = "2#" + str(raVal) + ";" + str(deVal) + "^"
                    # client_socket.sendall(bytes("2#1.234").encode('utf-8'))
                    client_socket.sendall(raDecStr.encode('utf-8'))
                case 3:
                    print("Sending Tracking Status")
                    client_socket.sendall("3#0.0^".encode('utf-8'))
                case _:
                    toPrint = "(Default) Sending " + data.decode("utf-8")
                    print(toPrint)
                    # client_socket.sendall(data)
            del data
            print("\n")