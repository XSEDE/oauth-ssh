# OAuth Authorization for SSH

OAuth SSH delivers use of federated identities to the SSH
platform providing a uniform security infrastructure for institutions.

The solution supports both programmatic and interactive use, eliminating
the need for management of SSH keys and is a replacement for the
deprecated GSI SSH product.

OAuth SSH is distributed as client and server packages, both referred to as
`oauth-ssh`. The client is a Python package that manages OAuth tokens on
behalf of the user. The server package provides a pam security module loadable
into the SSH authentication process for authentication via token introspect and
account mapping.

Bugs reports and feature requests are open submission, and should be filed at
https://github.com/xsede/oauth-ssh/issues.

The PyPi project can be found at: https://pypi.org/project/oauth-ssh/.

The server-side components are located in [the server subdirectory](server/).
The client-side components are located in [the client subdirectory](client/).

Currently this code works against the Globus Auth authorization service.
