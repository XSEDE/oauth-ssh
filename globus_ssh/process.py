import os
import pty
import psutil

"""Home for all process-related functions shared across other modules."""

def _get_process():
    """Return a psutil.Process of the child process.

    Assumes exactly one child has been spawned by the parent calling this.
    """
    parent = psutil.Process(os.getpid())
    return psutil.Process(parent.children()[0].pid)

def spawn(argv, master_read):
    """spawn in a pseudo terminal and exit with its exit code when it completes"""
    pty.spawn(argv, master_read)
    return _get_process().wait()
