import os
import pty
import psutil

from . import process, constants
from .inject_token_once import InjectTokenOnce

"""
Module for handling interactions with SSH. Currently supports OpenSSH_7.4p1.
"""


#
#     ssh [-1246AaCfGgKkMNnqsTtVvXxYy] [-b bind_address] [-c cipher_spec] [-D [bind_address:]port] [-E log_file] [-e escape_char]
#         [-F configfile] [-I pkcs11] [-i identity_file] [-J [user@]host[:port]] [-L address] [-l login_name] [-m mac_spec]
#         [-O ctl_cmd] [-o option] [-p port] [-Q query_option] [-R address] [-S ctl_path] [-W host:port]
#         [-w local_tun[:remote_tun]] [user@]hostname [command]
#

_options_w_values=['-'+x for x in list("bcDEeFIiJLlmOopQRSWw")]
_options=['-'+x for x in list("1246AaCfGgKkMNnqsTtVvXxYy-")]

def find_host_name(arg_list):
    """Parse args intended for SSH to find the hostname."""

    arg_iter = iter([arg for arg in arg_list if arg not in _options])
    for arg in arg_iter:
        if arg in _options_w_values:
            next(arg_iter)
            continue
        return arg.split('@')[-1]
    return None

def run(access_token, ssh_args):
    """Spawn SSH with ssh_args and inject the access token."""

    return process.spawn(["ssh"] + list(ssh_args), InjectTokenOnce(constants.PROMPT, access_token + '\n'))
