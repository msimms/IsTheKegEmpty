from setuptools import setup, find_packages

requirements = ['configparser', 'mako', 'bson', 'pymongo', 'bcrypt', 'flask', 'requests', 'unidecode']

setup(
    name='is_the_keg_empty',
    version='0.0.1',
    description='',
    url='https://github.com/msimms/IsTheKegEmpty',
    author='Mike Simms',
    packages=find_packages(exclude=['contrib', 'docs', 'tests']),
    install_requires=requirements,
)
