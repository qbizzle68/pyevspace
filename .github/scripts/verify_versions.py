import sys
import re
import tomllib
import pathlib

from generate_changes import Changelog, Version


def checkLog(path, tag):
    log = Changelog(str(path))
    version = Version(tag)

    return version in log


def checkConfig(path, tag):
    with open(path, 'rb') as f:
        toml = tomllib.load(f)

    version = toml['project']['version']
    return version == tag[1:]


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f'syntax: {__file__} <tag_name>')

    TAG_NAME = sys.argv[1]
    if not re.match(r'v\d+.\d+.\d$', TAG_NAME):
        print('error: tag_name does not match a version pattern')
        exit(1)

    root = pathlib.Path(__file__).parents[2]

    validateChangelog = checkLog(root / 'CHANGELOG.md', TAG_NAME)
    validateConfig = checkConfig(root / 'pyproject.toml', TAG_NAME)

    if validateChangelog is False:
        print('error: version tag not found in change log')
        exit(1)
    if validateConfig is False:
        print('error: pyproject.toml version is different than tag')
        exit(1)

    print('success: all versions validated')
    exit(0)
