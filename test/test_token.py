import globus_ssh.token as T

class TestToken():
    field = list(vars(T.Token()))[0]

    def test_iter(self):
        # Is it iterable
        assert '__iter__' in T.Token.__dict__
        # Is the iterable on the instance variables
        assert TestToken.field in T.Token()

    def test_getitem(self):
        assert '__getitem__' in T.Token.__dict__
        try:
            T.Token()[TestToken.field]
        except KeyError:
            assert False, "Token does not support Token[field] syntax"
        except:
            assert False
        assert True

    def test_eq_self_str(self):
        key   = 'scope'
        value = "XXX"
        t     = T.Token(**{key:value})
        assert(t == t)

    def test_eq_self_int(self):
        key   = 'expires_at_seconds'
        value = 4
        t     = T.Token(**{key:value})
        assert(t == t)

    def test_neq_token(self):
        key   = 'scope'
        assert(T.Token(**{key:"XXX"}) != T.Token(**{key:"YYY"}))

    def test_neq_none(self):
        key   = 'scope'
        value = 'XXX'
        t     = T.Token(**{key:value})
        assert(t != None)

    def test_neq_basic_type(self):
        key   = 'scope'
        assert(T.Token(**{key:"XXX"}) != 1)

    def test_neq_non_token(self):
        class X():
            pass
        key   = 'scope'
        assert(T.Token(**{key:"XXX"}) != X())

def test_has_scopes():
    scope_name="Hello World"
    t = T.Token(scope=scope_name)
    assert  T.has_scopes(t, scope_name)
    assert (T.has_scopes(t, scope_name+scope_name) == False)
