import sys
from setuptools import setup

print sys.version_info
if sys.version_info >= (2, 6) and sys.version_info < (3):
    raise NotImplementedError("""\n
####################################################
# globus-ssh currently supports python version 2.7 #
####################################################""")

setup(
    name='globus_ssh',
    version=0.2,
    description='SSH with Globus Auth',
    long_description=open("README.rst").read(),
    url='https://github.com/globus/globus-ssh',
    author="Globus Team",
    author_email='support@globus.org',
    packages=['globus_ssh'],

    # Only enforced with setuptools 24.2.0+ and pip 9.0.0+
    python_requires='>=2.6, <3',

    install_requires=[
        'globus-sdk>=1.5.0',
        'psutil>=5.4.3',
        'click>=6.7,<7.0',
    ],

    entry_points='''
        [console_scripts]
        globus-ssh=globus_ssh.main:globus_ssh
        globus-scp=globus_ssh.main:globus_scp
    ''',

    keywords=["globus", "ssh", "scp"],
    classifiers=[
        "Development Status :: 1 - Planning",
        "Intended Audience :: End Users/Desktop",
        "Operating System :: POSIX",
        "Natural Language :: English",
        "Programming Language :: Python :: 2.7",
    ],
)
