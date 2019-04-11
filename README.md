#### Overview

Provides Globus Auth token management and a basic SSH client that performs Globus Auth authorization.

Source Code: https://github.com/xsede/globus-ssh-client

Bugs reports and feature requests are open submission, and should be filed at
https://github.com/xsede/globus-ssh-client/issues

The PyPi project can be found at: https://pypi.org/project/globus-ssh/

#### Development Installation (Non PyPi)
In general, the build system supports the following syntax:
```shell
make {develop|test|package} [PYTHON_VERSION=<x.y.z>]
  develop - installation from source, recommended for new development
  test    - installation from source, run unit tests against install
  package - prepare a pypi package ready for upload
  
  PYTHON_VERSION is optional. When it is not given, virtualenv and the default
  system python installation is used to install globus-ssh to 'venv_system/'.
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
git clone git@github.com:globusonline/globus-ssh.git  
cd globus-ssh  
make develop
. ./venv_system/bin/activate  
```
Installation for version-specific python debugging and testing:
```shell
git clone https://github.com/pyenv/pyenv.git
export PYENV_ROOT=`pwd`/pyenv
export PATH="${PYENV_ROOT}/bin:$PATH"
git clone https://github.com/pyenv/pyenv-virtualenv.git ${PYENV_ROOT}/plugins/pyenv-virtualenv
git clone git@github.com:globusonline/globus-ssh.git  
cd globus-ssh
make develop PYTHON_VERSION=x.y.z
. .${PYENV_ROOT}/venv_x.y.z/bin/activate  
```
PyPi package creation:
```shell
sudo pip install virtualenv  
git clone git@github.com:globusonline/globus-ssh.git  
cd globus-ssh  
make package
