# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.13.0] - 2023-07-24

### Added

- This change log file.

### Changed

- Changed prior version number, so the project now adheres to semantic versioning.

## Note for versions prior to 0.13.0:

Prior to version 0.13.0 this project did not adhere strictly to semantic versioning. 
While all versions are < 1.0.0, they do follow the rule that the api is not stable
and can change at any time, however the numbering scheme used an incorrect formatting.
A leading zero ('0.') was used and was meant to signify the alpha nature of the project.
This of course was not needed, and should not have been used. From version 0.13.0 and on,
all previous versions will be refered to without the leading zero, with the exception of
the versions found on pypi (or using pip), and the version numbers used in git commit messages.
Moving forward these versions should not need to be referenced too often, and upon stable
release will be removed from pip and git releases.

## [0.12.5] - 2023-01-25

### Added

- Source code is compliant back to python 3.8.
- Now use tox to test against all compatible python versions.
- Added a Github workflow to test every commit pushed to repo.

### Fixed

- Fixed typo in Py_UNUSED macro that prevented successful build on Linux environment.
- Improved efficiency for offset rotation functions.

## [0.12.4] - 2023-01-10

### Added

- Added to documentation.

### Fixed

- Reduced long names in source for readability.
- Fixed ReferenceFrame method signatures to take EVSpace_ReferenceFrame* instead of EVSpace_Matrix* type.

### Removed

- Removed Rotation type from the c-api capsule.

## [0.12.3] - 2023-01-08

### Fixed

- Replaced matrix transpose with left-hand vector/matrix multiplication for efficiency increases.

## [0.12.2] - 2023-01-08

### Added

- Added left-hand vector multiplication of matrices.

## [0.12.1] - 2023-01-08

### Fixed

- Set project descriptions for pypi based on module docstring and readme.

## [0.12.0] - 2023-01-08

### Added

- Readme now links to documentation.
- Added rotation functionality with Euler angle and axis order types.
- Added functions for generating matrices, rotating vectors and automating
  the rotation process for you.

### Changed

- Changed EVector and EMatrix data type names to Vector and Matrix respectively.
- Changed to Rotation object to ReferenceFrame type.

### Fixed

- Function docstrings now properly show signatures.

## [0.11.2] - 2022-12-06

### Added

- Documentation and buld info with Sphinx.

## [0.11.1] - 2022-11-27

### Changed

- Made short description a single line.

## [0.11.0] - 2022-11-27

### Changed

- Added/updated readme and loading readme as __doc__ for package.

## [0.10.4] - 2022-11-27

### Added

- Buffer protocol for Vector and Matrix types.

## [0.10.3] - 2022-11-26

### Added

- Efficiency improvments in methods constructing new Vector types (via move semantics).

## [0.10.1] - 2022-11-26

### Fixed

- Fixed __str__() and __repr__() raising exceptions due to C-string buffer overflows.

## [0.10.0] - 2022-11-17

### Fixed

- Fixed inability for vectors that are extremely close to equate to True.

## [0.9.0] - 2022-11-16

### Added

- Added empty constructor for EVector.
- Added dot(), cross(), norm(), vang(), and vcxl() as module methods.
- Test and example modules within pyevspace package.
- Added elementary vectors and matrix.
- Added __reduce__() methods for pickling and copying using native modules.

### Changed

- Completely reorganized internal source structure.
- Rewrote tests and examples.


## [0.8.0] - 2022-08-29

### Changed

- Moved test files out of src directory.
- Cleaned up example files.
- Cleaned up setup.py.

## [0.7.0] - 2022-08-28

### Fixed

- Graft include directory with MANIFEST.in while uploading to pypi.

## 0.6.0 - 

## 0.5.0 - 

## [0.4.0] - 2022-07-23

### Added

- Updated readme to reflect recent additions.

## [0.3.0] - 2022-07-23

### Added

- Added initialization of EMatrix types with sequences.
- Added class methods for returning copies of instances.

### Changed

- vang() function return units now return radians instead of degrees.

## [0.2.0] - 2022-05-04

### Added

- Added to readme.

### Fixed

- Fixed poor grammer in docstrings.

## [0.1.0] - 2022-05-04

Initial version of project.

[unreleased]: https://github.com/qbizzle68/pyevspace/compare/v0.13.0...HEAD
[0.13.0]: https://github.com/qbizzle68/pyevspace/compare/v0.12.5...v0.13.0
[0.12.5]: https://github.com/qbizzle68/pyevspace/compare/v0.12.4...v0.12.5
[0.12.4]: https://github.com/qbizzle68/pyevspace/compare/v0.12.3...v0.12.4
[0.12.3]: https://github.com/qbizzle68/pyevspace/compare/v0.12.2...v0.12.3
[0.12.2]: https://github.com/qbizzle68/pyevspace/compare/v0.12.1...v0.12.2
[0.12.1]: https://github.com/qbizzle68/pyevspace/compare/v0.12.0...v0.12.1
[0.12.0]: https://github.com/qbizzle68/pyevspace/compare/v0.11.2...v0.12.0
[0.11.2]: https://github.com/qbizzle68/pyevspace/compare/v0.11.1...v0.11.2
[0.11.1]: https://github.com/qbizzle68/pyevspace/compare/v0.11.0...v0.11.1
[0.11.0]: https://github.com/qbizzle68/pyevspace/compare/v0.10.4...v0.11.0
[0.10.4]: https://github.com/qbizzle68/pyevspace/compare/v0.10.3...v0.10.4
[0.10.3]: https://github.com/qbizzle68/pyevspace/compare/v0.10.1...v0.10.3
[0.10.1]: https://github.com/qbizzle68/pyevspace/compare/v0.10.0...v0.10.1
[0.10.0]: https://github.com/qbizzle68/pyevspace/compare/v0.9.0...v0.10.0
[0.9.0]: https://github.com/qbizzle68/pyevspace/compare/v0.8.0...v0.9.0
[0.8.0]: https://github.com/qbizzle68/pyevspace/compare/v0.7.0...v0.8.0
[0.7.0]: https://github.com/qbizzle68/pyevspace/compare/v0.4.0...v0.7.0
[0.4.0]: https://github.com/qbizzle68/pyevspace/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/qbizzle68/pyevspace/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/qbizzle68/pyevspace/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/qbizzle68/pyevspace/releases/tag/v0.1.0