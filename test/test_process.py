from globus_ssh import process
import termios
import sys
import os

def _read(fd):
    data = os.read(fd, 1024)
    return data

def test_spawn_return_value():
    assert process.spawn("/usr/bin/false", _read) == 1
    assert process.spawn("/usr/bin/true", _read) == 0
    assert process.spawn("/", sys.stdin) == 255

def test_spawn_terminal_reset():
    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    new = termios.tcgetattr(fd)
    new[3] = new[3] & ~termios.ECHO
    termios.tcsetattr(fd, termios.TCSADRAIN, new)
    assert process.spawn("/", sys.stdin) == 255
    assert termios.tcgetattr(fd) == new
    termios.tcsetattr(fd, termios.TCSADRAIN, old)
