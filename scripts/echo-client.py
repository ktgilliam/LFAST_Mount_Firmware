# echo-client.py

import socket

# HOST = "127.0.0.1"  # The server's hostname or IP address
# PORT = 65432  # The port used by the server
# HOST = "192.168.1.177"
HOST = "192.168.190.101"
PORT = 4400  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b"Hello, world")
    data = s.recv(1024)

print(f"Received {data!r}")
