import socket

class DataplaneSocket():
    def __init__(self, interface_name = 's2-ethc'):

        self.interface = interface_name

        self.dpSocket = socket.socket(socket.PF_PACKET, socket.SOCK_RAW, socket.htons(0x0003))
        self.dpSocket.bind((self.interface, 0))
        self.dpSocket.settimeout(5)
        # self.timeoutUpdated = False

    def send_packet(self, p, initial_bytes=7):
        # Send packet containing the data
        print("--- send_packet ---")

        prepend_bytes = "000000000000C0"
        for i in range(initial_bytes - 7):
            prepend_bytes += "00"

        prepend_bytes = prepend_bytes.decode("hex")
        p = prepend_bytes + p
        bytes_sent = self.dpSocket.send(p)
        print("bytes_sent: ", bytes_sent)

    def receive_packet(self):
        # Read data from the dataplane and return raw packet
        try:
            print("--- receive_packet ---")
            packet, addr = self.dpSocket.recvfrom(512)
            # print("str size:", len(packet))
            # if not self.timeoutUpdated:
            #     self.timeoutUpdated = True
            #     self.dpSocket.settimeout(1)
            #     print("Timeout Updated")
            packet = bytearray(packet)
            print("--- packet received of size: {} ---".format(len(packet)))
            # print(" ".join(str(byte) for byte in packet))
            hex_string = " ".join("{:02x}".format(byte) for byte in packet)
            print(hex_string)
            return packet
        except socket.timeout:
            print("No packet within timeout")

            return None

    # def print_test(self):
    #     print("hi")
