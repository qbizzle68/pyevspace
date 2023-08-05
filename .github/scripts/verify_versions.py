import sys
import re
import tomllib
import pathlib

from generate_changes import Changelog, Version


def checkLog(path, tag):
    """Verify that the version associated with tag is listed in the change
    log at path."""

    log = Changelog(str(path))
    version = Version(tag)

    return version in log


def checkConfig(path, tag):
    """Verify that the version in the pyproject.toml is the same as the 
    name of the tag ref currently being handled."""

    with open(path, 'rb') as f:
        toml = tomllib.load(f)

    version = toml['project']['version']
    # slice the leading 'v' off the tag ref name
    return version == tag[1:]


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f'syntax: {__file__} <tag_name>')

    TAG_NAME = sys.argv[1]
    # ensure the tag ref name follows semantic version naming
    if not re.match(r'v\d+.\d+.\d$', TAG_NAME):
        print('error: tag_name does not match a version pattern')
        exit(1)

    # project root dir, where the toml and change log are
    root = pathlib.Path(__file__).parents[2]

    validateChangelog = checkLog(root / 'CHANGELOG.md', TAG_NAME)
    validateConfig = checkConfig(root / 'pyproject.toml', TAG_NAME)

    returncode = 0
    if validateChangelog is False:
        print('error: version tag not found in change log')
        returncode = 1
    if validateConfig is False:
        print('error: pyproject.toml version is different than tag')
        returncode = 1

    if returncode == 0:
        print('success: all versions validated')

    exit(returncode)
