from . import process, constants
from .inject_token_once import InjectTokenOnce

""" Module for handling interactions with SCP. Currently supports OpenSSH_7.4p1."""

#usage: scp [-12346BCpqrv] [-c cipher] [-F ssh_config] [-i identity_file]
#           [-l limit] [-o ssh_option] [-P port] [-S program]
#           [[user@]host1:]file1 ... [[user@]host2:]file2

_options_w_values=['-'+x for x in list("cFiloPS")]
_options=['-'+x for x in list("12346BCpqrv-")]

def find_host_name(arg_list):
    """Parse args intended for SCP to find the hostname."""

    arg_iter = iter([arg for arg in arg_list if arg not in _options])
    for arg in arg_iter:
        if arg in _options_w_values:
            next(arg_iter)
            continue
        host_file = arg.split('@')[-1].split(':')
        if len(host_file) >= 2:
            return host_file[0]
        continue
    return None

 
def run(access_token, scp_args):
    """Spawn SCP with scp_args and inject the access token."""

    return process.spawn(["scp"] + list(scp_args), InjectTokenOnce(constants.PROMPT, access_token + '\n'))
