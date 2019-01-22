from .template import Template

class Policy(Template):
    KEYS = {
        'permitted_idps': list,
        'authentication_timeout': int
    }

    def __init__(self, **kw):
        super(Policy, self).__init__(Policy.KEYS, kw)
