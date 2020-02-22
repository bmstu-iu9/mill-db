from setuptools import setup, find_packages
import os

setup(
    name='pymilldb',
    version='0.1',
    package_dat={
        'pymilldb': [
            'pymilldb/logging.yaml',
            'pymilldb/template/*.c',
            'pymilldb/template/*.h'
        ]
    },
    package=[
        'pymilldb',
        'pymilldb.context'
    ],
    install_requires=[
        'pyyaml>=5.1.2',
        'jinja2>=2.10.3',
    ]
)
