
#
# Catching this Exception captures all known/planned error conditions
#

class GlobusSSHError(Exception):
    "Base class for all custom exceptions in this module"
    def __init__(self, msg):
        super(GlobusSSHError, self).__init__(msg)
