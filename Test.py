import socket

# 创建UDP服务器套接字
server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# 绑定服务器的IP地址和端口
server_ip = "0.0.0.0"  # 监听所有IP地址
server_port = 12345     # 服务器监听的端口
server_socket.bind((server_ip, server_port))

print(f"服务器已启动，正在监听 {server_ip}:{server_port}")

while True:
    # 接收来自机器人的数据包（最大1024字节）
    data, addr = server_socket.recvfrom(1024)
    print(f"接收到来自 {addr} 的消息：{data.decode()}")

    # 向机器人发送控制指令
    control_message = "前进 1 米"
    server_socket.sendto(control_message.encode(), addr)
    print(f"向 {addr} 发送控制指令：{control_message}")