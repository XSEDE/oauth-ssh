import ast
import sys

if sys.version_info.major==3:unicode=str

class Template(dict):
    def __init__(self, template={}, values={}):
        self._template = template
        super(Template, self).__init__()
        for k in template.keys():
            super(Template, self).__setitem__(k, None)
            # Let setitem check types
            self[k] = values[k] if k in values else None

    def __setitem__(self, key, value):
        if key not in self._template:
            raise KeyError(self.__class__.__name__ +": No such key: "+str(key))

        expected_type = self._template[key]
        bad_value = False
        cast_value = value

        # Cast required
        if value is not None and type(value) is not expected_type:
            # Check for explicit bad casts
            if expected_type is int:
                if type(value) is not int and type(value) is not str:
                    bad_value = True
            elif expected_type is str:
                if type(value) is not str and type(value) is not unicode:
                    bad_value = True
            elif expected_type is list:
                if type(value) is not str:
                    bad_value = True

            # Check for implicit bad casts
            if not bad_value:
                try:
                    if expected_type is list:
                        cast_value = ast.literal_eval(value)
                    else:
                        cast_value = expected_type(value)
                except:
                    bad_value = True # Bad cast

        # Last minute list validation
        if bad_value is False and value is not None and expected_type is list:
            for i in cast_value:
                if type(i) is not str and type(i) is not unicode:
                    bad_value = True
            # Translate unicode strings
            if bad_value is True:
                cast_value = list(map(lambda x: str(x), cast_value))

        if bad_value is True:
            raise TypeError(self.__class__.__name__
                            + ": Bad value for "
                            + key
                            + ": "
                            + str(value))

        super(Template, self).__setitem__(key, cast_value)

    def update(self, values):
        for k,v in values.items():
            if k in self._template:
                # Let setitem check types
                self[k] = values[k]

    def __delitem__(self, k):
        if k not in self:
            raise KeyError(self.__class__.__name__ + ": No such key: " + str(k))
        super(Template, self).__setitem__(k, None)
