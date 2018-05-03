import globus_sdk

from .token import Token
from .constants import *

import sys
if sys.version[0]=="3": raw_input=input

"""This module provides wrappers for calls to Globus Auth via Globus SDK"""

def _create_token(token_response):
    """Convert a SDK token response into our Token type."""
    # Assumptions:
    #   Only 1 token is ever returned from Auth (refresh or auth code)
    assert len(token_response.by_resource_server) == 1
    return Token(**list(token_response.by_resource_server.values())[0])


def is_token_valid(token_string):
    """Ask Auth if token_string is valid.

    token_string - character string token previously returned from Auth.
    returns tuple(<error_msg>, bool)
    """
    try:
        auth_client = globus_sdk.NativeAppAuthClient(CLIENT_ID)
        response = auth_client.oauth2_validate_token(token_string)
    except globus_sdk.GlobusError as e:
        return (e.message, None)

    return (None, response.data['active'] == True)


def refresh_token(token):
    """Use refresh token 'token' to get a new access token.

    returns (<errmsg>, <new_token>) where
        <errmsg> is an error message or None if no errors occur
        <new_token> is the token info returned from Auth. Ignore if <errmsg> is not None
    """
    try:
        auth_client = globus_sdk.NativeAppAuthClient(CLIENT_ID)
        token_response = auth_client.oauth2_refresh_token(token['refresh_token'])
    except globus_sdk.GlobusError as e:
        return (e.message, None)

    new_token = _create_token(token_response)
    return (None, new_token)


def revoke_token(token):
    """Call Auth to revoke the given token."""
    for t in ['access_token', 'refresh_token']:
        if token[t]:
            try:
                auth_client = globus_sdk.NativeAppAuthClient(CLIENT_ID)
                token_response = auth_client.oauth2_revoke_token(token['access_token'])
            except:
                pass # XXX what can we do here?


def do_auth_code_grant(fqdn):
    """Perform an Oauth2 authorization grant consent flow."""
    try:
        auth_client = globus_sdk.NativeAppAuthClient(CLIENT_ID)
        auth_client.oauth2_start_flow(requested_scopes=SCOPES_FORMAT.format(fqdn=fqdn),
                                      refresh_tokens=USE_REFRESH_TOKENS,
                                      prefill_named_grant=PREFILL_NAMED_GRANT)
        authorize_url = auth_client.oauth2_get_authorize_url()
        print('Please go to this URL and login: {0}'.format(authorize_url))
        auth_code = raw_input('Please enter the code you get after login here: ').strip()
        token_response = auth_client.oauth2_exchange_code_for_tokens(auth_code)
        new_token = _create_token(token_response)
        return (None, new_token)
    except globus_sdk.GlobusError as e:
        return (e.message, None)

