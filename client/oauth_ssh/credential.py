from .template import Template


class Credential(Template):
    KEYS = {
        'client': str,
        'secret': str,
    }

    def __init__(self, **kw):
        super(Credential, self).__init__(Credential.KEYS, kw)
