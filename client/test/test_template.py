from oauth_ssh.template import Template

def raises_error(F, *args, **kw):
    try:
        F(*args, **kw)
    except:
        return True
    return False

def illegal_insert(inst, key, value):
    try:
        inst()[key] = value
    except:
        return True
    return False

class TestTemplate():
    def test_creation_no_template(self):
        assert len(Template({}, {'A': 1}).keys()) == 0

    def test_creation_no_values(self):
        assert Template({'A': int},  {})['A'] is None
        assert Template({'A': list}, {})['A'] is None

    def test_initial_valid_values(self):
        assert Template({'A': int},  {'A': 15})['A'] == 15
        assert Template({'A': str},  {'A': '15'})['A'] == '15'
        assert Template({'A': list}, {'A': ['15']})['A'] == ['15']

    def test_initial_valid_casts(self):
        assert Template({'A': int},  {'A':    '15'})  ['A'] ==   15
        assert Template({'A': list}, {'A':  "['15']"})['A'] == ['15']

    def test_initial_invalid_casts(self):
        assert raises_error(Template, {'A': list},  {'A': '[15]'})
        assert raises_error(Template, {'A': list},  {'A':  '15'})
        assert raises_error(Template, {'A': list},  {'A':   15})

        assert raises_error(Template, {'A': int},  {'A': '[15]'})
        assert raises_error(Template, {'A': int},  {'A':  'B'})

        assert raises_error(Template, {'A': str},  {'A': [15]})
        assert raises_error(Template, {'A': str},  {'A':  15})

    def test_initial_invalid_key(self):
        assert len(Template({}, {'B': 1})) == 0

    def test_insertion(self):
        template = Template({'A': int, 'B': str, 'C': list})
        template['A'] =   15
        template['B'] =  '15'
        template['C'] = ['15']
        assert template == {'A': 15, 'B': '15', 'C': ['15'] }

    def test_insertion_with_casts(self):
        template = Template({'I': int, 'L': list})
        template['I'] =   '15'
        template['L'] = "['15']"
        assert template == {'I': 15, 'L': ['15'] }


    def test_insertion_with_invalid_casts(self):
        assert illegal_insert(Template({'A': list}), 'A', '[15]')
        assert illegal_insert(Template({'A': list}), 'A',  '15')
        assert illegal_insert(Template({'A': list}), 'A',   15)

        assert illegal_insert(Template({'A': int}),  'A', '[15]')
        assert illegal_insert(Template({'A': int}),  'A',  'B')

        assert illegal_insert(Template({'A': str}),  'A', '[15]')
        assert illegal_insert(Template({'A': str}),  'A',   15)

    def test_insertion_invalid_key(self):
        assert illegal_insert(Template({'A': list}), 'B', [15])

    def test_deletion_int(self):
        template = Template({'A': int},  {'A': 15})
        del template['A']
        assert template['A'] is None

    def test_deletion_list(self):
        template = Template({'A': list},  {'A': ['1']})
        del template['A']
        assert template['A'] is None

