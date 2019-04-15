from setuptools import setup

setup(
    name='globus_dns',
    version='1.0',
    py_modules=['globus_dns'],
    install_requires=[
        'Click==6.7',
        'requests',
    ],
    entry_points='''
        [console_scripts]
        globus-dns=globus_dns:entry_point
    ''',
)

