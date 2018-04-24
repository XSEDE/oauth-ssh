import os
import ConfigParser
from .token import Token

"""Provides the interface for saving/retrieving tokens from disk.

ConfigParser already provides a generic configuration file functionality.
Our Config class makes the config file look like a coherent set of data 
types known to globus-ssh. Config files are currently indexed by fqdn. 
"""


class Config():
    """Abstract mgmt of persistent data.

    The config file to load is expected to use a basic ini format:
       [Section_Name]
       key1 = value1
       key2 = value2

    Note: Config does not cause a fuss if it can not access the config 
    file but it will complain if it can not write the config file. The
    read behavior will likely change in the future.

    Currently supports only Token persistence but may be extended with
    other data in the future. The Token structure is intentionally treated
    as opaque.
    """

    def __init__(self, file):
        self._file   = file
        self._config = ConfigParser.SafeConfigParser()
        self._config.read(self._file)

    def load_token(self, fqdn):
        """Load section 'fqdn', if present, and convert to Token"""
        if not self._config.has_section(fqdn):
            return None;
        return Token(**dict(self._config.items(fqdn)))

    def save_token(self, fqdn, token):
        """Save section 'fqdn' using attributes of Token"""
        if not self._config.has_section(fqdn):
            self._config.add_section(fqdn)
        for k in token:
           if token[k] is None:
               continue
           self._config.set(fqdn, k, token[k])
        self._flush()

    def delete_token(self, fqdn):
        """Delete section 'fqdn'"""
        if self._config.has_section(fqdn):
            for k in Token():
                self._config.remove_option(fqdn, k)
        self._flush()

    def _flush(self):
        mask = os.umask(0o177)

        try:
            with open(self._file, 'wb') as configfile:
                self._config.write(configfile)
            os.chmod(self._file, 0o600)

        except Exception as e:
            os.umask(mask)
            raise e
        os.umask(mask)
