# Globus SSH Server Side Components

globusonline/ssh provides a PAM module that introspects access tokens passed as passwords during challenge/response authentication. It has a companion client component located at globus/globus-ssh.

### User Documentation

The official, user-facing documents for installation and operation of SSH with Globus Auth is located on [the official Globus docs website](https://docs.globus.org). (_During beta, the user-facing documents are being_staged to the docs preview location. The current location can be found in [docs.globus.org PR #320](https://github.com/globusonline/docs.globus.org/pull/320)_).

### Developer Overview

**Compiling**

The latest dependencies and build targets can be found in `build_rpm` found in the top level of the repository.

**Testing**

Globus SSH relies on the [cmocka](https://cmocka.org/) testing library. To install cmocka:

>sudo yum install -y wget cmake gcc  
>wget https://cmocka.org/files/1.1/cmocka-1.1.1.tar.xz  
>unxz cmocka-1.1.1.tar.xz  
>tar xf cmocka-1.1.1.tar  
>cd cmocka-1.1.1; mkdir build; cd build  
>cmake ..; make; sudo make install  

Then reconfigure the SSH repository and point it to the cmake installation:

> PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ./configure

**Code Submissions**
1. Submit an issue with the [Globus SSH Repo](https://github.com/globusonline/ssh/issues).
2. Create a fork of the official Globus SSH repository for all work.
3. Use the branch naming scheme `issue/<issue_id>` for all work related to the issue.
4. Submit source updates to your fork.
5. Squash all commits into a single commit with log message `Fixes #<issue_id>`. Add additoinal information as applicable but try to keep details for the change within the issue.
6. Prior to submitting the pull request:
* Rebase your issue branch on top of the official repository's master branch.
* Run build_rpm in the top level to make sure it still functions.
* Run `make test` to verify that all tests still pass.
  * Add new tests for new features as necessary.
7. Create a pull request. Make sure the subject includes "Closes #<issue_id>" in the description.
8. Submit the pull request.
9. When approving the pull request, use `rebase and commit`. Reject any pull requests that can't merge cleanly.

