# For your helper functions, No classes here.
import random
from contextFileReader import ContextFileReader as context

def GetWorkingDirectoy():
    """ Determine the working directory from python path if possible else return None """
    # TODO: Clean up and pass it as an argument from shell to python
    if os.environ.has_key('PYTHONPATH'):
        for i in os.environ['PYTHONPATH'].split(":"):
            if 'install' in i:
                if 'p4factory' in i.split('install')[0]:
                    return os.path.join(i.split('p4factory')[0], 'p4factory')
        # we are here we didn't find 'p4factory'
        for i in os.environ['PYTHONPATH'].split(":"):
            if 'python2.7' in i:
                return os.path.join(i.split('install')[0])
    return None


def BuildPathToContextJsonFile():
    """ Build path to the context.json file for switch P4 """
    work_dir = GetWorkingDirectoy()
    if work_dir != None:
        return os.path.join(work_dir, 'install', 'share', 'tofinopd', 'switch',
                            'context.json')
    return None


def get_context_file_handler():
    """ initialize and return handle to ContextFileReader """
    context_file = BuildPathToContextJsonFile()
    return context(context_file)


# Copied from api_utils.py
def macincrement(mac):
    mac = str(hex(int(mac.replace(':', ''), 16) + 1))[2:]
    mac = "00" + ":" + mac[0:2] + ":" + mac[2:4] + ":" + \
          mac[4:6] + ":" + mac[6:8] + ":" + mac[8:10]
    return mac


def get_random_mac_address():
    mac = [ 0x00, 0x16, 0x3e,
            random.randint(0x00, 0x7f),
            random.randint(0x00, 0xff),
            random.randint(0x00, 0xff) ]
    return ':'.join(map(lambda x: "%02x" % x, mac))

def generate_random_mac_address(numToGen):
    """ Generate mac addresses """
    return [get_random_mac_address() for i in range(0, numToGen)]

def get_n_random_intergers(start, stop, numIntegers):
    """ return n random numbers from the range start, stop """
    if (int(stop) - int(start)) < int(numIntegers):
        return []
    return random.sample(range(start, stop), numIntegers)
