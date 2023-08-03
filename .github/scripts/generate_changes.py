import os
import re
import sys

from functools import total_ordering


# This parsing is based on rigorous formatting in the CHANGELOG.md. Each beginning of a
# version's changed should start with a line consiting only of: '## [X.X.X] - YYYY-MM-DD'.
# These will serve as the demarcation points with which to generate the changes.

LINE_DELIMITER = re.compile('^## \[\d+.\d+.\d+\] - \d{4}-\d{2}-\d{2}\s*$')
VERSION_PATTERN = re.compile('\[\d+.\d+.\d+\]')


@total_ordering
class Version:
    __slots__ = '_major', '_minor', '_patch'

    def __init__(self, versionString: str):
        major, minor, patch = self._parseVersion(versionString)
        self._major = int(major)
        self._minor = int(minor)
        self._patch = int(patch)

    @staticmethod
    def _parseVersion(version: str):
        if not isinstance(version, str):
            raise TypeError(f'version must be a str type, not {type(version)}')
        if version[0] != 'v':
            raise ValueError(f'version string must start with a \'v\', not {version[0]}')

        versionNumbers = version[1:].split('.')
        if len(versionNumbers) != 3:
            raise ValueError(f'there must be three parts to a version, not {len(versionNumbers)}')

        return tuple(versionNumbers)

    def __str__(self):
        return f'v{self._major}.{self._minor}.{self._patch}'

    def __repr__(self):
        return f'Version({self.__str__()})'

    def __hash__(self):
        return hash((self._major, self._minor, self._patch))

    def __eq__(self, other: 'Version'):
        if isinstance(other, Version):
            return self.__hash__() == other.__hash__()
        return NotImplemented

    def __lt__(self, other: 'Version'):
        if isinstance(other, Version):
            if self._major < other._major:
                return True
            elif self._major > other._major:
                return False
            else:
                if self._minor < other._minor:
                    return True
                elif self._minor > other._minor:
                    return False
                else:
                    return self._patch < other._patch


class Changelog(dict):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def __getitem__(self, key):
        if isinstance(key, str):
            key = Version(key)
        return super().__getitem__(key)

    def __contains__(self, key):
        if isinstance(key, str):
            key = Version(key)
        return super().__contains__(key)


def parseVersion(line: str) -> Version:
    match = re.search(VERSION_PATTERN, line)
    if match is None:
        raise ValueError('no version match found in string')

    versionString = line[match.start()+1:match.end()-1]
    return Version(f'v{versionString}')


def getChangelogContents(filename: str):
    changelog = Changelog()
    with open(filename, 'r') as f:
        content = ''
        version = None
        for line in f:
            if re.search(LINE_DELIMITER, line):
                if version is not None:
                    changelog[version] = content.strip()
                    content = ''
                    version = parseVersion(line)
                else:
                    # first version found
                    version = parseVersion(line)
                    content = ''
            else:
                content += line
        if version is not None:
            changelog[version] = content.strip()

    return changelog


if __name__ == '__main__':

    if len(sys.argv) != 4:
        print(f'syntax: {__file__} TAG_NAME CHANGELOG_FILE_PATH OUTPUT_FILE_PATH')
        exit(1)

    TAG_NAME = sys.argv[1]
    CHANGELOG_FILE_PATH = sys.argv[2]
    OUTPUT_FILE_PATH = sys.argv[3]

    log = getChangelogContents(CHANGELOG_FILE_PATH)
    if not TAG_NAME in log:
        print(f'tag {TAG_NAME} not found in {CHANGELOG_FILE_PATH}')
        exit(1)

    with open(OUTPUT_FILE_PATH, 'w') as f:
        f.write(log[TAG_NAME])
