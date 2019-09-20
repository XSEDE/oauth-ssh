from oauth_ssh.token import Token

class TestToken():
    def test_expires_in(self):
        assert 'expires_at' in Token(expires_in=10)
