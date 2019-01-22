import base64
import json

from .constants import *
from .account_map import AccountMap
from .transport import Transport
from .policy import Policy

####################################################################
#
# Public Command Interface
#
####################################################################
class SSHService(object):
    def __init__(self, fqdn, port):
        self._fqdn = fqdn
        self._port = port

    def get_security_policy(self):
        msg = {
            "command": { "op": "get_security_policy" },
            "version": PROTOCOL_VERSION
        }
        command = base64.b64encode(json.dumps(msg).encode("utf-8"))

        transport = Transport(self._fqdn, self._port)
        reply = transport.send_command(command, GLOBUS_ACCOUNT)
        policy = Policy(**reply['policy'])
        return policy

    def get_account_map(self, access_token):
        msg = {
            "command": {
                "op": "get_account_map",
                "access_token": access_token
            },
            "version": PROTOCOL_VERSION
        }
        command = base64.b64encode(json.dumps(msg).encode("utf-8"))

        transport = Transport(self._fqdn, self._port)
        reply = transport.send_command(command, GLOBUS_ACCOUNT)
        acct_map = AccountMap(**reply['account_map'])
        return acct_map

    def login(self, access_token, account):
        msg = {
            "command": {
                "op": "login",
                "access_token": access_token
            },
            "version": PROTOCOL_VERSION
        }
        command = base64.b64encode(json.dumps(msg).encode("utf-8"))

        transport = Transport(self._fqdn, self._port)
        reply = transport.send_command(command, account)
        return transport
