import configparser
from os.path import exists
from setuptools import setup


if __name__ == "__main__":
    # If the file 'clean' does not exist, then append '.dev0' to the package's
    # version.
    setup_args = {}
    if not exists('clean'):
        config = configparser.ConfigParser()
        config.read('setup.cfg')
        setup_args['version'] = config['metadata']['version'] + '.dev0'
    setup(**setup_args)
