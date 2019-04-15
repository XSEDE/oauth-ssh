import base64
import click
import json
import time

from . import constants
from . import config as Config
from . import globus_auth as Auth
from .account_map import AccountMap
from .ssh_service import SSHService
from .exceptions import OAuthSSHError
from .policy import Policy
from .token  import Token

class NeedToAuthorize(OAuthSSHError):
    def __init__(self, fqdn, msg):
        super(NeedToAuthorize, self).__init__(
             msg
           + " Use `oauth-ssh-token authorize " 
           + fqdn 
           + "`.")


####################################################################
#
#  Token Management
#
####################################################################
def revoke_token(fqdn, token_type):
    token = Config.load_object(fqdn, Token)
    if token is not None and token[token_type] is not None:
        # We do not ultimately care if revoke fails
        try:
            Auth.revoke_token(token[token_type])
        except Exception as e:
            click.echo("Warning: token revoke failed: " + str(e) + "(Ignored)")
        del token[token_type]
        if token_type == 'access_token':
            del token['expires_at']
        Config.save_object(fqdn, token)


def find_access_token(fqdn):
    token = Config.load_object(fqdn, Token)
    if token is None:
        raise NeedToAuthorize(fqdn, "No token found.")

    policy = Config.load_object(fqdn, Policy)
    if policy is None:
        raise NeedToAuthorize(fqdn, "No policy found.")

    if policy['authentication_timeout'] is not None:
        if policy['authentication_timeout'] > 0:
            timedout = True
            if token['authorized_at'] is not None:
                exp = token['authorized_at'] + (60*policy['authentication_timeout'])
                timedout = exp < time.time()
            if timedout is True:
                raise NeedToAuthorize(fqdn, "Authentication has timed out.")

    if token['access_token']:
        if Auth.is_token_valid(token['access_token']) == True:
            return token['access_token']
        del token['access_token']
        Config.save_object(fqdn, token)

    if token['refresh_token']:
        if Auth.is_token_valid(token['refresh_token']) == True:
            token.update(Auth.refresh_token(token['refresh_token']))
            revoke_token(fqdn, 'access_token')
            Config.save_object(fqdn, token)
            return token['access_token']
        del token['refresh_token']
        Config.save_object(fqdn, token)

    raise NeedToAuthorize(fqdn, "No valid token found.")

#####################################
#
# Decorator(s)
#
#####################################

# Cleanly provide exception handling common to many of our sub commands
def handle_errors(func):
    def wrapper(*args, **kw):
        try:
            func(*args, **kw)
        except OAuthSSHError as e:
            click.echo(e)
            click.get_current_context().exit(1)
        click.get_current_context().exit(1)
    return wrapper

#####################################
#
# BEGIN oauth-ssh-token SUBCOMMANDS
#
#####################################
@click.group()
def oauth_ssh_token():
    pass

#####################################
#
# oauth-ssh-token authorize
#
#####################################
@click.command('authorize', 
       short_help='Perform an interactive consent flow to get an access token.')
@click.option('--identity', nargs=1, metavar='<id>',
              help='Preferred Globus Auth identity to use during consent.')
@click.option('-p', '--port', nargs=1, metavar='<port>', default=22,
              help='Port that the SSH services runs on. Defaults to 22.')
@click.argument('fqdn', 'FQDN')
@handle_errors
def token_authorize(fqdn, port, identity):
    # If we are going to authorize, might as well make a clean start
    revoke_token(fqdn, 'access_token')
    revoke_token(fqdn, 'refresh_token')
    Config.delete_section(fqdn)

    # Grab the policy
    policy = SSHService(fqdn, port).get_security_policy()
    Config.save_object(fqdn, policy)

    if policy['permitted_idps'] is not None:
        if len(policy['permitted_idps']) > 0 and identity is None:
            click.echo("Use --identity to specify an account from one of "
                       + str(policy['permitted_idps']))
            click.get_current_context().exit(1)

    if identity is not None:
        identity = Auth.lookup_identity(identity)

    # Authentication timeout
    force_login = False
    if policy['authentication_timeout'] is not None:
         if policy['authentication_timeout'] > 0:
            force_login = True

    token = Auth.do_auth_code_grant(fqdn, force_login, identity)
    revoke_token(fqdn, 'access_token')
    revoke_token(fqdn, 'refresh_token')
    Config.save_object(fqdn, token)
    
oauth_ssh_token.add_command(token_authorize)

###################################
#
# oauth-ssh-token revoke
#
###################################
@click.command('revoke', short_help='Revoke the access and refresh tokens')
@click.argument('fqdn', 'FQDN')
@handle_errors
def token_revoke(fqdn):
    revoke_token(fqdn, 'access_token')
    revoke_token(fqdn, 'refresh_token')
    Config.delete_section(fqdn)

oauth_ssh_token.add_command(token_revoke)

###################################
#
# oauth-ssh-token show
#
###################################
@click.group('show', short_help='Display stored details about FQDN.')
def token_show():
    pass

oauth_ssh_token.add_command(token_show)

###################################
#
# oauth-ssh-token show account_map
#
###################################
@click.command('accounts',
                short_help='Display the accounts available for FQDN.')
@click.option('-p', '--port', nargs=1, metavar='<port>', default=22,
              help='Port that the SSH services runs on. Defaults to 22')
@click.argument('fqdn', 'FQDN')
@handle_errors
def show_accounts(fqdn, port):
    acct_map = Config.load_object(fqdn, AccountMap)

    if acct_map is None:
        access_token = find_access_token(fqdn)
        acct_map = SSHService(fqdn, port).get_account_map(access_token)
        Config.save_object(fqdn, acct_map)

    for k in acct_map.keys():
        click.echo(k + ': ' + str(acct_map[k]))

token_show.add_command(show_accounts)

###################################
#
# oauth-ssh-token show token
#
###################################
@click.command('token', short_help='Display the token for FQDN.')
@click.argument('fqdn', 'FQDN')
@handle_errors
def show_token(fqdn):
    click.echo(find_access_token(fqdn))

token_show.add_command(show_token)
