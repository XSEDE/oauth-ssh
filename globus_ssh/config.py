import os
import ConfigParser
from .token import Token

"""Provides the interface for saving/retrieving tokens from disk.

ConfigParser already provides a generic configuration file functionality.
Our Config class makes the config file look like a coherent set of data 
types known to globus-ssh. Config files are currently indexed by fqdn. 
"""


class Config():
    """Abstract mgmt of persistent data."""

    def __init__(self, file):
        self._file   = file
        self._config = ConfigParser.SafeConfigParser()
        self._config.read(self._file)

    def save_token(self, fqdn, token):
        if not self._config.has_section(fqdn):
            self._config.add_section(fqdn)
        for k in token:
            self._config.set(fqdn, k,  unicode(token[k]))
        self._flush()

    def delete_token(self, fqdn):
        if self._config.has_section(fqdn):
            for k in Token._keys:
                self._config.remove_option(fqdn, k)
        self._flush()

    def load_token(self, fqdn):
        if not self._config.has_section(fqdn):
            return None;
        return Token(**dict(self._config.items(fqdn)))

    def _flush(self):
        mask = os.umask(0o177)

        try:
            with open(self._file, 'wb') as configfile:
                self._config.write(configfile)
            os.chmod(self._file, 0o600)

        except Exception as e:
            os.umask(mask)
            raise e
