import os


"""
Module for injecting a string 'token' after a string 'marker' is encountered.
"""

def trailing_partial_match(haystack, needle):
    """Return length of trailing partial match of needle in haystack"""
    for i in range(len(needle), 0, -1):
        if haystack.endswith(needle[0:i]):
            return i
    return 0

class FilterTrailingMarker():

    def __init__(self, marker):
        self._marker = marker
        self._previous_match = ""
        self._marker_found = False

    def __call__(self, string):
        self._marker_found = False

        string = self._previous_match + string
        self._previous_match = ""

        matching_length = trailing_partial_match(string, self._marker)
        if matching_length == 0:
            return string

        if matching_length < len(self._marker):
            self._previous_match = self._marker[0:matching_length]

        if matching_length == len(self._marker):
            self._marker_found = True

        return string[:-matching_length]

    def marker_found(self):
        return self._marker_found

def bytes_to_string(function):

    def wrapper(self, byte_sequence):
        (input, output) = function(self, byte_sequence.decode('utf8'))
        return (input.encode('utf8'), output.encode('utf8'))

    return wrapper

class InjectTokenOnce():

    def __init__(self, marker, token):
        self._token = token
        self._token_injected = False
        self._filter_marker  = FilterTrailingMarker(marker)

    def __call__(self, fd):
        """Write 'token' and filter out 'marker' when 'marker' is found"""
        (bytes_in, bytes_out) = self._process(os.read(fd, 1024))
        if len(bytes_out) > 0:
            os.write(fd, bytes_out)
        if len(bytes_in) > 0:
            return bytes_in
        return self(fd)

    @bytes_to_string
    def _process(self, string_in):
        """Write 'token' and filter out 'marker' when 'marker' is found

        Provided strictly for testing.
        """
        string_out = ""

        if self._token_injected == False:
            string_in = self._filter_marker(string_in)
            if self._filter_marker.marker_found() == True:
                string_out = self._token
                self._token_injected = True

        return (string_in, string_out)
