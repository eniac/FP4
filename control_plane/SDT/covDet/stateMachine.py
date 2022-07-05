from network.packet import Packet 
import json
from enum import Enum

class TransitionType(Enum):
    NOCONDITION = 0
    CONDITION = 1
    CURRENT = 2

class StateMachine(object):
    def __init__(self, filename):
        self.start = "start"
        self.terminal = "ingress"

        jsonFile = open(filename)
        self.headerJson = json.loads(jsonFile.read())
        self.createStates()
        self.createExtract()
        self.createTransitions(self.headerJson["transitions"])

    def createStates(self):
        self.states = []
        self.defaultTransition = dict()
        for parserName in self.headerJson["states"]:
            print(parserName)
            self.states.append(parserName)
            self.defaultTransition[parserName] = None

    
    def createExtract(self):
        self.extract = dict()

        for parserName, headerList in self.headerJson["extract"].items():
            if parserName not in self.extract:
                self.extract[parserName] = []

            for header in headerList:
                self.extract[parserName].append(header)

        if self.start not in self.extract:
            self.extract[self.start] = []

    def createTransitions(self, transitionList):
        self.transitions = dict()
        for transition in transitionList:
            if transition["from"] not in self.transitions:
                self.transitions[transition["from"]] = []

            if transition["condition"] == 0:
                self.defaultTransition[transition["from"]] = transition["to"]
                # self.transitions[transition["from"]].append(Transition(transition["from"], transition["to"]))
            else:
                if "default" in transition["value"]:
                    self.defaultTransition[transition["from"]] = transition["to"]
                else:
                    self.transitions[transition["from"]].append(Transition(transition["from"], transition["to"], transition["condition"], transition["header"], transition["value"]))
                        


    def parsePacket(self, byteStreamObj):
        currentState = self.start
        packet = Packet(byteStreamObj)
        prevState = None
        while currentState != self.terminal:
            # print("currentState", currentState)
            prevState = currentState
            if currentState in self.extract:
                for headerName in self.extract[currentState]:
                    try:
                        # print("extracting:", headerName)
                        currentIndex = packet.addHeader(headerName)
                    except IndexError as e:
                        print("incomplete packet", headerName, "not found")
                        return None
                
            for transition in self.transitions[currentState]:
                # print("Checking transition:", transition)
                if transition.isTrue(packet):
                    currentState = transition.toState
                    break

            if prevState == currentState:
                currentState = self.defaultTransition[currentState]

        if Transition.convertBinArrayToHexStr(packet.headers["fp4_visited"]["preamble"]) != 14593470:
            print("Unknown packet - not from dataplane - discarding packet")
            return None


        return packet

class Transition(object):

    def __init__(self, _fromState, _toState, _condition=False, _matchWith=None, _value = None):
        self.fromState = _fromState
        self.toState = _toState
        self.conditionType = None
        self.rangeStart = None 
        self.rangeEnd = None
        if _value is not None and "default" in _value:
            self.conditionType = TransitionType.NOCONDITION
        else:
            headerAndField = _matchWith.split(".")
            if "mask" in _value:
                out = '0x'
                _value = _value.split(" ")
                for i in range(2, len(_value[0])):
                    out += str(int(_value[0][i],16) & int(_value[2][i],16))

                _value = out


            self.value = int(_value,16)
            if len(headerAndField) == 2:
                self.headerMatch = headerAndField[0]
                self.fieldMatch = headerAndField[1]
                self.conditionType = TransitionType.CONDITION
            else:
                self.rangeStart, self.rangeEnd = _matchWith.split("(")[1].split(")")[0].split(",")
                self.rangeStart = int(self.rangeStart)
                self.rangeEnd = int(self.rangeEnd)
                self.conditionType = TransitionType.CURRENT

    def isTrue(self, packet):
        if self.conditionType == TransitionType.NOCONDITION:
            return True

        elif self.conditionType == TransitionType.CONDITION:
            packetVal = Transition.convertBinArrayToHexStr(packet.headers[self.headerMatch][self.fieldMatch])
        else:
            packetVal = Transition.convertBinArrayToHexStr(packet.readFutureBits(self.rangeStart, self.rangeEnd))

        # print("input:", packet.headers[self.headerMatch][self.fieldMatch])
        # print("converted:", packetVal, type(packetVal))
        # print("transition value:", self.value, type(self.value))
        if packetVal == self.value:
            return True

        return False

    @staticmethod
    def convertBinArrayToHexStr(inputArray):
        return int("".join([str(x) for x in inputArray]),2)

    def __str__(self):
        out = "from: " + self.fromState + " to: " + self.toState

        if self.conditionType == TransitionType.CONDITION:
            out += " " + "match: " + self.headerMatch + "." + self.fieldMatch + " == " + self.value
        elif self.conditionType == TransitionType.CURRENT:
            out += " " + "match: current(" + str(self.rangeStart) + "," +  str(self.rangeEnd) + ") == " + str(self.value)
        return out



    def __repr__(self):
        out = "from: " + self.fromState + " to: " + self.toState
        if self.conditionType == TransitionType.CONDITION:
            out += " " + "match: " + self.headerMatch + "." + self.fieldMatch + " == " + self.value
        elif self.conditionType == TransitionType.CURRENT:
            out += " " + "match: current(" + str(self.rangeStart) + "," +  str(self.rangeEnd) + ") == " + str(self.value)
        return out
