# echo-server.py

import socket

# HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
# PORT = 65432  # Port to listen on (non-privileged ports are > 1023)

HOST = "192.168.190.101"
PORT = 4400  # The port used by the server

print("Attempting to connect.")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.bind((HOST, PORT))
    server.listen()
    client_socket, client_address = server.accept()
    with client_socket:
        print(f"Connected by {client_address}")
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            client_socket.sendall(data)
