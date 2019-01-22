from .template import Template


class AccountMap(Template):
    KEYS = {
        'permitted_accounts': list
    }

    def __init__(self, **kw):
        super(AccountMap, self).__init__(AccountMap.KEYS, kw)
