# echo-client.py

import socket

# HOST = "127.0.0.1"  # The server's hostname or IP address
# PORT = 65432  # The port used by the server
HOST = "192.168.121.177"
# HOST = "localhost"
# HOST = "192.168.190.101"
# HOST = "192.168.190.101"
PORT = 4400  # The port used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
err = s.connect((HOST, PORT))
    # print(err)
s.sendall(b"Hello, world\n")
data = s.recv(1024)

print(f"Received {data!r}")
