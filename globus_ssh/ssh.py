import os
import pty
import psutil

from . import process

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

class InjectToken():
    PROMPT="Enter your Globus Auth token:"

    def __init__(self, access_token):
        self._access_token = access_token
        self._prompt_found = False
        self._data = ""

    def __call__(self, fd):
        if self._prompt_found == True:
            return os.read(fd, 1024)
        data = self._data + os.read(fd, 1024)
        self._data = ""
        if InjectToken.PROMPT in data:
            os.write(fd, self._access_token + "\n")
            self._prompt_found = True
            data = data.replace(InjectToken.PROMPT, "")
            if len(data) == 0:
                return self(fd)
            return data
        data = self._save_split_prompt(data)
        if len(data) == 0:
            return self(fd)
        return data

    def _trailing_prompt_chars(self, data):
        if data.endswith(InjectToken.PROMPT):
            return len(InjectToken.PROMPT)
        for i in range(1, len(InjectToken.PROMPT)):
            if data.endswith(InjectToken.PROMPT[:-i]):
                return len(InjectToken.PROMPT) - i
        return 0

    def _save_split_prompt(self, data):
        i = self._trailing_prompt_chars(data)
        if i == 0:
            return data
        self._data = data[len(data)-i:]
        return data[0:-i]

        
def run(access_token, ssh_args):
    """Spawn SSH with ssh_args and inject the access token."""

    return process.spawn(["ssh"] + list(ssh_args), InjectToken(access_token))
