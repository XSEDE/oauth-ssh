import os


"""
Module for injecting a string 'token' after a string 'marker' is encountered.
"""

class InjectToken():

    def __init__(self, marker, token):
        self._marker = marker
        self._token = token
        self._prompt_found = False
        self._data = ""

    def __call__(self, fd):
        if self._prompt_found == True:
            return os.read(fd, 1024)
        data = self._data + os.read(fd, 1024)
        self._data = ""
        if self._marker in data:
            os.write(fd, self._token + "\n")
            self._prompt_found = True
            data = data.replace(self._marker, "")
            if len(data) == 0:
                return self(fd)
            return data
        data = self._save_split_prompt(data)
        if len(data) == 0:
            return self(fd)
        return data

    def _trailing_prompt_chars(self, data):
        if data.endswith(self._marker):
            return len(self._marker)
        for i in range(1, len(self._marker)):
            if data.endswith(self._marker[:-i]):
                return len(self._marker) - i
        return 0

    def _save_split_prompt(self, data):
        i = self._trailing_prompt_chars(data)
        if i == 0:
            return data
        self._data = data[len(data)-i:]
        return data[0:-i]

