import json

## fields[headerName] = [(field, bits), () ...]

class HeaderSynthesizer(object):
    def __init__(self, headerFileName):
        jsonFile = open(headerFileName)
        headerJson = json.loads(jsonFile.read())
        outString = ["class Header(object):\n"]
        headerTypeToName = dict()
        headerTypeToFields = dict()
        headerTypeSize = dict()
        totalSize = 0

        for header_type, fields in headerJson.items():
            if header_type == "type_to_instance":
                continue

            if header_type not in headerTypeToFields:
                headerTypeToFields[header_type] = list()

            if header_type not in headerTypeSize:
                headerTypeSize[header_type] = 0

            for fieldBit in fields:
                for f,fSize in fieldBit.items():
                    headerTypeToFields[header_type].append((f, int(fSize)))
                    headerTypeSize[header_type] += int(fSize)


        for header_type, header_name in headerJson["type_to_instance"].items():
            headerTypeToName[header_type] = header_name
            totalSize += headerTypeSize[header_type]


        outString.append("\tmaxLength = " + str(totalSize) + "\n")
        outString.append("\tfields = {\n")

        for header_type, header_name in headerTypeToName.items():
            outString.append('\t\t"' + header_name + '": [')
            for f, fSize in headerTypeToFields[header_type]:
                outString.append('("' + f + '",' + str(fSize) + ')')
                outString.append(',')

            outString.pop()
            outString.append(']')
            outString.append(',')
            outString.append('\n')

        outString.pop()
        outString.pop()
        outString.append('\n\t}')

        outFile = open("header.py", 'w')
        outFile.write("".join(outString))

import sys
if __name__ == '__main__':
    HeaderSynthesizer(sys.argv[1])
    
