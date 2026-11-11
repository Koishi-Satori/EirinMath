Changelog
=========

Changelog since the version v1.0.0

v1.1.3
--------

- Fix header installation so the library can be installed correctly.

**Full Changelog**: https://github.com/Koishi-Satori/EirinFixed/compare/v1.1.2...v1.1.3

v1.1.2
--------

- Add ``numbers.hpp`` to provide fixed point constants.
- Add ``detail/util.hpp`` to provide some utility functions.
- Make sure compile passed without int128 support.
- Adjust test and CI.
- Update numbers and documentation.
- rename ``fpmath.hpp`` to ``math.hpp``.

**Full Changelog**: https://github.com/Koishi-Satori/EirinFixed/compare/v1.1.1...v1.1.2

v1.1.1
--------

## What's Changed
- Port to MSVC
- Update project structure
- Update documents
- Remove dependency on boost
- @HenryAWE made their first contribution in https://github.com/Koishi-Satori/EirinFixed/pull/5

**Full Changelog**: https://github.com/Koishi-Satori/EirinFixed/compare/v1.0.2...v1.1.1

v1.0.2
--------

- add specialization for ``std::numeric_limits`` for ``eirin::fixed_point``.
- add functions ``eirin::max_value()`` and ``eirin::min_value()``.
- add build options.
- add documentation.
- replace `std::is_signed` specialization for int128_t in Boost with ``eirin::detail::is_signed``.
- fix recruit inline.
- fix the stream output function return wrong result when the number is min_value.

**Full Changelog**: https://github.com/Koishi-Satori/EirinFixed/compare/v1.0.1...v1.0.2

v1.0.1
--------

- add a namespace ``eirin``.
- fix the output function
- add the ``is_fixed_point_v`` and the concept ``fixed_point``, you can use ``fixed_point auto& fp`` to represent a fixed point number rather than boring template declaration!

**Full Changelog**: https://github.com/Koishi-Satori/EirinFixed/compare/v1.0.0...v1.0.1

v1.0.0
-------

- The first release of EirinFixed.
- Basic fixed point number template class.
- Basic fixed point number mathematics and IO.
