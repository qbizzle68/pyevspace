f"""This script produces the contents of a change log under a certain version into an output
file.

The semantics of this rely on two very specific assumptions.

    - First, all version tags are of the form vX.X.X where X is any numeric value.
      This is checked with the regular expression r'^v\d+.\d+.\d+$'.
    - The second assumption made is that each heading for a versions changes is of the form
      '## [X.X.X] - YYYY/MM/DD', which matches the regular expression
      r'^## [\d+.\d+.\d+] - \d{{4}}-\d{{2}}-\d{{2}}\w*$' (note that whitespace is allowed after the
      title. These headings act as delimiters for the content of version changes.
      
The syntax of the script is {__file__} <TAG_NAME> <CHANGELOG_PATH> <OUTPUT_FILE_PATH>. A tag
that is not of the form vX.X.X will produce an error, and a valid tag that is not found in the
change log will output 'No changes found in change log.'."""

import os
import re
import sys

from functools import total_ordering


@total_ordering
class Version:
    """Class to represent a semantic version number. All semantic version strings must be of
    the form vX.X.X where X is any non-negative integer. The class supports comparison operators
    to compare versions, and is also hashable."""

    __slots__ = '_major', '_minor', '_patch'

    def __init__(self, versionString: str):
        """Initializes a Version object by parsing a semantic version string and setting the major,
        minor and patch values."""

        major, minor, patch = self._parseVersion(versionString)
        self._major = int(major)
        self._minor = int(minor)
        self._patch = int(patch)

    @staticmethod
    def _parseVersion(version: str):
        """Parse the version string for the major, minor, and patch values."""
        
        if not isinstance(version, str):
            raise TypeError(f'version must be a str type, not {type(version)}')
        # version strings must start with 'v'
        if version[0] != 'v':
            raise ValueError(f'version string must start with a \'v\', not {version[0]}')

        versionNumbers = version[1:].split('.')
        if len(versionNumbers) != 3:
            raise ValueError(f'there must be three parts to a version, not {len(versionNumbers)}')

        return tuple(versionNumbers)

    @property
    def major(self) -> int:
        """Returns the major part of the version."""

        return self._major

    @major.setter
    def major(self, value):
        raise AttributeError('major attribute is immutable')

    @property
    def minor(self):
        """Returns the minor part of the version."""

        return self._minor

    @minor.setter
    def minor(self, value):
        raise AttributeError('minor attribute is immutable')

    @property
    def patch(self):
        """Returns the patch part of the version."""

        return self._patch

    @patch.setter
    def patch(self, value):
        raise AttributeError('patch attribute is immutable')

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
    """Class to represent the contents of a change log. The Changelog class subclasses the dict
    type, and is meant to hold change contents as values to the Version keys. The Version of the
    content of the change log is parsed based on rigorous but simple formatting rules. The content
    of a version will be the text between the two lines of the format '## [X.X.X] - YYYY-MM-DD'.
    
    The __getitem__ and __contains__ methods are overridden to also support value lookup with a
    key of type string. For example if v123 is a Version object constructed with a version string
    of 'v1.2.3', then log['v1.2.3'] is the same thing as log[v123]."""

    LINE_DELIMITER = re.compile('^## \[\d+.\d+.\d+\] - \d{4}-\d{2}-\d{2}\s*$')
    VERSION_PATTERN = re.compile('\[\d+.\d+.\d+\]')

    def __init__(self, filename: str):
        """Instantiate a Changelog object with the contents of a change log with a given filename."""

        if not isinstance(filename, str):
            raise TypeError(f'filename must be a str type, not {type(filename)}')

        super().__init__()
        self._getContents(filename)

    def __getitem__(self, key):
        # support lookup with a version string as well as Version key
        if isinstance(key, str):
            key = Version(key)
        return super().__getitem__(key)

    def __contains__(self, key):
        # support in keyword with a version string as well as Version key
        if isinstance(key, str):
            key = Version(key)
        return super().__contains__(key)

    @staticmethod
    def _parseVersion(line: str) -> Version:
        """Retrieve the version number from a delimeter line."""

        match = re.search(Changelog.VERSION_PATTERN, line)
        # lose the brackets in the string
        versionString = match[0][1:-1]
        
        return Version(f'v{versionString}')

    def _getContents(self, filename: str):
        """Maps the version with the change contents associated with it."""

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
            # no match could have been found, so version would still be None
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
        contents = 'No changes found in change log.'
    else:
        contents = log[TAG_NAME]

    with open(OUTPUT_FILE_PATH, 'w') as f:
        f.write(contents)
