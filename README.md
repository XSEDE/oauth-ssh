#### Overview
This repo contains the globus-ssh and globus-scp Python wrappers that make SSH and SCP compatibile with Globus Auth. The wrappers launch the ssh/scp process, pass it all user supplied arguments, monitor for the Globus access token request prompt and injects the access token. globus-ssh also includes utilities for managing access tokens.

#### Supported Platforms
Tested on CentOS Linux release 7.4.1708 (Core). Please report other platforms.

#### Supported OpenSSH Versions
Tested with OpenSSH_7.4p1, OpenSSL 1.0.2k-fips  26 Jan 2017. Please report other platforms.

#### Non OpenSSH Products
The capability to display the access token for cut-n-paste into other SSH clients is forthcoming.

#### Supported Python Versions
Tested with Python 2.7.5.

#### Development Installation (Non PyPi)
sudo yum install epel-release  
sudo yum install python-pip  
sudo pip install virtualenv  
sudo yum install git  
sudo yum install gcc  
sudo yum install python-devel  
git clone git@github.com:globusonline/globus-ssh.git  
cd globus-ssh  
git checkout develop/prototype  
make develop  
. ./venv/bin/activate (if sh, bash)  

#### Usage
Usage: globus-ssh [OPTIONS] COMMAND [ARGS]...

Options:
  --help  Show this message and exit.

Commands:
  connect  Connect to the SSH service @ <fqdn>
  login    Get an access token for <fqdn>
  logout   Revoke all tokens for <fqdn>

Usage: globus-scp <OpenSSH scp args>

#### Examples
1. First attempt to get an access token for `ssh.demo.globus.org`: 
```
(venv) [centos] globus-ssh login ssh.demo.globus.org
Please go to this URL and login: https://auth.globus.org/v2/oauth2/authorize?code_challenge=nyK-0n6r2IiGajKe8WxgEw6x-1o6jaz4EYe1hXz0yP0&state=_default&redirect_uri=https%3A%2F%2Fauth.globus.org%2Fv2%2Fweb%2Fauth-code&prefill_named_grant=SSH+Client+with+Globus+Auth+%28Demo%29&response_type=code&client_id=892ee39b-545a-4505-965a-cff0c96f4e74&scope=https%3A%2F%2Fauth.globus.org%2Fscopes%2Fssh.demo.globus.org%2Fssh&code_challenge_method=S256&access_type=offline
Please enter the code you get after login here: UDuEvPPoK0lkfig1Cklr8Nb8L4vXfx
Login successful
```

2. Subsequent attempts refresh the access token:
```
(venv) [centos] globus-ssh login ssh.demo.globus.org
Login successful
```

3. Use 'logout' to revoke the access token for a specific host:
```
(venv) [centos] globus-ssh logout ssh.demo.globus.org
Logout successful
```

4. Connect with the special account 'globus-mapping' to view your available accounts:
```
(venv) [centos@ip-172-31-33-122 SSH]$ globus-ssh -l globus-mapping ssh.demo.globus.org

You can log in as jasonalt1, jasonalt2

Permission denied (publickey,gssapi-keyex,gssapi-with-mic).
```

Note: on your first connection attempt you may get the following SSH message. Enter 'yes' to continue.
```
The authenticity of host 'ssh.demo.globus.org (35.163.193.209)' can't be established.
ECDSA key fingerprint is SHA256:VgGgMeafzaLBufMPR8UQ1OJIrey92tU87YaG7YKeI8k.
Are you sure you want to continue connecting (yes/no)? yes
Warning: Permanently added 'ssh.demo.globus.org,35.163.193.209' (ECDSA) to the list of known hosts.
```

5. You can omit the 'connect' subcommand to globus-ssh:
```
(venv) [centos] globus-ssh ssh.demo.globus.org
Last login: Wed Feb 14 16:35:32 2018 from foo.com
[centos] 
```

6. globus-ssh and globus-scp pass through all OpenSSH client options:
```
globus-scp -v -r dir/ user@host:/dir/
```

### FAQ
Q: Why does using `globus-ssh <fqdn>` produce the error `An error occurred: invalid_grant`?
A: Your prior consent was rescinded (globus.org => login => account => manage your consents). To fix, perform
`globus-logout <fqdn>` then `globus-login <fqdn>`.
