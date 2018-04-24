
"""Wrap Globus Auth token information and provide basic utilities."""

class Token():
    """Provide a container for information related to the Globus Auth token.

    Token implements the following contractual agreement(s):
    - it can be parsed, accessed and filled as an opaque structure
    - it lays no claims towards the validity of the held information

    Rationale:
    - An external use (ex Config) should be able to store, retrieve and
      initialize this structure without being tightly coupled to how a
      Token is implemented or which fields determine a valid token.
    - Error handling for missing or invalid field values is context
      sensative and should be handled at a higher logic level
    """

    def __init__(self, **kw):
        self.scope              = kw.get('scope',              None)
        self.token_type         = kw.get('token_type',         None)
        self.access_token       = kw.get('access_token',       None)
        self.refresh_token      = kw.get('refresh_token',      None)
        self.resource_server    = kw.get('resource_server',    None)
        self.expires_at_seconds = kw.get('expires_at_seconds', None)

    def __iter__(self):
        """Return an iter of our variables to support opaque handlers"""
        return iter(vars(self))

    def __getitem__(self, key):
        """Return the value of local var 'key' to support opaque handlers"""
        return vars(self)[key]

    def __eq__(self, other): 
        """Determine if two Tokens are identical"""
        if other is None:
            return False
        try:
            for k in self:
                if self[k] != other[k]:
                    return False
        except:
            return False
        return True

def has_scopes(token, scopes):
        if token.scope is None:
		return False;
        for s in scopes.split():
            if not s in token.scope.split():
                return False
        return True

