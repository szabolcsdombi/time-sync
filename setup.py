from setuptools import Extension, setup

ext = Extension(
    name='time_sync',
    sources=['./time_sync.cpp'],
)

setup(
    name='time-sync',
    version='0.2.0',
    ext_modules=[ext],
)
