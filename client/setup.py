import sys
from setuptools import setup

setup(
    name='oauth_ssh',
    version='0.13',
    description='SSH with OAuth Tokens',
    long_description=open("README.rst").read(),
    url='https://github.com/xsede/oauth-ssh',
    author="Jason Alt",
    author_email='jasonalt@gmail.com',
    packages=['oauth_ssh'],

    # Only enforced with setuptools 24.2.0+ and pip 9.0.0+
    python_requires='>=3.5',

    install_requires=[
        # >= 2.4.0 doesn't accept the json keyword in put requests
        'requests>=2.5.0,<3',
        'click>=7.0,<8',
        # paramiko >= 2.4 creates CryptographyDeprecationWarning
        'paramiko>=2.5.0,<3',
    ],

    entry_points='''
        [console_scripts]
        oauth-ssh=oauth_ssh.oauth_ssh:oauth_ssh
        oauth-ssh-token=oauth_ssh.oauth_ssh_token:oauth_ssh_token
    ''',

    keywords=["oauth", "ssh"],
    classifiers=[
        "Development Status :: 1 - Planning",
        "Intended Audience :: End Users/Desktop",
        "Operating System :: POSIX",
        "Natural Language :: English",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
    ],
)
