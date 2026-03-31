# Contributing Notes

## Getting Started

After cloning the repo you should run the `./setup-dev` script which will
symlink git hooks and can also be used to initialize the submodules by
using the `--update-submodules` option.

## Release Process

### Testing

Before any releases several things need to be done. The obvious is all tests
should pass, meaning all supported versions are built in isolated invironments
and pass all tests (including capsule tests). Tox should be used to do this
and these are the default tests ran by tox. Additionally the 'lint' and
'stubtest' environments of Tox should be ran and passed. The `scripts/tools/pre_push`
script will execute all of these, on top of testing that the documentation
builds. Documentation also need to be updated describing any changes made
that affect Python users interaction/effects of using the package.

Any modifications to source need appropriate tests added/modified. Test
should **ONLY** be *modified* if a bug is being fixed, and the test(s) allowed
behavior that is no longer being supported.

### Versioning

All metadata needs to be up-to-date, which means the versions in the
`pyproject.toml`, and `README.md` files need to match
the next release version (`scripts/tools/version_match` can be used to validate
this). After a new release is created, subsequent pushes should increment
the version (using [Semantic Versioning](https://semver.org/spec/v2.0.0.html))
and add the 'dev' suffix so it is more explicit that the current state of
the repo is past a given release (e.g. after '0.1.2dev').

### Changelog

Changes also need to be documented in `CHANGELOG.md`. If about to push
a release, ensure the most recent version heading is updated from 'Unreleased'
and the hyperlink is added to the bottom of the file.

### Stubs

Any values to the module/types interface need to be reflected in `__init__.pyi`.

### Workflows

Documentation changes will now only be built and pushed on release. There is
no real way (without using multiple branches) to continually update the
docs and push to master while working on a prerelease and not have those
changes, which are not reflected in any current release, pushed to the
active documentation site. This may mean a patch update be required for
solely documentation fixes, but that's something we have to live with.

## Tools
The project supports some simple wrappers around commands that can make
developing a bit easier. None of these commands are complex in nature,
but the script makes running them a bit easier. Use the --help command
argument for more info on what they do or read the comments at the top
of the file. The tools can be found in `/scripts/tools`
