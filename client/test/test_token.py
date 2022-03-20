from oauth_ssh.token import Token

def test_expires_in():
    assert 'expires_at' in Token(expires_in=10)
