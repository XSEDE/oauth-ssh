import time

from .template import Template

class Token(Template):
    KEYS = {
        'scope': str,
        'token_type': str,
        'access_token': str,
        'refresh_token': str,
        'authorized_at': int,
        'resource_server': str,
        'expires_at': int,
    }

    def __init__(self, **kw):
        if 'expires_in' in kw:
            kw = kw.copy()
            kw['expires_at'] = int(kw['expires_in']) + int(time.time())
        super(Token, self).__init__(Token.KEYS, kw)

    def update(self, values):
        if 'expires_in' in values:
            values = values.copy()
            values['expires_at'] = int(values['expires_in']) + int(time.time())
        super(Token, self).update(values)
