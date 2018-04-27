import os

PREFILL_NAMED_GRANT = "SSH Client with Globus Auth (Demo)"
USE_REFRESH_TOKENS  = True
SCOPES_FORMAT       = "https://auth.globus.org/scopes/{fqdn}/ssh"
CONFIG_FILE         = '~/.globus-ssh.cfg'
PROMPT              = "Enter your Globus Auth token:"

CLIENT_ID = '892ee39b-545a-4505-965a-cff0c96f4e74' # On Prod
if os.environ.get('GLOBUS_SDK_ENVIRONMENT') == "sandbox":
    CLIENT_ID = 'b15c619c-e40e-461a-9a43-0fa21e0f3d35' # On Sandbox

