from globus_ssh.inject_token import InjectToken

class TestInjectToken():

   def test_trailing_prompt_chars(self):
       assert InjectToken("XX", "")._trailing_prompt_chars("") == 0
       assert InjectToken("XX", "")._trailing_prompt_chars("X") == 1
       assert InjectToken("XX", "")._trailing_prompt_chars("XX") == 2
       assert InjectToken("XX", "")._trailing_prompt_chars("AAXX") == 2

