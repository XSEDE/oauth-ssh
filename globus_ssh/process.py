import os
import sys
import pty
import psutil
import termios

"""Home for all process-related functions shared across other modules."""

def _get_process():
    """Return a psutil.Process of the child process.

    Assumes exactly one child has been spawned by the parent calling this.
    """
    parent = psutil.Process(os.getpid())
    return psutil.Process(parent.children()[0].pid)

def _get_terminal_attrs():
    """Return a list of the terminal attributes"""

    return termios.tcgetattr(sys.stdin)

def _set_terminal_attrs(atts):
    """Set terminal attributes to the list provided"""
    termios.tcsetattr(sys.stdin, termios.TCSANOW, atts)

def spawn(argv, master_read):
    """spawn in a pseudo terminal and exit with its exit code when it completes"""

    atts = _get_terminal_attrs()

    # Relying on the SSH process to have displayed the real error message
    try:
        pty.spawn(argv, master_read)
        return _get_process().wait()
    except:
        _set_terminal_attrs(atts)
        return 255
