import globus_ssh.inject_token_once as ITO

def test_trailing_partial_match():
    assert ITO.trailing_partial_match("X",  "X") == 1
    assert ITO.trailing_partial_match("YX", "X") == 1
    assert ITO.trailing_partial_match("XY", "X") == 0
    assert ITO.trailing_partial_match("ABC", "CAT") == 1
    assert ITO.trailing_partial_match("ABCDEF", "DEFXXX") == 3

class TestFilterTrailingMarker():

    def test_marker_found(self):
        ito = ITO.FilterTrailingMarker("marker")
        assert ito.marker_found() == False
        ito("marker")
        assert ito.marker_found() == True
        ito("m")
        assert ito.marker_found() == False
        ito("arker")
        assert ito.marker_found() == True
        ito("m")
        assert ito.marker_found() == False
        ito("barker")
        assert ito.marker_found() == False

    def test_simple_match(self):
        assert ITO.FilterTrailingMarker("marker")("marker") == ""

    def test_no_match(self):
        assert ITO.FilterTrailingMarker("marker")("XXXXX") == "XXXXX"

    def test_split_match(self):
        ito = ITO.FilterTrailingMarker("marker")
        assert ito("mar") == ""
        assert ito("ker") == ""

    def test_split_false_positive(self):
        ito = ITO.FilterTrailingMarker("marker")
        assert ito("mar") == ""
        assert ito("ked") == "marked"

def bytes_is_distinct_type():
    return type(b"X") != type("X")

def is_str_type(var):
    if bytes_is_distinct_type():
        return isinstance(var, str)
    return isinstance(var, str) or isinstance(var, unicode)

def is_bytes_type(var):
    if bytes_is_distinct_type():
        return isinstance(var, bytes)
    return isinstance(var, bytes)

def test_bytes_to_string():
    def dummy_func(self, string):
        assert is_str_type(string)
        return ("Hello", "World")

    (b1, b2) = ITO.bytes_to_string(dummy_func)(None, b'ABC')
    assert is_bytes_type(b1)
    assert is_bytes_type(b2)

class TestInjectTokenOnce():

    def test_process_simple(self):
        ITO.InjectTokenOnce("marker", "token")._process(b"marker") == ("", "token\n")

    def test_process_no_match(self):
        ITO.InjectTokenOnce("marker", "token")._process(b"XXXX") == ("XXXX", "")

    def test_process_partial_match(self):
        ITO.InjectTokenOnce("marker", "token")._process(b"mar") == ("", "")

    def test_process_split_match(self):
        ito = ITO.InjectTokenOnce("marker", "token")
        ito._process(b"mar") == ("", "")
        ito._process(b"ker") == ("", "token\n")

    def test_process_false_positive(self):
        ito = ITO.InjectTokenOnce("marker", "token")
        ito._process(b"mar") == ("", "")
        ito._process(b"ked") == ("marked", "")
