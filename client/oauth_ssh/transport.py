import paramiko
import warnings
import hashlib
import socket
import base64
import json
import sys
import os

from .exceptions import OAuthSSHError
from .constants import *

if sys.version[0]=="3": raw_input=input

class UnexpectedSSHReply(OAuthSSHError):
    def __init__(self, reply):
        msg = "Unexpected reply from the SSH service: " + str(reply)
        super(UnexpectedSSHReply, self).__init__(msg)

class HostLookupFailed(OAuthSSHError):
    def __init__(self, fqdn):
        msg = "ssh: Could not resolve hostname " \
               + fqdn \
               + ": Name or service not known"
        super(HostLookupFailed, self).__init__(msg)

class ConnectionTimeout(OAuthSSHError):
    def __init__(self, host_port):
        msg = "Timedout connecting to " + host_port
        super(ConnectionTimeout, self).__init__(msg)

class SessionViolation(OAuthSSHError):
    def __init__(self, fqdn):
        msg = "The access token no longer meets this site's security policy requirements. " \
            + "Use `oauth-ssh-token authorize " + fqdn + "`."
        super(SessionViolation, self).__init__(msg)

class AuthorizationFailure(OAuthSSHError):
    def __init__(self):
        msg = "Authorization to this host has failed. Likely causes are no local account or a misconfigured service."
        super(AuthorizationFailure, self).__init__(msg)

class InvalidToken(OAuthSSHError):
    def __init__(self):
        msg = "The access token is invalid." \
            + "Use `oauth-ssh-token authorize " + fqdn + "`."
        super(InvalidToken, self).__init__(msg)

class UnknownHostKey(OAuthSSHError):
    def __init__(self):
        super(UnknownHostKey, self).__init__("Host key verification failed.")

####################################################################
#
# Paramiko Transport Creation
#
####################################################################
FAMILY=0
SOCKTYPE=1
PROTO=2
SOCKADDR=4

def _lookup_host(fqdn, port):
    try:
        infos = socket.getaddrinfo(fqdn, port)
        infos = [a for a in infos if a[PROTO] == socket.IPPROTO_TCP]
    except socket.gaierror as e:
        if e.errno == -2:
            raise HostLookupFailed(fqdn)
        raise
    return infos[0]

def _connect_to_host(fqdn, port):
    info = _lookup_host(fqdn, port)
    try:
        s = socket.socket(info[FAMILY], info[SOCKTYPE], info[PROTO])
        s.settimeout(15)
        s.connect(info[SOCKADDR])
    except socket.timeout as e:
        raise ConnectionTimeout(fqdn + ':' + str(port))
    except:
        raise
    finally:
        s.settimeout(None)
    return s

####################################################################
#
# SSH Command/Reply Exchange
#
####################################################################
class CmdInjectHandler():
    def __init__(self, command):
        self._state = 0
        self._reply = None
        self._command = command

    def __call__(self, title, instructions, prompt_list):
        if self._state == 0:
            if len(prompt_list) > 0:
                if prompt_list[0][0] == PROMPT:
                    self._state = 1
                    return [self._command]
        elif self._state == 1:
            self._reply = instructions
            self._state = 0
        return []

def decode_reply(reply):
    try:
        return json.loads(base64.b64decode(reply.rstrip('\n')).decode("utf-8"))
    except Exception as e:
        raise UnexpectedSSHReply(reply)

class Transport(paramiko.transport.Transport):
    def __init__(self, fqdn, port):
        self._fqdn = fqdn
        self._port = port
        super(Transport, self).__init__(_connect_to_host(fqdn, port))
        self.start_client(timeout=15)
        remote_host_key = self.get_remote_server_key()
        known_hosts_file = os.path.expanduser('~/.ssh/known_hosts')

        # Treat missing/bad hosts files as unknown host
        try:
            known_host_keys = paramiko.hostkeys.HostKeys(filename=known_hosts_file)
        except:
            known_host_keys = paramiko.hostkeys.HostKeys()

        if known_host_keys.check(fqdn, remote_host_key) is False:
            print("The authenticity of host '" \
                + fqdn \
                + "' can't be established.")

            key_name = remote_host_key.get_name()
            if key_name == "ssh-ed25519":
                key_name = "ED25519"
            elif key_name == "ssh-ecdsa":
                key_name = "ECDSA"
            elif key_name == "ssh-dsa":
                key_name = "DSA"
            elif key_name == "ssh-rsa":
                key_name = "RSA"

            m = hashlib.sha256()
            m.update(remote_host_key.get_fingerprint())
            fingerprint = base64.b64encode(m.digest()).decode("utf-8")

            print(key_name + " key fingerprint is SHA256:" + fingerprint)

            answer = None
            while answer not in ("yes", "y", "no", "n"):
                answer = raw_input(
                  "Are you sure you want to continue connecting (yes/no)? ")

            if answer in ('no', 'n'):
                raise UnknownHostKey()

            known_host_keys.add(fqdn,
                                remote_host_key.get_name(),
                                remote_host_key)
            # Ignore host key file issues
            try:
                known_host_keys.save(known_hosts_file)
            except:
                pass

    def send_command(self, command, account):
        handler = CmdInjectHandler(command)
        try:
            self.auth_interactive(account, handler)
        except paramiko.AuthenticationException as e:
            # Authentication errors are always expected to generate JSON replies
            if handler._reply is None:
                raise AuthorizationFailure()
            reply = decode_reply(handler._reply)
            if 'error' in reply and 'code' in reply['error']:
                if reply['error']['code'] == 'SESSION_VIOLATION':
                    raise SessionViolation(self._fqdn)
                if reply['error']['code'] == 'INVALID_TOKEN':
                    raise InvalidToken()
                raise UnexpectedSSHReply(reply)
            return reply

