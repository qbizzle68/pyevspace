=======================
PyEVSpace Release Notes
=======================

.. currentmodule:: pyevspace

0.15.0
======

This update completely re-implemented the library. The library now wraps a much
better defined separate C++ library. All function naming conventions changed from
camel case to snake case. No global functions survive, and were either moved to
instance methods, renamed to more complete names, or combined into a single function
that supports multiple signatures.

Added
-----

- :py:attr:`Vector.E1`, :py:attr:`Vector.E2`, and :py:attr:`Vector.E3` are now the elementary vector attribute names.
- :py:attr:`Matrix.IDENTITY` is the identity matrix attribute name.
- :py:type:`EulerAngles` type name which replaces the `Angles` type.
- :py:func:`vector_angle()`, :py:func:`vector_cross()`, :py:func:`vector_dot()`, :py:func:`vector_exclude()`, and :py:func:`vector_proj()`
  module functions replace old functions with similar abreviated names.
- :py:meth:`Vector.norm()`, :py:meth:`Vector.magnitude()`, and :py:meth:`Vector.magnitude_squared()` added to :py:type:`Vector`.
- :py:meth:`Matrix.determinate()`, :py:meth:`Matrix.inverse()`, :py:meth:`Matrix.transpose()`, and
  :py:meth:`Matrix.transpose_inplace()` added to :py:type:`Matrix`.
- :py:meth:`Matrix.__getitem__()` supports slicing semantics now.
- :py:func:`compute_rotation_matrix()` to compute all rotation matrices.
- :py:func:`rotate_from()`, :py:func:`rotate_to()`, and :py:func:`rotate_between()` for rotating vectors.

Changed
-------

- New rotation function library names.
- Module vector functions have new names.
- Some :py:type:`Vector` instance methods have expanded names.
- :py:type:`Matrix` module methods are now instance methods.
- `pyevspace.norm()` is now an instance method.
- Elementary vectors on :py:type:`Vector` are now capitalized
- Identity matrix on :py:type:`Matrix` is now named Matrix.IDENTITY
- Renamed `Angles` type to :py:type:`EulerAngles` type.
- :py:type:`RotationOrder` is now hashable.
- Renamed :py:meth:`ReferenceFrame.rotateTo()` and :py:meth:`ReferenceFrame.rotateFrom()` methods.

Removed
-------

- Removed `Vector.e1` attribute, use :py:attr:`Vector.E1` now.
- Removed `Vector.e2` attribute, use :py:attr:`Vector.E2` now.
- Removed `Vector.e3` attribute, use :py:attr:`Vector.E3` now.
- Removed `Vector.mag()` function, use :py:meth:`Vector.magnitude()` now.
- Removed `Vector.mag2()` function, use :py:meth:`Vector.magnitude_squared()` now.
- Removed `Matrix.id` attribute, use :py:attr:`Matrix.IDENTITY` now.
- Removed `det()` module function, use :py:meth:`Matrix.determinate()` now.
- Removed `transpose()` module function, use :py:meth:`Matrix.transpose()` now.
- Removed `Angles` type, now named :py:type:`EulerAngles`.
- Removed `EulerAngles.alpha`, `EulerAngles.beta`, and `EulerAngles.gamma`. Use
  :py:meth:`EulerAngles.__getitem__()` instead.
- Removed `getMatrixAxis()` function.
- Removed `getMatrixEuler()` function.
- Removed `getMatrixFromTo()` function.
- Removed `rotateAxisFrom()` function.
- Removed `rotateAxisTo()` function.
- Removed `rotateEulerFrom()` function.
- Removed `rotateEulerTo()` function.
- Removed `rotateMatrixFrom()` function.
- Removed `rotateMatrixTo()` function.
- Removed `rotateOffsetFrom()` function.
- Removed `rotateOffsetTo()` function.
- Removed `ReferenceFrame.rotateToFrame()` method.
- Removed `ReferenceFrame.rotateFromFrame()` method.

0.14.2
======

Added
-----

- Much more automation via CI/CD.
- Build and deploy documentation to server after publishing new releases to PyPi.
- Added version verifying script to ensure version strings in the pyproject.toml and
  change log match the tag ref name.

Fixed
-----

- Fixed changing an Angles sub-angle via the __setitem__() method not recomputing the
  internal rotation matrix.

0.14.1
======

Added
-----

- Workflows to automatically build and distrubute releases and packages for use. This has
  no effect on the public API.

0.14.0
======

Changed
-------

- The alpha, beta, and gamma attributes of the Angles type are now properties of the class.
  This allows angles attributes of ReferenceFrame objects to call back to the owner and
  update the internal rotation matrix.
- The angles attribute is now copied when constructing ReferenceFrames. The angles object
  was stored as a reference before, which meant changing an angles attribute changed the
  attribute for all ReferenceFrame objects that were constructed with that Angles instance.
  Due to an implementation detail this is no longer possible.

Removed
-------

- Removed the alpha, beta, and gamma attributes of ReferenceFrame objects. These attributes
  were used to change the underlying matrix of the ReferenceFrame object, which is now
  handled by via a callback mechanism in the Angles class.
- Tests and examples packages are no longer distributed with pyevspace. They can still be
  found in the [source repository](https://github.com/qbizzle68/pyevspace).

Fixed
-----

- Improved docstring format for prettier printing while using the help command.

0.13.1
======

Security
--------

- Upgraded Pygments to 2.15.1 due to ReDoS vulnerability.
- Upgraded requests to 2.31.0 due to potentially leaking Proxy-Authorization headers.

0.13.0
======

Added
-----

- This change log file.

Changed
-------

- Changed prior version number, so the project now adheres to semantic versioning.

0.12.5
======

Added
-----

- Source code is compliant back to python 3.8.
- Now use tox to test against all compatible python versions.
- Added a Github workflow to test every commit pushed to repo.

Fixed
-----

- Fixed typo in Py_UNUSED macro that prevented successful build on Linux environment.
- Improved efficiency for offset rotation functions.

0.12.4
======

Added
-----

- Added to documentation.

Fixed
-----

- Reduced long names in source for readability.
- Fixed ReferenceFrame method signatures to take EVSpace_ReferenceFrame* instead of EVSpace_Matrix* type.

Removed
-------

- Removed Rotation type from the c-api capsule.

0.12.3
======

Fixed
-----

- Replaced matrix transpose with left-hand vector/matrix multiplication for efficiency increases.

0.12.2
======

Added
-----

- Added left-hand vector multiplication of matrices.

0.12.1
======

Fixed
-----

- Set project descriptions for pypi based on module docstring and readme.

0.12.0
======

Added
-----

- Readme now links to documentation.
- Added rotation functionality with Euler angle and axis order types.
- Added functions for generating matrices, rotating vectors and automating
  the rotation process for you.

Changed
-------

- Changed EVector and EMatrix data type names to Vector and Matrix respectively.
- Changed to Rotation object to ReferenceFrame type.

Fixed
-----

- Function docstrings now properly show signatures.

0.11.2
======

Added
-----

- Documentation and buld info with Sphinx.

0.11.1
======

Changed
-------

- Made short description a single line.

0.11.0
======

Changed
-------

- Added/updated readme and loading readme as __doc__ for package.

0.10.4
======

Added
-----

- Buffer protocol for Vector and Matrix types.

0.10.3
======

Added
-----

- Efficiency improvments in methods constructing new Vector types (via move semantics).

0.10.1
======

Fixed
-----

- Fixed __str__() and __repr__() raising exceptions due to C-string buffer overflows.

0.10.0
======

Fixed
-----

- Fixed inability for vectors that are extremely close to equate to True.

0.9.0
=====

Added
-----

- Added empty constructor for EVector.
- Added dot(), cross(), norm(), vang(), and vcxl() as module methods.
- Test and example modules within pyevspace package.
- Added elementary vectors and matrix.
- Added __reduce__() methods for pickling and copying using native modules.

Changed
-------

- Completely reorganized internal source structure.
- Rewrote tests and examples.

0.8.0
=====

Changed
-------

- Moved test files out of src directory.
- Cleaned up example files.
- Cleaned up setup.py.

0.7.0
=====

Fixed
-----

- Graft include directory with MANIFEST.in while uploading to pypi.

0.6.0
=====

0.5.0
=====

0.4.0
=====

Added
-----

- Updated readme to reflect recent additions.

0.3.0
=====

Added
-----

- Added initialization of EMatrix types with sequences.
- Added class methods for returning copies of instances.

Changed
-------

- vang() function return units now return radians instead of degrees.

0.2.0
=====

Added
-----

- Added to readme.

Fixed
-----

- Fixed poor grammer in docstrings.
