import pytest

from oauth_ssh.template import Template

#
# Initialization Tests
#
def test_initialize_no_template():
    assert len(Template({}, {'A': 1}).keys()) == 0

def test_initialize_no_values():
    assert Template({'A': int},  {})['A'] is None
    assert Template({'A': list}, {})['A'] is None

def test_initialize_valid_values():
    assert Template({'A': int},  {'A': 15})['A'] == 15
    assert Template({'A': str},  {'A': '15'})['A'] == '15'
    assert Template({'A': list}, {'A': ['15']})['A'] == ['15']

def test_initialize_valid_casts():
    assert Template({'A': int},  {'A':    '15'})  ['A'] ==   15
    assert Template({'A': list}, {'A':  "['15']"})['A'] == ['15']

@pytest.mark.parametrize(
    "template,values",
    [
        ({'A': list}, {'A': '[15]'}),
        ({'A': list}, {'A': '15'}),
        ({'A': list}, {'A': 15}),
        ({'A': int},  {'A': '[15]'}),
        ({'A': int},  {'A': 'B'}),
        ({'A': str},  {'A': [15]}),
        ({'A': str},  {'A': 15}),
    ],
)
def test_initialize_invalid_casts(template, values):
    with pytest.raises(TypeError):
        Template(template, values)

def test_initialize_invalid_key():
    assert len(Template({}, {'B': 1})) == 0

#
# Insertion Tests
#

def test_insertion():
    template = Template({'A': int, 'B': str, 'C': list})
    template['A'] =   15
    template['B'] =  '15'
    template['C'] = ['15']
    assert template == {'A': 15, 'B': '15', 'C': ['15'] }

def test_insertion_with_casts():
    template = Template({'I': int, 'L': list})
    template['I'] =   '15'
    template['L'] = "['15']"
    assert template == {'I': 15, 'L': ['15'] }

@pytest.mark.parametrize(
    'template,key,value',
    [
        ({'A': list}, 'A', '[15]'),
        ({'A': list}, 'A', '15'),
        ({'A': list}, 'A', 15),
        ({'A': int},  'A', '[15]'),
        ({'A': int},  'A', 'B'),
        ({'A': str},  'A', [15]),
        ({'A': str},  'A', 15),
    ]
)
def test_insertion_with_invalid_casts(template, key, value):
    with pytest.raises(TypeError):
        Template(template)[key] = value

def test_insertion_invalid_key():
    with pytest.raises(KeyError):
        Template({'A': list})['B'] = [15]

#
# Deletion Tests
#

def test_deletion_int():
    template = Template({'A': int},  {'A': 15})
    del template['A']
    assert template['A'] is None

def test_deletion_list():
    template = Template({'A': list},  {'A': ['1']})
    del template['A']
    assert template['A'] is None
