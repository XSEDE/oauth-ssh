
#
# Catching this Exception captures all known/planned error conditions
#

class OAuthSSHError(Exception):
    "Base class for all custom exceptions in this module"
    def __init__(self, msg):
        super(OAuthSSHError, self).__init__(msg)
