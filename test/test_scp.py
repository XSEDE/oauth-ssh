from globus_ssh import scp
import random
import string

def _create_random_string(length):
    return ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(length))


def test_scp_find_host_name():
    host_name = _create_random_string(6)
    assert scp.find_host_name([host_name+':F']) == host_name
    assert scp.find_host_name(['-1', host_name+':F']) == host_name
    assert scp.find_host_name(['-1', '-2', host_name+':F']) == host_name
    assert scp.find_host_name(['-1', host_name+':F', '-2']) == host_name
    assert scp.find_host_name(['-b', 'c', host_name+':F']) == host_name
    assert scp.find_host_name(['user@' + host_name+':F']) == host_name

    assert scp.find_host_name([]) == None
    assert scp.find_host_name(['-1']) == None
    assert scp.find_host_name(['-b', 'c']) == None
    assert scp.find_host_name(['-1', '-b', 'c']) == None

