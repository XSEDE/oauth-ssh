import os
import sys
import tty
import select
import socket
import termios
import getpass # get the local username for SSH logins

from . import config as Config
from . import globus_auth as Auth
from .policy import Policy
from .token import Token
from .constants import *
from .account_map import AccountMap
from .ssh_service import SSHService
from .exceptions import OAuthSSHError
from .oauth_ssh_token import find_access_token

#####################################
#
# Decorator
#
#####################################

# Cleanly provide exception handling common to many of our sub commands
def handle_errors(func):
    def wrapper(*args, **kw):
        try:
            func(*args, **kw)
        except OAuthSSHError as e:
            print(e)
            sys.exit(1)
        sys.exit(0)
    return wrapper


#####################################
#
# Helpers
#
#####################################
# click would not handle:
    #  oauth-ssh -l <account> -p <port> ssh.foo.com ls -l /
# so I rolled my own

def show_help():
    print("Usage: oauth-ssh [OPTIONS] [user@]FQDN COMMAND [ARGS]...")
    print("")
    print("Options:")
    print("  -l <user> User to log in as on the remote machine.")
    print("  -p <port> Port the SSH services runs on. Defaults to 22.")
    print("  --help    Show this message and exit.")
    print("")

def parse_args():
    args = sys.argv[1:]

    account = None
    port = 22
    acct_fqdn = None
    i = 0
    while i < len(args):
        if args[i] == '-l':
            if len(args) == i:
                print("Missing value for option '-l'")
                sys.exit(1)
            account = args[i+1]
            i = i + 2
            continue

        if args[i] == '-p' or args[i] == '--port':
            if len(args) == i:
                print("Missing value for option '" + args[i] + "'")
                sys.exit(1)
            port = args[i+1]
            i = i + 2
            continue

        if args[i] == '--help' or args[i] == '-h':
            show_help()
            sys.exit(1)

        if args[i].startswith('-'):
            print("Error: no such option: " + str(args[i]))
            sys.exit(1)

        acct_fqdn = args[i]
        i = i + 1
        break

    if acct_fqdn is None:
        print("Missing FQDN.")
        sys.exit(1)

    return account, port, acct_fqdn, args[i:]


#####################################
#
# Entry point
#
#####################################

@handle_errors
def oauth_ssh():
    account, port, acct_fqdn, command = parse_args()

    fqdn = acct_fqdn.split('@')[-1]

    # Grab the access token
    access_token = find_access_token(fqdn)

    # Explicit account settings : Set on command line
    if len(acct_fqdn.split('@')) > 1:
        account = acct_fqdn.split('@')[0]

    # Implicit account settings: Use the account map
    if account is None:
        acct_map = Config.load_object(fqdn, AccountMap)
        if acct_map is None:
            acct_map = SSHService(fqdn, port).get_account_map(access_token)
            Config.save_object(fqdn, acct_map)
        if acct_map['permitted_accounts'] is not None:
            if getpass.getuser() in acct_map['permitted_accounts']:
                account = getpass.getuser()
            elif len(acct_map['permitted_accounts']) == 1:
                account = acct_map['permitted_accounts'][0]

    if account is None:
        print('Could not determine remote account to use. Please use -l <account>.')
        sys.exit(1)

    try:
        transport = SSHService(fqdn, port).login(access_token, account)
    except:
        raise

    channel = transport.open_session()
    channel.setblocking(0)

    if command is not None and len(command) > 0:
        channel.exec_command(' '.join(command))
    else:
        channel.get_pty()
        channel.invoke_shell()

    try:
        oldtty = None
        if os.isatty(sys.stdin.fileno()):
            # Enable one char at a time, don't wait on lines, but do interpret
            # escape characters
            oldtty = termios.tcgetattr(sys.stdin)
            tty.setcbreak(sys.stdin.fileno())
            if command is None or len(command) == 0:
            	# Forward escapes like control-C
            	tty.setraw(sys.stdin.fileno())
        channel.settimeout(None)

        watch_list = [channel, sys.stdin]
        while True:
            r, w, e = select.select(watch_list, [], [])
            if channel in r:
                try:
                    x = channel.recv(1024).decode(sys.stdout.encoding)
                    if len(x) == 0:
                        break
                    sys.stdout.write(x)
                    sys.stdout.flush()
                except socket.timeout:
                    pass

            if sys.stdin in r:
                x = sys.stdin.read(1)
                if len(x) == 0:
                    watch_list.remove(sys.stdin)
                else:
                    channel.send(x)
    finally:
        if oldtty is not None:
            termios.tcsetattr(sys.stdin, termios.TCSADRAIN, oldtty)

    sys.exit(channel.recv_exit_status())
