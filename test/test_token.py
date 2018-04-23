import globus_ssh.token as T

class TestToken():
    field = vars(T.Token()).keys()[0]

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

def test_has_scopes():
    scope_name="Hello World"
    t = T.Token(scope=scope_name)
    assert  T.has_scopes(t, scope_name)
    assert (T.has_scopes(t, scope_name+scope_name) == False)
