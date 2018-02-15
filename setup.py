from setuptools import setup

setup(
    name="globus_ssh",
    version=0.1,
    packages=['globus_ssh'],
    install_requires=[
        'globus-sdk',
        'psutil',
        'click>',
    ],

    entry_points='''
        [console_scripts]
        globus-ssh=globus_ssh.main:globus_ssh
        globus-scp=globus_ssh.main:globus_scp
    ''',
)
