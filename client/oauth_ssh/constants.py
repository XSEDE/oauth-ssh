import os

PREFILL_NAMED_GRANT = "OAuth SSH Client"
USE_REFRESH_TOKENS  = True
SCOPE_FORMAT        = "https://auth.globus.org/scopes/{fqdn}/ssh"
CONFIG_FILE         = os.path.expanduser('~/.oauth-ssh.cfg')
PROMPT              = "Enter your OAuth token: "
GLOBUS_ACCOUNT      = 'oauth-ssh'
PROTOCOL_VERSION    = 1
DEFAULT_SECTION     = "General"

SETTINGS = {
    'production': (
                      '892ee39b-545a-4505-965a-cff0c96f4e74',
                      'auth.globus.org'
                  ),
    'sandbox':    (
                      'b15c619c-e40e-461a-9a43-0fa21e0f3d35',
                      'auth.sandbox.globuscs.info'
                  )
}

ENVIRONMENT=os.environ.get('OAUTH_SSH_ENVIRONMENT')
if ENVIRONMENT not in SETTINGS:
    ENVIRONMENT='production'

TEMPLATE_ID, GLOBUS_AUTH_HOST = SETTINGS[ENVIRONMENT]
