# Logging module for system-test.
# It creates systest_logger. Before create checks if it exisits.
import os
import logging
import time
import datetime

# Ideally shoyld be passing this from ptf.config['systest_logs_dir']
default_logs_dir = os.path.abspath("../../systest_logs")
print "Default logs dir : {}".format(default_logs_dir)
msg_form = "%(asctime)s :%(name)s :%(levelname)s :%(module)s :%(lineno)d :%(message)s"


def get_current_date(fmt="%d_%m_%y"):
    """ return current date """
    return datetime.date.today().strftime(fmt)


def get_current_time(fmt='%H_%M_%S'):
    """ Return current time  """
    return datetime.datetime.now().strftime(fmt)


def check_and_get_systest_logs_dir():
    """ Check if the logs directroy exisits , if not create \
        Also make a seperate logs directory for this run """
    if not os.path.isdir(default_logs_dir):
        os.mkdir(default_logs_dir)
    logs_dir = os.path.join(default_logs_dir, get_current_date())
    if not os.path.isdir(logs_dir):
        os.mkdir(logs_dir)
    logs_dir = os.path.join(logs_dir, get_current_time())
    if not os.path.isdir(logs_dir):
        os.mkdir(logs_dir)
    print "Logs_dir: {}".format(logs_dir)
    return logs_dir


def get_systest_logger(name='systemTest', fileName='systest.log'):
    """ Create the system logger, attach file handler """
    # Create/Fetch logger.
    handlers = logging.Logger.manager.loggerDict
    if name in handlers.keys():
        return handlers[name]
    logger = logging.getLogger(name)
    # Create handlers
    stderr_hand = logging.StreamHandler(sys.stderr)
    stderr_hand.setLevel(logging.INFO)
    # set formatting
    fmt = logging.Formatter(msg_form)
    stderr_hand.setFormatter(fmt)
    logfile_hand = logging.FileHandler(
        os.path.join(check_and_get_systest_logs_dir(), fileName))
    logfile_hand.setLevel(logging.INFO)
    logfile_hand.setFormatter(fmt)
    # attch these handlers to logger.
    logger.addHandler(stderr_hand)
    logger.addHandler(logfile_hand)
    return logger


def get_generic_logger(name='test', fileName = '/tmp/test.log'):
    """ Generic logger """
    handlers = logging.Logger.manager.loggerDict
    if name in handlers.keys():
        return handlers[name]
    logger = logging.getLogger(name)
    logger.setLevel(logging.INFO)
    # create the file logger
    fmt = logging.Formatter(msg_form)
    fHandler = logging.FileHandler(fileName)
    fHandler.setLevel(logging.INFO)
    fHandler.setFormatter(fmt)
    logger.addHandler(fHandler)
    return logger

def get_basic_logger(name='basic', fileName = '/tmp/test.log'):
    """ Get a very basic logger, No module info writing """
    handlers = logging.Logger.manager.loggerDict
    if name in handlers.keys():
        return handlers[name]
    logger = logging.getLogger(name)
    logger.setLevel(logging.INFO)
    # Create the file logger
    fHandler = logging.FileHandler(fileName)
    fHandler.setLevel(logging.INFO)
    logger.addHandler(fHandler)
    return logger

def remove_logger(logger, loggerName):
    """ Removes a logger defined by name """
    if loggerName in logger.handlers:
        logger.removeHandler(loggerName)
    return

def remove_all_loggers(logger):
    """ Remove all loggers """
    for handler in logger.handlers:
        logger.removeHandler(handler)
    return
