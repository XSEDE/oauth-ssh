import os
import sys
import pty
import psutil
import termios

"""Home for all process-related functions shared across other modules."""

def reset_terminal(function):
    def wrapper(var1, var2):
        atts = None
        if sys.stdout.isatty():
            atts = termios.tcgetattr(sys.stdout)
        ret = function(var1, var2)
        if atts is not None:
            termios.tcsetattr(sys.stdout, termios.TCSANOW, atts)
        return ret

    return wrapper

@reset_terminal
def spawn(argv, master_read):
    """spawn in a pseudo terminal and exit with its exit code when it completes"""

    # Relying on the SSH process to have displayed the real error message
    try:
        # Python2: no return value
        # Python3: (UNIX) return_code    = (pid, <exit_value<<8|signal>)
        #          (Windows) return_code = (pid, <exit_value<<8|0>)
        return_code = pty.spawn(argv, master_read)

        if return_code is not None:
            signal = return_code & 0xFF
            if signal != 0:
                return signal
            return ((return_code >> 8) & 0xFF)
        
        parent = psutil.Process(os.getpid())
        return psutil.Process(parent.children()[0].pid).wait()
    except Exception as e:
        return 255
