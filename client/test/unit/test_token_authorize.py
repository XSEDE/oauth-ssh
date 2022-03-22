from uuid import UUID

from unittest import mock
import pytest
from click.testing import CliRunner

from oauth_ssh.oauth_ssh_token import token_authorize
from oauth_ssh.ssh_service import SSHService


@pytest.fixture(autouse=True)
def mock_revoke_token():
    with mock.patch('oauth_ssh.oauth_ssh_token.revoke_token') as m:
        yield m


@pytest.fixture(autouse=True)
def mock_config():
    with mock.patch('oauth_ssh.oauth_ssh_token.Config', autospec=True) as m:
        yield m


@pytest.fixture(autouse=True)
def mock_ssh_service__init__():
    with mock.patch.object(
        SSHService,
        '__init__',
        autospec=True,
        return_value=None
    ) as m:
        yield m

DEFAULT_SECURITY_POLICY = {
    'permitted_idps': [],
    'authentication_timeout': 0,
}

@pytest.fixture(autouse=True)
def mock_ssh_service_get_security_policy():
    with mock.patch.object(
        SSHService,
        'get_security_policy',
        autospec=True,
        return_value=DEFAULT_SECURITY_POLICY,
    ) as m:
        yield m


DEFAULT_TOKEN = {'foo': 'bar'}
@pytest.fixture(autouse=True)
def mock_auth_do_auth_code_grant():
    with mock.patch(
        'oauth_ssh.oauth_ssh_token.Auth.do_auth_code_grant',
        autospec=True,
        return_value=DEFAULT_TOKEN
    ) as m:
        yield m


DEFAULT_IDENTITY = {'identity': 'foo'}
@pytest.fixture(autouse=True)
def mock_auth_lookup_identity():
    with mock.patch(
        'oauth_ssh.oauth_ssh_token.Auth.lookup_identity',
        autospec=True,
        return_value=DEFAULT_IDENTITY
    ) as m:
        yield m


def test_canary(
    mock_ssh_service__init__,
    mock_auth_do_auth_code_grant,
    mock_config,
):
    """
    Verify that the simplest case succeeds given our assumptions.

    The tests in this file make certain assumptions. This test case verifies
    that those assumptions have not changed. Assumptions are listed inline.
    """
    fqdn = 'foo.example.com'

    runner = CliRunner()
    # Assumption: Only one requirement argument
    result = runner.invoke(token_authorize, [fqdn])
    # Assumption: SSHService configured to use fqdn
    assert mock_ssh_service__init__.call_args[0][1] == fqdn
    # Assumption: SSHService configured to use port 22 (default)
    assert mock_ssh_service__init__.call_args[0][2] == 22
    # Assumption: Auth code grant flow is based on service's FQDN
    assert mock_auth_do_auth_code_grant.call_args[0][0] == fqdn
    # Assumption: No force_login when authentication_timeout == 0
    assert mock_auth_do_auth_code_grant.call_args[0][1] == False
    # Assumption: No identity passed to Auth.do_auth_code_grant without --identity
    mock_auth_do_auth_code_grant.call_args[0][2] == DEFAULT_IDENTITY
    # Assumption: On success, the config selection if deleted
    assert mock_config.delete_section.call_args[0][0] == fqdn
    # Assumption: On success, the policy is saved in section fqdn
    assert mock_config.save_object.call_args_list[0][0][0] == fqdn
    assert mock_config.save_object.call_args_list[0][0][1] == DEFAULT_SECURITY_POLICY
    # Assumption: On success, the token is saved in section fqdn
    assert mock_config.save_object.call_args_list[1][0][0] == fqdn
    assert mock_config.save_object.call_args_list[1][0][1] == DEFAULT_TOKEN
    # Assumption: exit with zero on success
    assert result.exit_code == 0


def test_non_default_port(
    mock_ssh_service__init__,
):
    ...
    port = 2222

    runner = CliRunner()
    result = runner.invoke(token_authorize, ['example.com', '--port', port])
    # Assumption: SSHService configured to use our port
    assert mock_ssh_service__init__.call_args[0][2] == port
    # Assumption: exit with zero on success
    assert result.exit_code == 0


def test_identity_option(
    mock_auth_lookup_identity,
    mock_auth_do_auth_code_grant,
):
    identity = 'foo'
    runner = CliRunner()
    result = runner.invoke(token_authorize, ['example.com', '--identity', identity])
    # Assumption: Our identity is passed to Auth.lookup_identity()
    mock_auth_lookup_identity.call_args[0][0] == identity
    # Assumption: DEFAULT_IDENTITY is passsed to Auth.do_auth_code_grant()
    mock_auth_do_auth_code_grant.call_args[0][2] == DEFAULT_IDENTITY
    # Assumption: exit with zero on success
    assert result.exit_code == 0

def test_single_permitted_idp(
    mock_ssh_service_get_security_policy,
):
    """
    A single permitted_idp does not have any impact on the unit.
    """
    security_policy = DEFAULT_SECURITY_POLICY.copy()
    security_policy['permitted_idps'] = ['globus.org']
    mock_ssh_service_get_security_policy.return_value = security_policy

    runner = CliRunner()
    result = runner.invoke(token_authorize, ['example.com'])
    # Assumption: exit with zero on success
    assert result.exit_code == 0


def test_multiple_permitted_idps(
    mock_ssh_service_get_security_policy,
):
    """
    A multiple permitted_idps requires use of the --identity option.
    """
    security_policy = DEFAULT_SECURITY_POLICY.copy()
    security_policy['permitted_idps'] = ['globus.org', 'foo.com']
    mock_ssh_service_get_security_policy.return_value = security_policy

    runner = CliRunner()
    result = runner.invoke(token_authorize, ['example.com'])
    # Assumption: exit with non zero
    assert result.exit_code != 0
    # Assumption: output mentions the --identity option
    assert '--identity' in result.output

    result = runner.invoke(token_authorize, ['example.com', '--identity', 'foo'])
    # Assumption: exit with non zero
    assert result.exit_code == 0


def test_non_zero_authentication_timeout(
    mock_ssh_service_get_security_policy,
    mock_auth_do_auth_code_grant,
):
    """
    Non zero authentication_timeout in the policy sets force_login to True.
    """
    security_policy = DEFAULT_SECURITY_POLICY.copy()
    security_policy['authentication_timeout'] = 300
    mock_ssh_service_get_security_policy.return_value = security_policy

    runner = CliRunner()
    result = runner.invoke(token_authorize, ['example.com'])
    # Assumption: force_login is True when authentication_timeout > 0
    assert mock_auth_do_auth_code_grant.call_args[0][1] == True
    # Assumption: exit with non zero
    assert result.exit_code == 0


def test_client_id_option(
    mock_auth_do_auth_code_grant,
):
    """
    When --client_id is used, it is used as the basis of the auth flow
    instead of fqdn.
    """
    client_id = '892577a9-a465-4cfc-a652-b879fedfc2fe'
    runner = CliRunner()
    result = runner.invoke(token_authorize, ['example.com', '--client-id', client_id])

    # Assumption: Auth code grant flow is based on --client-id
    assert mock_auth_do_auth_code_grant.call_args[0][0] == UUID(client_id)
    # Assumption: exit with non zero
    assert result.exit_code == 0


def test_client_id_option_invalid_type(
    mock_auth_do_auth_code_grant,
):
    """
    --client-id must be a UUID
    """
    client_id = 'foo_bar'
    runner = CliRunner()
    result = runner.invoke(token_authorize, ['example.com', '--client-id', client_id])

    # Assumption: exit with non zero
    assert result.exit_code != 0
    assert "not a valid UUID value" in result.output