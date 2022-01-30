# Client-Side Installation

The `oauth-ssh` client is an end-user package that provides
federated-identity authorization for SSH. By leveraging the `oauth-ssh`
product, users can authenticate and authorize to `oauth-ssh` enabled SSH
services using their preferred linked identity. It is comprised of two
programs: `oauth-ssh-token` which performs OAuth token management and
`oauth-ssh` provides a simple SSH v2 client.

It is available as a PyPi package at <https://pypi.org/project/oauth-ssh/>.

Currently this code works against the Globus Auth authorization service.

## Supported Platforms

This documentation describes installation on enterprise linux and it’s
derivatives. However, it is expected to work on any pip-capable
platform.

## Supported Python Versions

`oauth-ssh` is compatible with Python 2.7+ and 3.6+.

## Prerequisites

### Pip

`pip` is the Python package manager and is required for `oauth-ssh`
installation. Steps for installation depend on whether you have
administrative access on the target system. Details are outlined below.

Determine if pip is available:

    $ which pip
    /usr/bin/which:
    no pip in (/usr/local/bin:/usr/bin:/usr/local/sbin:/usr/sbin)

If you do not have Pip installed, then try these three solutions, in
order:

1.  Use a package manager

    If you’re already using a package manager like `apt`, `yum`,
    `choco`, or `brew`, you should install pip via that package manager.
    For example, on Enterprise Linux:

        $ sudo yum install -y epel-release
        $ sudo yum install -y python-pip

2.  Use `ensurepip`

    Try to bootstrap `pip` from the standard library:

        python -m ensurepip --default-pip

3.  Use `get-pip.py`

        $ curl "https://bootstrap.pypa.io/get-pip.py" -o "get-pip.py"
        $ python get-pip.py --user

## Installation

These steps allow a user to install `oauth-ssh` into their home
directory which is necessary when you do not have administrative access
to the target machine. This installation makes `oauth-ssh` available
only to the current user.

On **Linux** and **macOS**, you need to make sure that the install
directory is in your PATH. This is usually ~/.local/bin, and you can run
the following commands:

    $ echo "PATH=${PATH}:~/.local/bin" >> ~/.bash_profile
    $ echo "export PATH" >> ~/.bash_profile
    $ .  ~/.bash_profile

And finally, install the `oauth-ssh` client into your local target
directory:

    $ pip install --user oauth-ssh

## Usage Overview

The `oauth-ssh` client is comprised of two components: `oauth-ssh`
which is a basic Python-based SSH client implementation which supports
OAuth token authentication and `oauth-ssh-token` which provides
token management routines. You should use `oauth-ssh-token` to enable
SSH access to a remote host and then use `oauth-ssh` for accessing that
host.

### oauth-ssh-token

    Usage: oauth-ssh-token [OPTIONS] COMMAND [ARGS]

    Options:
       --help     Show this message and exit.

    Commands:
       authorize  Perform an interactive consent flow to get an access token.
       revoke     Revoke the access and refresh tokens
       show       Display stored details about FQDN

### oauth-ssh

    Usage: oauth-ssh [OPTIONS] [user@]FQDN COMMAND [ARGS]

    Options:
       -l <user> User to log in as on the remote machine
       -p <port> Port the SSH services runs on. Defaults to 22

## Examples

### Authorize oauth-ssh to Access a Host

Before connecting to a SSH service, you must authorize this client in
order to obtain a token for use with the SSH authentication process.
This step is only required once per SSH service you plan to connect to.

Use the `authorize` command to authorize the client and retrieve an
access token. Substitute your host in place of ssh.demo.globus.org:

    $ oauth-ssh-token authorize ssh.demo.globus.org
    Please go to this URL and login: https://auth.globus.org/v2/oauth2/authoriz
    e?code_challenge=nyK-0n6r2IiGajKe8WxgEw6x-1o6jaz4EYe1hXz0yP0&state=_default
    &redirect_uri=https%3A%2F%2Fauth.globus.org%2Fv2%2Fweb%2Fauth-code&prefill_
    named_grant=OAuth+SSH+Client+%28Demo%29&response_type=code&client_id=892ee3
    9b-545a-4505-965a-cff0c96f4e74&scope=https%3A%2F%2Fauth.globus.org%2Fscopes
    %2Fssh.demo.globus.org%2Fssh&code_challenge_method=S256&access_type=offline
    Please enter the code you get after login here:

Cut-n-paste the URL given in the output of the `authorize` command to
perform the authorization. Paste the authorization code provided by on
the webpage into the prompt of the `oauth-ssh-token` command.

    Please enter the code you get after login here: UDuEvPPoK0lkfig1Cklr8Nb8L4vXfx

> **Note**
>
> The authorization URL and code will change with every instantiation of
> the `authorize` command.

### Using oauth-ssh to Access a Host

Use `oauth-ssh` to access the host that we just authorized:

    $ oauth-ssh ssh.demo.globus.org
    Last login: Tue Mar 5 22:52:18 UTC 2019
    [<johndoe@ssh.demo.globus.org> ~]$

### Choosing the Remote User Account

When connecting to a remote SSH service, `oauth-ssh` will try to
simplify account selection. For example, if the remote SSH service is
configured to only allow your linked identities to log in as user
`janedoe`, `oauth-ssh` will automatically select `janedoe` as the
target remote account unless you explicitly request a different account
with the command line options `-l <user>` or `user@fqdn`. Account
selection is performed in the following order of precedence:

1.  Specified on the command line as user@fqdn.

2.  Specified on the command line using the `-l <user>` option.

3.  Your current local user account if that account name is permitted by
    the remote SSH service.

4.  Your only permitted remote account on the SSH service, if there is
    only one permitted account for your linked identities.

If `oauth-ssh` is unable to select a remote account from this ordering,
for example if you have multiple accounts permitted by the remote
service and none of those accounts match you current username, then you
must specify your account on the command line using `-l <user>` or
`user@fqdn`. In order to assist you with account selection,
`oauth-ssh-token` provides the `show accounts` option which will list
all accounts associated with your linked identities permitted on the
remote SSH service:

    $ oauth-ssh-token show accounts ssh.demo.globus.org
    permitted_accounts: ['johndoe', 'jd']

#### Using Third-Party SSH Clients

In order to support third-party clients, `oauth-ssh` provides the
command `show token` which can be used to display a useable access token
which can be cut-n-paste into another SSH client application. In order
to display the access token, first `authorize` then:

    $ oauth-ssh-token show token ssh.demo.globus.org
    AdfjklweidAADDdjee5ddSSD44DccgglksiejklsdDDD44

Then paste this access token at the token prompt:

    $ ssh ssh.demo.globus.org
    Enter your OAuth Token: AdfjklweidAADDdjee5ddSSD44DccgglksiejklsdDDD44
    Last login: Tue Mar 5 22:52:18 UTC 2019
    [johndoe@ssh.demo.globus.org ~]$

#### Authorize oauth-ssh with a Specific Linked Identity

SSH service administrators may require that you authenticate with a
specific linked identity in order to meet security requirements at their
site. In this case, the `authorize` command will insist that you provide
a linked identity associated with one of the identity providers required
by the site. For example, the site `ssh.demo.globus.org` below requires
that you authorize with an identity from either xsede.org or globus.org:

    $ oauth-ssh-token authorize ssh.demo.globus.org
    Use --identity to specify an account from one of ['xsede.org', 'globus.org']

You will need to reissue the `authorize` command and specify an identity
of yours from one of the listed providers:

    $ oauth-ssh-token authorize --identity johndoe@xsede.org ssh.demo.globus.org
    Please go to this URL and login: https://auth.globus.org/v2/oauth2/authorize
    ?code_challenge=nyK-0n6r2IiGajKe8WxgEw6x-1o6jaz4EYe1hXz0yP0&state=_default&r
    edirect_uri=https%3A%2F%2Fauth.globus.org%2Fv2%2Fweb%2Fauth-code&prefill_nam
    ed_grant=OAuth+SSH+Client+%28Demo%29&response_type=code&client_id=892ee39b-5
    45a-4505-965a-cff0c96f4e74&scope=https%3A%2F%2Fauth.globus.org%2Fscopes%2Fss
    h.demo.globus.org%2Fssh&code_challenge_method=S256&access_type=offline
    Please enter the code you get after login here:

This may require you to reauthenticate as the specified identity, this
in case johndoe@xsede.org. As with the previous authorization flow
example, cut-n-paste the resulting authorization code to complete the
process:

    Please enter the code you get after login here: UDuEvPPoK0lkfig1Cklr8Nb8L4vXfx

#### Site Required Reauthentication

SSH service admistrators may require that you reauthenticate on a
particular cadence in order to meet their site’s security policy. For
example the site `ssh.demo.globus.org` below requires that the user
reauthenticate with the identity provider every four hours. When this
timeout occurs, `oauth-ssh` will request that you reauthorize access
for that host:

    $ oauth-ssh ssh.demo.globus.org
    The access token no longer meets this site’s security policy requirements.
    Use `oauth-ssh-token authorize ssh.demo.globus.org`

> **Note**
>
> that this does not interrupt existing SSH sessions; only new
> connections are subject to reauthentication.

#### Revoking SSH Access to a Host

If you no longer wish to allow the local `oauth-ssh` client to access a
particular host, for example if you are going to leave the system
unintended, you can issue the `revoke` command which destroys the access
token and refresh token associated with that host:

    $ oauth-ssh-token revoke ssh.demo.globus.org

Further access to ssh.demo.globus.org will require you to rerun the
`authorize` command.

### Development Installation (Non PyPi)
In general, the build system supports the following syntax:
```shell
make {develop|test|release} [PYTHON_VERSION=<x.y.z>]
  develop - installation from source, recommended for new development
  test    - installation from source, run unit tests against install
  release - prepare a pypi package ready for upload

  PYTHON_VERSION is optional. When it is not given, virtualenv and the default
  system python installation is used to install oauth-ssh to 'venv_system/'.
  When PYTHON_VERSION is given, pyenv and pyenv-virtualenv are used to build
  a virtual environment against the given python version. You must choose a
  python version supported by pyenv.
```

Installation of prerequisites (Enterprise Linux and derivatives):
```shell
sudo yum install -y epel-release git
sudo yum install -y python2-pip
```

Installation for general development against the system default python:
```shell
sudo pip install virtualenv
git clone git@github.com:xsede/oauth-ssh.git
cd oauth-ssh
make develop
. ./venv_system/bin/activate
```
Installation for version-specific python debugging and testing:
```shell
git clone https://github.com/pyenv/pyenv.git
export PYENV_ROOT=`pwd`/pyenv
export PATH="${PYENV_ROOT}/bin:$PATH"
git clone https://github.com/pyenv/pyenv-virtualenv.git ${PYENV_ROOT}/plugins/pyenv-virtualenv
git clone git@github.com:xsede/oauth-ssh.git
cd oauth-ssh/client
make develop PYTHON_VERSION=x.y.z
. .${PYENV_ROOT}/venv_x.y.z/bin/activate
```
PyPi package creation:
```shell
sudo pip install virtualenv
git clone git@github.com:xsede/oauth-ssh.git
cd oauth-ssh/client
make release
```

**Tagging**

All client releases are tagged using the format CLIENT_&lt;version&gt;, ie. `CLIENT_0.10`. Building from master instead of from a
release tag may include newer changes not associated with the release. Always build from release tags for production use and when
creating packages for distribution.

**Code Submissions**
1. Submit an issue with the [OAuth SSH Repo](https://github.com/xsede/oauth-ssh/issues).
2. Create a fork of the official repository for all work.
3. Use the branch naming scheme `issue/<issue_id>` for all work related to the issue.
4. Submit source updates to your fork.
5. Squash all commits into a single commit with log message `Fixes #<issue_id>`. Add additoinal information as applicable but try to keep details for the change within the issue.
6. Prior to submitting the pull request:
* Rebase your issue branch on top of the official repository's master branch.
* Run build_rpm in the top level to make sure it still functions.
* Run `make test` to verify that all tests still pass.
  * Add new tests for new features as necessary.
7. Create a pull request. Make sure the subject includes `Closes #<issue_id>` in the description.
8. Submit the pull request.
9. When approving the pull request, use `rebase and commit`. Reject any pull requests that can't merge cleanly.
