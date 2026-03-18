# Contributing Notes

## Release Process
To support typo fixes and general errors in documentation, the `docs.yml`
workflow is triggererd on any push to master when any tracked files in `docs/`
(or the `docs.yml` file) have changes. This means in normal workflow, the
final push before adding a release tag will update the online documentation
bedore the release may be finalized. Contributors must ensure doc changes
that reference new behavior are not pushed until the corresponding release
tag is ready to be pushed immediately after. Similarly, any issues with the
`release.yml` workflow need to be addressed ASAP so live documentation
changes reflect the most recent available release in a timely manner.
