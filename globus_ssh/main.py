import os
import click
from . import auth, ssh, scp, token
from .config import Config
from .constants import *

"""Entry point for globus-ssh and globus-scp.

This module is the center of all business logic for this package.
"""

def load_token(fqdn):
    """Wrap the similar call to Config to handle a missing config file."""
    try:
        config = Config(os.path.expanduser(CONFIG_FILE))
    except:
        return None

    return config.load_token(fqdn)


def save_token(fqdn, token):
    """Wrap the similar call to Config to handle a missing config file."""
    try:
        config = Config(os.path.expanduser(CONFIG_FILE))
    except:
        return None
    config.save_token(fqdn, token)


def delete_token(fqdn):
    """Wrap the similar call to Config to handle a missing config file."""
    try:
        config = Config(os.path.expanduser(CONFIG_FILE))
    except:
        return None
    config.delete_token(fqdn)


# returns (<errmsg>, <token>)
# if <token> is not None, it is safe to assume that access_token is valid
def try_get_access_token(fqdn):
    """Non interactively search for a valid access token, refresh if necessary."""
    t = load_token(fqdn)
    s = SCOPES_FORMAT.format(fqdn=fqdn)

    if t and token.has_scopes(t, s):
        if t['access_token']:
            (errmsg, validity) = auth.is_token_valid(t['access_token'])
            if errmsg:
                return (errmsg, None)
            if validity:
                return (None, t)

        if t['refresh_token']:
            (errmsg, new_token) = auth.refresh_token(t)
            if errmsg:
                return (errmsg, None)
            return (None, new_token)

    return (None, None)


class DefaultToConnect(click.Group):
    """Click Group that directs unknown subcommands to 'connect'"""

    def get_command(self, ctx, cmd_name):
        cmd = click.Group.get_command(self, ctx, cmd_name)
        if cmd is not None:
            return cmd
        else:
            ctx.obj = cmd_name
            return click.Group.get_command(self, ctx, 'connect')


@click.command(context_settings=dict(ignore_unknown_options=True))
@click.argument('scp_args', nargs=-1, type=click.UNPROCESSED)
def globus_scp(scp_args):
    """globus-scp entry point. Find an access token and exec scp."""

    fqdn = scp.find_host_name(scp_args)
    if fqdn is None:
        click.echo('The hostname could not be found')
        click.get_current_context().exit(1)

    (errmsg, token) = try_get_access_token(fqdn)
    if errmsg:
        click.echo ("An error occurred: " + errmsg)
        click.get_current_context().exit(1)

    if token is None:
        click.echo("Use globus-ssh login " + fqdn)
        click.get_current_context().exit(1)

    save_token(fqdn, token)

    exit_code = scp.run(token['access_token'], scp_args)
    click.get_current_context().exit(exit_code)


@click.group(cls=DefaultToConnect, context_settings=dict(ignore_unknown_options=True))
def globus_ssh():
    """Access token management and SSH wrapper command"""

    pass


@globus_ssh.command()
@click.argument('hostname')
def login(hostname):
    """Get an access token for <hostname>"""

    (errmsg, token) = try_get_access_token(hostname)
    if errmsg:
        click.echo ("An error occurred: " + errmsg)
        click.get_current_context().exit(1)

    if token is None:
        (errmsg, token) = auth.do_auth_code_grant(hostname)
        if errmsg:
            click.echo ("An error occurred: " + errmsg)
            click.get_current_context().exit(1)

    save_token(hostname, token)
    click.echo ("Login successful")
    click.get_current_context().exit(0)


@globus_ssh.command()
@click.argument('hostname')
def logout(hostname):
    """Revoke all tokens for <hostname>"""

    token = load_token(hostname)
    if token:
        auth.revoke_token(token)
        delete_token(hostname)

    click.echo('Logout successful')


@globus_ssh.command(context_settings=dict(ignore_unknown_options=True))
@click.argument('ssh_args', nargs=-1, type=click.UNPROCESSED)
@click.pass_context
def connect(ctx, ssh_args):
    """Connect to the SSH service @ <hostname>"""

    if ctx.obj is not None:
        ssh_args = (ctx.obj,) + ssh_args

    fqdn = ssh.find_host_name(ssh_args)
    if fqdn is None:
        click.echo('The hostname could not be found')
        click.get_current_context().exit(1)

    (errmsg, token) = try_get_access_token(fqdn)
    if errmsg:
        click.echo ("An error occurred: " + errmsg)
        click.get_current_context().exit(1)

    if token is None:
        click.echo("Use globus-ssh login " + fqdn)
        click.get_current_context().exit(1)

    save_token(fqdn, token)

    exit_code = ssh.run(token['access_token'], ssh_args)
    click.get_current_context().exit(exit_code)

