"""
Implementation of the pystate tracking.

Includes basic functionality for CRC32 codes.

Source and license at: https://github.com/cdstanford/pystate
"""

import pickle
import unittest

"""
CRC constants
"""

# Polynomial:
#   x^32 + x^26 + x^23 + x^22 + x^16
#     + x^12 + x^11 + x^10 + x^8 + x^7
#     + x^5 + x^4 + x^2 + x + 1
POLY_33 = 0b100000100110000010001110110110111
POLY_32  = 0b00000100110000010001110110110111
MAX_32   = 0b11111111111111111111111111111111

SHIFT_FWD1 = 0b00000000000000000000000000000010
SHIFT_BCK1 = 0b10000010011000001000111011011011
SHIFT_FWD8 = 0b00000000000000000000000100000000
SHIFT_BCK8 = 0b10101001110100111110011010100110

SHIFT_BCK_32 = 0xcbf1acda

"""
Polynomial arithmetic (on 32-bit values)
"""
def crc_add(x, y):
    return x ^ y

def crc_mul(x, y):
    result = 0

    # Multiply stage
    for i in range(32):
        result ^= (y << i) if (x & (1 << i)) else 0

    # Reduce mod 'poly'
    for i in range(31, -1, -1):
        result ^= (POLY_33 << i) if (result & (1 << (32 + i))) else 0

    assert result <= MAX_32
    return result

"""
CRC32 calculation auxiliary functions and primitives
"""
def rev_bits(x, width=32):
    # Reverse-bits function
    x_bin = bin(x)[2:].zfill(width)
    x_bin_rev = ''.join(reversed(x_bin))
    return int(x_bin_rev, 2)

def crc_push(c, byte):
    c ^= byte
    c = crc_mul(c, SHIFT_FWD8)
    return c

def crc_pop(c, byte):
    c = crc_mul(c, SHIFT_BCK8)
    c ^= byte
    return c

def crc_push_bytes(c, bytes):
    for byte in bytes:
        c = crc_push(c, byte)
    return c

def crc_pop_bytes(c, bytes):
    for byte in reversed(bytes):
        c = crc_pop(c, byte)
    return c

"""
True CRC32 calculation agreeing with a standard implementation such
as binascii.crc32.

Note that this reverses the bits of each byte. Since we are the
ones providing the bytes and can use whatever convention we want,
we omit this in our code and use crc_push / crc_pop directly instead.
"""

def crc32(x, init=0):
    result = rev_bits(init ^ MAX_32)
    for byte in x:
        result = crc_push(result, rev_bits(byte))
    return rev_bits(result ^ MAX_32)

"""
CRC transformations

This code calculates a transformation that can be then applied to any CRC
code, rather than just updating a given initial CRC code.

The transformation is represented as a triple (m, m', b),
where the forward transformation is x |-> mx + b,
and the backward transformation is x |-> m'(x + b).
Here, m and m' are inverses, i.e. m m' = 1.

Note: these are currently unused.
"""

def crc_transformation(bytes, init=(1, 1, 0)):
    m_fwd, m_bck, b = init
    for byte in bytes:
        m_fwd = crc_push(m_fwd, 0)
        m_bck = crc_pop(m_bck, 0)
        b = crc_push(b, byte)
    return (m_fwd, m_bck, b)

def compose_transformations(tfm1, tfm2):
    m_fwd1, m_bck1, b1 = tfm1
    m_fwd2, m_bck2, b2 = tfm2
    m_fwd = crc_mul(m_fwd1, m_fwd2)
    m_bck = crc_mul(m_bck1, m_bck2)
    b = crc_add(crc_mul(b1, m_fwd2), b2)
    return (m_fwd, m_bck, b)

def apply_fwd(tfm, b):
    m_fwd, _, b = tfm
    x = crc_mul(x, m_fwd)
    x ^= b
    return x

def apply_bck(tfm, b):
    _, m_bck, b = tfm
    x ^= b
    x = crc_mul(x, m_bck)
    return x

"""
Pickle wrapper -- convert an arbitrary object to a sequence of bytes

As of Python 3, pickle.dumps returns a bytes object, but in Python 2.7,
it returned a string, so we need to convert it to bytes.
The following code works in both Python 2 and Python 3.
"""
def pickle_bytes(x):
    return bytearray(pickle.dumps(x))

def try_pickle_bytes(x):
    try:
        return pickle_bytes(x)
    except (TypeError, pickle.PicklingError):
        return None

"""
The core superclass and decorators that do CRC tracking automatically.

- Superclass wrapper which allows calling .get_crc()
- Method decorators which update the stack-based CRC code
- .is_new() checks whether the current CRC is new since the last call
  to the function. A set is used to store the past CRC values.
"""
class TrackState:
    def __init__(self):
        self._stack_crc = MAX_32
        self._seen = set()

    def get_crc(self):
        c = self._stack_crc

        # Get a pickled byte sequence for the attributes.
        # Bad: the pickle of self.__dict__ includes our attribute _seen!
        # Old solution was to temporarily remove this before pickling: like
        # self._seen, tmp_seen = set(), self._seen
        # Now we ignore all attributes with _ at the start.

        # Pickle remaining attributes, filtering out:
        #   - attributes starting with an underscore
        #   - non-picklable cases (TODO)
        for attr in sorted(self.__dict__.keys()):
            if attr[0] != '_':
                attr_val = self.__dict__[attr]
                attr_pickle = try_pickle_bytes(attr_val)
                if attr_pickle is not None:
                    c = crc_push_bytes(c, attr_pickle)

        return c ^ MAX_32

    def is_new(self):
        c = self.get_crc()
        if c in self._seen:
            return False
        else:
            self._seen.add(c)
            return True

    def debug_str(self):
        # Return current CRC and whether it's new
        # Also performs the same update as in .is_new()
        c = self.get_crc()
        if c in self._seen:
            return "CRC {} (old)".format(c)
        else:
            self._seen.add(c)
            return "CRC {} (new)".format(c)

# Mandatory decorator for the __init__ function
def track_init(f):
    def init_super(self, *args, **kwargs):
        self._stack_crc = MAX_32
        self._seen = set()
        self._print_stack_calls = False
        f(self, *args, **kwargs)
        self.is_new() # initialize seen set
    return init_super
# Alternative version for __init__ if printing stack calls is desired.
# Useful for debugging purposes.
def track_init_print_stack_calls(f):
    def init_super(self, *args, **kwargs):
        self._stack_crc = MAX_32
        self._seen = set()
        self._print_stack_calls = True
        f(self, *args, **kwargs)
        print("Init, {}".format(self.debug_str()))
    return init_super

# Optional decorator for any function calls to track
def track_stack_calls(f):
    def deco(self, *args, **kwargs):
        call = (f.__name__, args, kwargs)
        call_pickle = pickle_bytes(call)
        self._stack_crc = crc_push_bytes(self._stack_crc, call_pickle)
        if self._print_stack_calls:
            print("Call {}, {}".format(call, self.debug_str()))
        result = f(self, *args, **kwargs)
        self._stack_crc = crc_pop_bytes(self._stack_crc, call_pickle)
        if self._print_stack_calls:
            print("Return, {}".format(self.debug_str()))
        return result
    return deco

"""
Unit tests
"""
class TestCrc32(unittest.TestCase):
    def test_constants(self):
        assert MAX_32 == 4294967295
        assert POLY_32 <= MAX_32
        assert POLY_32 + MAX_32 + 1 == POLY_33

    def test_mul(self):
        # Matching regular multiplication
        assert crc_mul(0, 0) == 0
        assert crc_mul(0, 37) == 0
        assert crc_mul(1, 1) == 1
        assert crc_mul(1, 255) == 255
        assert crc_mul(2, 2) == 4
        assert crc_mul(2, 12) == 24
        assert crc_mul(8, 9) == 72
        assert crc_mul(8, 16) == 128
        assert crc_mul(50, 5) == 250
        assert crc_mul(2, 2**30) == 2**31
        # Matching Nim multiplication, but no overflow
        assert crc_mul(3, 3) == 5
        assert crc_mul(4, 8) == 32
        assert crc_mul(9, 9) == 65
        assert crc_mul(41, 5) == 141
        # Overflow cases
        assert crc_mul(2, 2**31) == POLY_32
        assert crc_mul(2, MAX_32) == POLY_32 ^ MAX_32 ^ 1
        assert crc_mul(3, MAX_32) == POLY_32 ^ 1
        assert crc_mul(8, 2**31) == 4 * POLY_32
        assert crc_mul(128, 2**31) == (64 * POLY_32) ^ POLY_33

    def test_mul_shift_fwd(self):
        shift1 = 2
        shift2 = crc_mul(shift1, shift1)
        shift4 = crc_mul(shift2, shift2)
        shift8 = crc_mul(shift4, shift4)
        assert shift8 == 256
        assert shift1 == SHIFT_FWD1
        assert shift8 == SHIFT_FWD8

    def test_mul_shift_bck(self):
        shift1 = SHIFT_BCK1
        shift2 = crc_mul(shift1, shift1)
        shift4 = crc_mul(shift2, shift2)
        shift8 = crc_mul(shift4, shift4)
        assert shift8 == SHIFT_BCK8

    def test_mul_inverses(self):
        assert crc_mul(1, 1) == 1
        assert crc_mul(SHIFT_FWD1, SHIFT_BCK1) == 1
        assert crc_mul(SHIFT_FWD8, SHIFT_BCK8) == 1
        assert crc_mul(POLY_32, SHIFT_BCK_32) == 1

    def test_rev_bits(self):
        assert rev_bits(1, 1) == 1
        assert rev_bits(1, 2) == 2
        assert rev_bits(3, 2) == 3
        assert rev_bits(0, 4) == 0
        assert rev_bits(3, 4) == 12
        assert rev_bits(6, 4) == 6
        assert rev_bits(10, 4) == 5

    """
    TODO: Failing unit tests
    """

    def test_crc32_easy(self):
        assert crc32(bytearray(b"")) == 0
        assert crc32(bytearray(b"\xFF")) == 0xff000000

    def test_crc32_medium(self):
        assert crc32(bytearray(b"\x00")) == 0xd202ef8d
        assert crc32(bytearray(b"a")) == 0xe8b7be43
        assert crc32(bytearray(b"abc")) == 0x352441c2
        assert crc32(bytearray(b"cat")) == 0x9e5e43a8

    def test_crc32_hard(self):
        assert crc32(bytearray(b"a" * 100)) == 0xaf707a64

if __name__ == "__main__":
    # Run unit tests
    unittest.main()
