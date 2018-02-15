
class Token():
    """Token is essentially a dictionary with some token-related helper functions."""

    _keys = ["expires_at_seconds", 
             "access_token",
             "resource_server",
             "token_type",
             "scope",
             "refresh_token",
            ]

    def __init__(self, **kw):
        self._token = dict(dict.fromkeys(Token._keys), **{k:kw[k] for k in kw.keys() if k in Token._keys})

    def __iter__(self):
        return iter(self._token)

    def __getitem__(self, key):
        return self._token[key]

    def has_scopes(self, scopes):
        if self._token['scope'] is None:
		return False;
        for s in scopes.split():
            if not s in self._token['scope'].split():
                return False
        return True
