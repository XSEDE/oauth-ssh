import os
import sys
import ast
import time
import base64
import urllib
import socket
import hashlib
import getpass
import requests



from .exceptions import OAuthSSHError
from .credential import Credential
from .constants import *
from .credential import Credential
from .token import Token
from . import config as Config

if sys.version_info.major==3:raw_input=input

if sys.version_info.major==3 and sys.version_info.minor > 4:
    from urllib.parse import urlencode as urlencode
else:
    from urllib import urlencode as urlencode

#if sys.version[0]=="3": raw_input=input

class GlobusAuthError(OAuthSSHError):
    "Base exception for all Globus Auth exceptions"""

def _perform_request(method, path, **kw):
    url = "https://" + GLOBUS_AUTH_HOST + path

    try:
        # Raises ConnectionError
        r = method(url, timeout=15, **kw)
        # Raises HTTPError
        r.raise_for_status()
    except requests.exceptions.RequestException as e:
        raise GlobusAuthError(e.message)
    return r

def register_client():
    path = '/v2/api/clients'
    name = 'Oauth SSH Client [' \
            + getpass.getuser() \
            + "@" \
            + socket.gethostname()  \
            + ']'

    body = {
        'client': {
            'template_id': TEMPLATE_ID,
            'name': name
        }
    }
    r = _perform_request(requests.post, path, json = body)
    return Credential(**r.json()['included']['client_credential'])

def _lookup_credentials():
    credentials = Config.load_object(DEFAULT_SECTION, Credential)
    if credentials is not None:
        return credentials

    credentials = register_client()
    Config.save_object(DEFAULT_SECTION, credentials)
    return credentials

def _authenticated_request(method, path, **kw):
    creds = _lookup_credentials()
    return _perform_request(method,
                            path,
                            auth=(creds['client'], creds['secret']),
                            **kw)


def lookup_identity(username):
    path = "/v2/api/identities?" + urlencode({'usernames': username})
    identities = _authenticated_request(requests.get, path).json()
    return identities['identities'][0]['id']

# {"access_token":"XXX","expires_in":172800,"resource_server":"30efa54d-081a-4098-8d02-325d6525dab2","token_type":"Bearer","other_tokens":[],"scope":"https://auth.globus.org/scopes/ssh.sandbox.globuscs.info/ssh","refresh_token":"XXX"}
def refresh_token(refresh_token):
    """Use the refresh token to get a new access token."""
    path = '/v2/oauth2/token'
    body = {'refresh_token': refresh_token, 'grant_type' : 'refresh_token'}
    r = _authenticated_request(requests.post, path, data = body)
    return ast.literal_eval(r.text)

def revoke_token(token):
    """Call Auth to revoke the given token."""
    path = "/v2/oauth2/token/revoke"
    body = { 'token' : token }
    _authenticated_request(requests.post, path, data = body)

# {"active": [true|false]}
def is_token_valid(token_string):
    """Ask Auth if token_string is valid."""
    path = "/v2/oauth2/token/validate"
    body = {'token': token_string}
    r = _authenticated_request(requests.post, path, data = body)
    return r.json()['active']

def _gen_code():
    verifier = base64.urlsafe_b64encode(
                      os.urandom(32)).decode("utf-8").rstrip("=")
    hashed = hashlib.sha256(verifier.encode("utf-8")).digest()
    challenge = base64.urlsafe_b64encode(hashed).decode("utf-8").rstrip("=")
    return (verifier, challenge)

def do_auth_code_grant(fqdn, force_login=False, identity=None):
    """Perform an Oauth2 authorization grant consent flow."""

    code_verifier, code_challenge = _gen_code()
    scope = (SCOPE_FORMAT.format(fqdn=fqdn))
    host  = GLOBUS_AUTH_HOST

    creds = _lookup_credentials()
    params = {
        'redirect_uri'         : 'https://' + host + '/v2/web/auth-code',
        'client_id'            :  creds['client'],
        'access_type'          : 'offline',
        'state'                : '_default',
        'code_challenge'       :  code_challenge,
        'code_challenge_method': 'S256',
        'response_type'        : 'code',
        'scope'                :  scope
    }

    if identity is not None:
        params['session_message'] = 'The SSH service requires that you authenticate using this identity:'
        params['session_required_identities'] = str(identity)

    if force_login is True:
        params['prompt'] = 'login'

    url = "https://" + host + '/v2/oauth2/authorize?' + urlencode(params)

    print('Please go to this URL and login: {0}'.format(url))
    auth_code = raw_input(
                    'Please enter the code you get after login here: ').strip()

    body = {
        'code'         :  auth_code,
        'code_verifier':  code_verifier,
        'redirect_uri' : 'https://' + host + '/v2/web/auth-code',
        'grant_type'   : 'authorization_code'
    }

    r = _authenticated_request(requests.post, '/v2/oauth2/token', data = body)
    return Token(authorized_at=int(time.time()), **ast.literal_eval(r.text))
