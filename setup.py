from setuptools import setup, find_packages

setup(
    name='pymilldb',
    version='0.1',
    package_data={
        '': [
            'logging.yaml',
            'template/*.c',
            'template/*.h'
        ]
    },
    packages=find_packages(),
    install_requires=[
        'pyyaml>=5.1.2',
        'jinja2>=2.10.3',
    ]
)
