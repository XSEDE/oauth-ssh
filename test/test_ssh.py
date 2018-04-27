from globus_ssh import ssh
import random
import string

def _create_random_string(length):
    return ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(length))


def test_ssh_find_host_name():
    host_name = _create_random_string(6)
    assert ssh.find_host_name([host_name]) == host_name
    assert ssh.find_host_name(['-1', host_name]) == host_name
    assert ssh.find_host_name(['-1', '-2', host_name]) == host_name
    assert ssh.find_host_name(['-1', host_name, '-2']) == host_name
    assert ssh.find_host_name(['-b', 'c', host_name]) == host_name
    assert ssh.find_host_name([]) == None
    assert ssh.find_host_name(['-1']) == None
    assert ssh.find_host_name(['-b', 'c']) == None
    assert ssh.find_host_name(['-1', '-b', 'c']) == None

