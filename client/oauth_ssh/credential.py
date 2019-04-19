from .template import Template


# [General.Production]
# secret = str
# client = str

class Credential(Template):
    KEYS = {
        'client': str,
        'secret': str,
    }

    def __init__(self, **kw):
        super(Credential, self).__init__(Credential.KEYS, kw)
