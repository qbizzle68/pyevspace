import os
import re
import sys

from functools import total_ordering


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
    # This parsing is based on rigorous formatting in the CHANGELOG.md. Each beginning of a
    # version's changed should start with a line consiting only of: '## [X.X.X] - YYYY-MM-DD'.
    # These will serve as the demarcation points with which to generate the changes.

    LINE_DELIMITER = re.compile('^## \[\d+.\d+.\d+\] - \d{4}-\d{2}-\d{2}\s*$')
    VERSION_PATTERN = re.compile('\[\d+.\d+.\d+\]')

    def __init__(self, filename: str):
        if not isinstance(filename, str):
            raise TypeError(f'filename must be a str type, not {type(filename)}')

        super().__init__()
        self._getContents(filename)

    def __getitem__(self, key):
        if isinstance(key, str):
            key = Version(key)
        return super().__getitem__(key)

    def __contains__(self, key):
        if isinstance(key, str):
            key = Version(key)
        return super().__contains__(key)

    @staticmethod
    def _parseVersion(line: str) -> Version:
        match = re.search(Changelog.VERSION_PATTERN, line)
        if match is None:
            raise ValueError('no version match found in string')

        versionString = line[match.start()+1:match.end()-1]
        return Version(f'v{versionString}')

    def _getContents(self, filename: str):
        with open(filename, 'r') as f:
            content = ''
            version = None
            for line in f:
                if re.search(self.LINE_DELIMITER, line):
                    if version is not None:
                        self.__setitem__(version, content.strip())
                        content = ''
                        version = self._parseVersion(line)
                    else:
                        # first version found
                        version = self._parseVersion(line)
                        content = ''
                else:
                    content += line
            if version is not None:
                self.__setitem__(version, content.strip())


if __name__ == '__main__':

    if len(sys.argv) != 4:
        print(f'syntax: {__file__} TAG_NAME CHANGELOG_FILE_PATH OUTPUT_FILE_PATH')
        exit(1)

    TAG_NAME = sys.argv[1]
    CHANGELOG_FILE_PATH = sys.argv[2]
    OUTPUT_FILE_PATH = sys.argv[3]

    log = Changelog(CHANGELOG_FILE_PATH)
    if not TAG_NAME in log:
        contents = 'No changes found in change log'
    else:
        contents = log[TAG_NAME]

    with open(OUTPUT_FILE_PATH, 'w') as f:
        f.write(contents)
