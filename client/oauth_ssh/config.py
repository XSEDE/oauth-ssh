import stat
import ast
import os

from .constants import *
from .exceptions import OAuthSSHError

try:
    import ConfigParser as configparser
except ImportError:
    import configparser

class ConfigError(OAuthSSHError):
    "Base exception for all Config exceptions"""

def _check_permissions(path):
    if os.path.exists(path):
        if not os.path.isfile(path):
            raise ConfigError(path + ' is not a regular file')
        if not os.access(path, os.R_OK|os.W_OK):
            raise ConfigError(path +  ' has bad permissions, should be 0600')
        # Don't allow Group/Other permissions
        st = os.stat(path)
        if st.st_mode & (stat.S_IRWXG|stat.S_IRWXO):
            raise ConfigError(path +  ' is too permissive, should be 0600')
    else:
        dir = os.path.dirname(path)
        if not os.path.isdir(dir):
            raise ConfigError(path 
                              + ' is not a valid path: '
                              + ' parent is not a directory')

        if not os.access(dir, os.X_OK|os.W_OK):
            raise ConfigError('Can not create the config file in '
                              + dir
                              + 'parent directory permissions are too '
                              + 'restrictive')

def _load_file(path):
    _check_permissions(path)
    config = configparser.SafeConfigParser()
    config.optionxform = str # case-sensitive keys
    try:
        config.read(path)
    except configparser.Error as e:
        raise ConfigError('Error parsing ' + path + ': ' + e.message)
    return config

def _save_file(path, config):
    _check_permissions(path)
    try:
        mask = os.umask(0o077)
        fd = os.open(path, os.O_WRONLY|os.O_CREAT|os.O_TRUNC, 0o600)
    except OSError as e:
        raise ConfigError('Could not open ' + path + ': ' + e.strerror)
    finally:
        mask = os.umask(0o077)
    with os.fdopen(fd, 'w') as f:
        config.write(f)

def load_section(section):
    config = _load_file(CONFIG_FILE)
    if not config.has_section(section):
        return {}
    return dict(config.items(section))

def save_section(section, values):
    config = _load_file(CONFIG_FILE)
    if config.has_section(section):
    	config.remove_section(section)
    config.add_section(section)
    for k,v in values.items():
        config.set(section, k, str(v))
    _save_file(CONFIG_FILE, config)

def delete_section(section):
    config = _load_file(CONFIG_FILE)
    if not config.has_section(section):
        return
    config.remove_section(section)
    _save_file(CONFIG_FILE, config)

def load_object(section, cls):
    values = load_section(section)
    if cls.__name__ in values:
        return cls(**ast.literal_eval(values[cls.__name__]))
    return None

def save_object(section, inst):
    values = load_section(section)
    values[inst.__class__.__name__] = inst
    save_section(section, values)

def delete_object(section, cls):
    values = load_section(section)
    if cls.__name__ in values:
        del values[cls.__name__]
    save_section(section, values)
