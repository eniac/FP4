import socket
import sys

class DataplaneSocket():
    def __init__(self, interface_name = 's2-ethc'):

        self.interface = interface_name

        self.dpSocket = socket.socket(socket.PF_PACKET, socket.SOCK_RAW, socket.htons(0x0003))
        self.dpSocket.bind((self.interface, 0))
        self.dpSocket.settimeout(15)
        self.timeoutUpdated = False

    def send_packet(self, p):
        # Send packet containing the data
        # print("sending packet")
        bytes_sent = self.dpSocket.send(p)
        # print("sent bytes: ", bytes_sent)

    def receive_packet(self):
        # Read data from the dataplane and return raw packet
        try:
            packet, addr = self.dpSocket.recvfrom(512)
            # print("str size:", len(packet))
            if not self.timeoutUpdated:
                self.timeoutUpdated = True
                self.dpSocket.settimeout(1)
                print("--- Timeout Updated to 1s ---")
            packet = bytearray(packet)
            print("\n\n\n[INFO] Packet received of size: {}".format(len(packet)))
            sys.stdout.flush()

            return packet
        except socket.timeout:
            print("No packet within timeout")
            return None

    # def print_test(self):
    #     print("hi")
