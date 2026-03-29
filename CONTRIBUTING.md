# Contributing Notes

## Release Process

### Prerequisites

#### Testing

Before any releases several things need to be done. The obvious is all tests
should pass, meaning all supported versions are built in isolated invironments
and pass all tests (including capsule tests). Tox should be used to do this
and these are the default tests ran by tox. Additionally the 'lint' and
'stubtest' environments of Tox should be ran and passed. The `tools/pre_push`
script will execute all of these, on top of testing that the documentation
builds. Documentation also need to be updated describing any changes made
that affect Python users interaction/effects of using the package.

Any modifications to source need appropriate tests added/modified. Test
should **ONLY** be *modified* if a bug is being fixed, and the test(s) allowed
behavior that is no longer being supported.

#### Versioning

All metadata needs to be up-to-date, which means the versions in the
`pyproject.toml`, `__init__.py`, and `README.md` files need to match
the next release version (`tools/version_match` can be used to validate
this). After a new release is created, subsequent pushes should increment
the version (using [Semantic Versioning](https://semver.org/spec/v2.0.0.html))
and add the 'dev' suffix so it is more explicit that the current state of
the repo is past a given release (e.g. after '0.1.2dev').

#### Changelog

Changes also need to be documented in `CHANGELOG.md`. If about to push
a release, ensure the most recent version heading is updated from 'Unreleased'
and the hyperlink is added to the bottom of the file.

#### Stubs

Any values to the module/types interface need to be reflected in `__init__.pyi`.

### Workflows

To support typo fixes and general errors in documentation, the `docs.yml`
workflow is triggererd on any push to master when any tracked files in `docs/`
(or the `docs.yml` file) have changes. This means in normal workflow, the
final push before adding a release tag will update the online documentation
bedore the release may be finalized. Contributors must ensure doc changes
that reference new behavior are not pushed until the corresponding release
tag is ready to be pushed immediately after. Similarly, any issues with the
`release.yml` workflow need to be addressed ASAP so live documentation
changes reflect the most recent available release in a timely manner.

## Tools
The project supports some simple wrappers around commands that can make
developing a bit easier. None of these commands are complex in nature,
but the script makes running them a bit easier. Use the --help command
argument for more info on what they do or read the comments at the top
of the file.
