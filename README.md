# SSH with Globus Auth

SSH with Globus Auth delivers use of federated identities to the SSH
platform providing a uniform security infrastructure for institutions.
The solution supports both programmatic and interactive use, eliminating
the need for management of SSH keys and is a replacement for the
deprecated GSI SSH product.

SSH with Globus Auth is distributed as client and server packages, both
referred to as `globus-ssh`. The client is a Python package that manages
Globus Auth tokens on behalf of the user. The server package provides a
security module loadable into the SSH authentication process for
authentication via Globus Auth token introspect and account mapping.

Bugs reports and feature requests are open submission, and should be filed at
https://github.com/xsede/globus-ssh-server/issues.

The client-side components are located at [xsede/globus-ssh-client](https://github.com/xsede/globus-ssh-client).

# Server-Side Installation

The `globus-ssh` server package enables federated identity authorization
within the SSH service. It provides a PAM module which allows for
authorization of remote users based on identities linked across
institutional boundaries. This permits the service administrator to
authorize login access to a system based upon known local identities (ie
`johndoe@example.com`) while permitting the end user to authenticate as
the linked identity of their choice (ie `jd@foo.org`). This decoupling
for a simpler authorization scheme.

The PAM module also allows the service administrator to place additional
assurances on the user’s session in order to comply with local site
security policies. The SSH service may be configured to (1) require that
the user has previously authenticated with a specific identity provider
chosen by the service administrator (2) that the end user has
authenticated to an identity provider within a recent timeframe or (3)
both options 1&2. This gives the service administrator a higher
assurance that the connecting user is infact the authorized user.

## Supported Linux Distributions

The globus-ssh server packages are currently supported on the following
Linux distributions:

  - CentOS 7

## Register the SSH Service on Globus developers console

In order for globus-ssh to accept access tokens and perform account
mapping, it is first necessary to register this SSH installation with
Globus Auth. Visit
[developers.globus.org](https://developers.globus.org/) and select
"Register your app with Globus:"

1.  Visit [developers.globus.org](https://developers.globus.org/) and
    select "Register your app with Globus". Authenticate if asked.

2.  Click "Add another project" and fill out the form. The project
    provides a way to manage this registration, by adding other
    administrators.

3.  From the "Add.." menu for the project click "Add new app" and fill
    out the form.
    
    1.  Set "App Name" to "SSH@\<FQDN\>" where '\<FQDN\>' is the fully
        qualified domain name of the SSH service you are registering.
    
    2.  Leave "Native App" unselected.
    
    3.  Leave "Scopes" blank.
    
    4.  In the "Redirects" box, enter
        "https://auth.globus.org/v2/web/auth-code".
    
    5.  Leave "Required Identity" unselected.
    
    6.  Leave "Pre-select Identity Provider" unselected.
    
    7.  Leave "Privacy Policy" unselected.
    
    8.  Leave "Terms & Conditions" unselected.

4.  Select "Create App".

5.  Click "Generate a New Client Secret", fill out the form.

6.  Save the client_id and client_secret values for use in the
    globus-ssh.conf file later.

7.  It is also recommended that you add other appropriate users in your
    organization as administrators of the project for the sake of
    redundancy, and also to prevent the loss of administrative control
    of your Globus Project should any one project administrator leave
    your organization.

> **Note**
> 
> Each new SSH service requires a new Globus app registration, with its
> own client id and client secret. These can be within the same project
> or in different projects.

## Installation on CentOS 7

First, add the Globus repository to your package management system:

    $ sudo rpm --import https://downloads.globus.org/toolkit/gt6/stable/repo/rpm/RPM-GPG-KEY-Globus
    $ sudo yum install https://downloads.globus.org/toolkit/gt6/stable/installers/repo/rpm/globus-toolkit-repo-latest.noarch.rpm

Then, install the globus-ssh server packages:

    $ sudo yum install globus-ssh

## Configure /etc/globus/globus-ssh.conf

Edit /etc/globus/globus-ssh.conf to configure globus-ssh.

These first two options, `client_id` and `client_secret` allow
globus-ssh to communicate with Globus Auth in order to validate access
tokens passed by the client and to determine the client identity:

  - client_id <client_id>  
    Enter the client_id saved during app registration

  - client_secret <client_secret>  
    Enter the client_secret saved during app registration

The next two options configure globus-ssh for mapping from Globus Auth
identities to local user accounts. You must choose at least one of the
account mapping options, `idp_suffix` or `map_file`, or you can choose
both. If both options are enabled, no priority or ordering is given to
accounts mapped by either option; the user is able to choose the local
account from all accounts returned from either option.

  - idp_suffix \<string\>  
    Suffix of identity provider whose usernames can be used for local
    account names. This option can only be used once.

  - map_file \<path\>  
    Path to a text file that maps Globus identities to local users. This
    option may be used more than once.

### (Optional) IdP Suffix Account Mapping

When an identity provider (IdP) registers with Globus Auth, they are
assigned a unique suffix (ex. example.com) to identify identities issued
by that IdP. Globus Auth generates unique usernames of the form
'\<user\>@\<idp_suffix\>'. For example, the IdP example.com may have a
user 'joe' whose unique Globus identity would then be
'joe@example.com'.

A Globus account may have identities from multiple IdPs. For example, a
single Globus account may have identities 'joe@example.com' and
'joe224@foo.com'. These identities are linked to the same Globus
Account.

See <https://docs.globus.org/api/auth/specification/#identities> for
more details on how Globus account identities are formed and linked.

IdP Suffix account mapping is the process of mapping a Globus Auth
account to local accounts by selecting identities linked to the Globus
account that were issued by the given IdP and then dropping the IdP
suffix; the remainder is the local account user name. For example, if
`idp_suffix` was set to 'example.com', the account '<joe224@foo.com>'
would be ignored because it is from a different IdP. '<joe@example.com>'
would be chosen because of the matching IdP issuer, '@example.com' IdP
suffix would be dropped, and 'joe' would be an allowed local account
name.

### (Optional) Map File Account Mapping

When the config option `map_file` is enabled, the value is the absolute
path to a text file that contains mappings, one per line, from Globus
accounts to local account names. The format of this file is:

  - blank lines are ignored.

  - comments begin with '#' and are ignored.

  - one key, value pair per line, separated by space.

  - keys do not need to be unique, values will be merged for matching
    keys.

  - the user will be able to choose which account to u se based on any
    key matching any of his/her linked identities. Order within this
    file is not important.

  - leading spaces are ignored.

  - each key must be an Auth identity or username.

  - value must specify one or more local accounts delimited by space or
    commas.

Each line contains a single mapping. Each mapping is either:

  - \<globus-account-username\> to \<local-account\>

  - \<globus-account-id\> to \<local-account\>

`<globus-account-username>` is of the form `<user>@<idp_suffix>`,
for example, `joe@example.com`. `<globus-account-id>` is a standard
UUID format. These two formats may be in used throughout the map file.
`<local-account>` is a valid, local Linux user account (ex 'joe').

Consider the following example map file:

    joe227@foo.com                        joe
    8229a82e-d04c-478b-b2a9-f86219eee3d8  joe
    94855e14-2b0d-4d85-861b-4e7d155625a2  jane
    user123@exmaple.com                   bob
    2927e521-5582-4caf-897d-f978ec9a1c21  suzy

When a user connects to the SSH service, globus-ssh will query Globus
Auth for all identities linked to the Globus account associated with the
access token provided by the SSH client. Each linked identity is used to
search the map file for a matching `<globus-account-username>` or
`<globus-account-id>`. All matching entries are used for the account
mapping; ordering of entries within the file does not have any
consequence.

### High Assurance

The following two options allow the service administrator to impose
additional requirements upon the authentication performed by the end
user when authorizing their SSH client to access the local SSH service.
By using these options, the administrator will have an increased level
of confidence that the access request originated from the actual
authorized user.

### (Optional) Requring Use of Specific Identity Providers

  - permitted_idps idp1 [idp2]  
    List of identity providers a user must choose from to authenticate
    to during authorization prior to accessing the local SSH service.

### (Optional) Requring Reauthentication on a Given Cadence

  - authentication_timeout <minutes>  
    Number of minutes before a user is required to reauthenticate with
    the selected identity provider. If permitted_idps is specified, the
    authentication must occur from one of the IdPs listed there.
    Otherwise, an authentication with any IdP is sufficient.

## Register the service FQDN with Globus Auth

In order for users of the globus-ssh client to connect and authorize to
your SSH service, you must associate a fully qualified domain name
(FQDN) with your Globus Auth client registered above. This association
provides a mapping between the FQDN and your Auth client_id allowing
users to initiate an authorization flow using the FQDN of your SSH
service instead of knowing the client_id.

When you register the FQDN with Globus Auth and associate it with your
client_id, Globus Auth will verify your ownership of FQDN by retrieving
a DNS TXT record at <FQDN> and comparing the contents of that record
to your Globus Auth client_id. In order for registration of your FQDN
to succeed, the contents of the TXT record must match your Globus Auth
client_id.

You can verify the contents of a given DNS TXT record with the nslookup
utility. For example, if your Globus Auth client id was
`779714b7-d1c1-4678-9128-c3f4b536f2a5` and you wanted to associate the
FQDN `ssh.demo.globus.org` with your client registration, do:

    $ nslookup -type=txt ssh.demo.globus.org
    Server: 172.31.0.2
    Address: 172.31.0.2#53

    Non-authoritative answer: ssh.demo.globus.org text = "779714b7-d1c1-4678-9128-c3f4b536f2a5"

Note that the TXT record contents match the client ID exactly. In order
to associate the FQDN with the client ID, use globus-ssh-config:

    # /usr/sbin/globus-ssh-config register <fqdn>

By default, globus-ssh-config will retrieve the client_id and
client_secret from /etc/globus/globus-ssh.conf. You can override the ID
and secret using the commandline options `--client-id` and
`--client-secret`. See `globus-ssh-config --help` for more details.

## Configure SSHD to use PAM

Set the following values in /etc/ssh/sshd_config:

1.  Set `UsePAM` to `yes`

2.  Set `ChallengeResponseAuthentication` to `yes`

Restart sshd:

    # systemctl restart sshd.service

## Configure PAM to Use Globus

You must configure PAM to use pam_globus.so for sshd authentication.
The example below illustrates the modifications to PAM necessary on a
fresh EL7 installation. Your installation may already have modifications
to PAM in order to implement your site security policy. In that case,
this example should serve as a reference and you are strongly encouraged
to read PAM documentation (pam.conf(5)) before proceeding.

In order to route SSH authentication requests to pam_globus.so, replace
the 'auth' directives in /etc/pam.d/sshd with:

    auth        required      pam_sepermit.so
    auth        required      pam_env.so
    auth        [success=done maxtries=die new_authtok_reqd=done default=ignore]    pam_globus.so
    auth        requisite     pam_succeed_if.so uid >= 1000 quiet_success
    auth        required      pam_deny.so

pam_globus.so returns the following control values:

  - success  user has successfully authenticated

  - authinfo_unavail  unable to reach Globus Auth

  - auth_err  an unexpected error has occurred

  - maxtries  a `globus-mapping` request was processed successfully

  - user_unknown  the authenticated user requested an unmapped or non existent account

> **Note**
> 
> When the requested account is unknown, sshd will reprompt the user
> with the Globus access token prompt in order to avoid disclosing
> available accounts. This is by design for OpenSSH and is unavoidable.

## Developer Overview

**Compiling**

Makefile.bootstrap has two options of interest:
  debug: build the repo using '-DDEBUG -Wall -ggdb3'
  release: build the repo using '-O3' and generate an rpm

Both options will use sudo to install prerequisites.


**Testing**

Globus SSH relies on the [cmocka](https://cmocka.org/) testing library. CMOCKA is installed as part of the debug and release builds.


**Code Submissions**
1. Submit an issue with the [Globus SSH Repo](https://github.com/xsede/globus-ssh-server/issues).
2. Create a fork of the official Globus SSH repository for all work.
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

