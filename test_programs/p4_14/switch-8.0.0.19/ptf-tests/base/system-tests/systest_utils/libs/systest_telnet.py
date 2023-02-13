import telnetlib
from utils import get_basic_logger

HOST = "localhost"
PORT = 9999

#TODO: Convert into context_manager
class telnet_helper(object):
    def __init__(self, host='localhost', port=9999, logger=None):
        """ Constructor class for telnet helper """
        self.host = host
        self.port = port
        self.logger = logger if logger != None else get_basic_logger(
            name='telnet', fileName='/tmp/telnet.log')
        self.logger.info("Opening telnet connection to host: %s on port: %s" %
                         (self.host, self.port))
        self.telnet_session = self._connect()
        self.telnet_session.read_until(">")

    def _connect(self):
        """ Opens the telnet connection and """
        return telnetlib.Telnet(self.host, self.port)

    def write(self, cmd):
        """ Wrapper around telnetlib write """
        self.logger.info("%s" %cmd)
        if "\n" not in cmd:
            cmd = cmd + '\n'
        self.telnet_session.write(cmd)

    def read_until(self, string=">"):
        """ Wrapper around telnetlib read_until """
        out = self.telnet_session.read_until(string)
        for line in out.split('\n'):
            self.logger.info(line)
        return out

    def expect(self, expect_list):
        """ Wrapper around telnetling expect """
        out = self.telnet_session.expect(expect_list)
        for line in out.split("\n"):
            print line
        return out

    def close(self):
        """ Close the session """
        if self.telnet_session != None:
            self.telnet_session.close()

    def get_show_config(self):
        """ Gets the output of show config command output on switchapi """
        cmd  = 'show config'
        self.write('switchapi')
        _ = self.read_until('>')
        self.write('show config')
        out = self.read_until('>')
        self.write('end')
        _ = self.read_until('>')
        return out

    def get_show_device(self):
        """ Gets the output of show device command output on switchapi """
        cmd = 'show config'
        self.write('switchapi')
        _ = self.read_until('>')
        self.write('show device')
        out = self.read_until('>')
        self.write('end')
        _  = self.read_until('>')
        return out
