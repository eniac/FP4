from header import Header
import numpy as np
from collections import OrderedDict

class Packet(object):
    def __init__(self, byteStream):
        self.headers = OrderedDict()
        self.headerStacks = dict()
        self.currentByteIndex = 0
        self.currentBitIndex = 0
        
        numBytes = min(Header.maxLength, len(byteStream))
        perByteArray = []

        for _byte in byteStream[:numBytes]:
            perByteArray.append([_byte])
        perByteArray = np.array(perByteArray, dtype=np.uint8)
        self.readBytes = np.unpackbits(perByteArray, axis=1)
        self.headers = OrderedDict()
        for headerName in Header.fields:
            self.headers[headerName] =  OrderedDict()

    def addHeader(self, headerName):
        if "[next]" in headerName:
            headerNameInField = headerName[:-6]
            if headerNameInField not in self.headerStacks:
                self.headerStacks[headerNameInField] = 0
            else:
                self.headerStacks[headerNameInField] += 1
        else:
            headerNameInField = headerName

        if "clone" in headerNameInField:
            headerNameInField = headerNameInField[:-6]
        else:
            headerNameInField = headerNameInField


        for field, numBits in Header.fields[headerNameInField]:
            if headerName not in self.headers:
                self.headers[headerName] = OrderedDict()
            self.headers[headerName][field] = self.getBits(numBits)

        self.headers["latest"] = self.headers[headerName]


    def getBits(self, numBits):
        out = []

        while numBits > 0:
            out.append(self.readBytes[self.currentByteIndex][self.currentBitIndex])
            self.currentBitIndex += 1
            if self.currentBitIndex == 8:
                self.currentBitIndex = 0
                self.currentByteIndex += 1

            numBits -= 1

        return out

    def readFutureBits(self, start, end):
        out = []

        bitIndex = self.currentBitIndex
        byteIndex = self.currentByteIndex
        while start > 0:
            bitIndex += 1
            if bitIndex == 8:
                bitIndex = 0
                byteIndex += 1
            start -= 1

        end = end - start

        while end > 0:
            out.append(self.readBytes[byteIndex][bitIndex])
            bitIndex += 1
            if bitIndex == 8:
                bitIndex = 0
                byteIndex += 1

            end -= 1

        return out

    def __str__(self):
        output = ""
        for header in self.headers:
            output += header + ":" + str(self.headers[header]) + "\n"
        return output


def createEmptyPacket(initalBits):
    packet_type = 0
    prefixString = ["00" for i in range(int(initalBits/8))]
    prefixString.append("0")

    # 6 bytes for ether src, 6 bytes for ether dst, 2 bytes for ethernet.etherType, 2 bytes visited.etherType
    hexadecimal_string = " ".join(prefixString) + str(packet_type) + ' 00 88 88 88 88 88 88 99 99 99 99 99 99 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00'

    packet = bytearray.fromhex(hexadecimal_string)
    return packet