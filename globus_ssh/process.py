import os
import sys
import pty
import psutil
import termios

"""Home for all process-related functions shared across other modules."""

def spawn(argv, master_read):
    """spawn in a pseudo terminal and exit with its exit code when it completes"""

    atts = termios.tcgetattr(sys.stdin)

    # Relying on the SSH process to have displayed the real error message
    try:
        pty.spawn(argv, master_read)
        parent = psutil.Process(os.getpid())
        return psutil.Process(parent.children()[0].pid).wait()
    except:
        termios.tcsetattr(sys.stdin, termios.TCSANOW, atts)
        return 255
