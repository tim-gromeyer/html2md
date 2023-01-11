# Change log

[TOC]

## v1.4.0

- Improved CMake support massively!
- Fixed tests
- Added support for CMake 3.8
- Fix Python source package

## v1.3.0

**BREAKING CHANGES!**

- Renamed `Converter::Convert2Md` -> `Converter::convert()`
- Renamed `options` -> `Options`

## v1.2.2

- Fixed bug when calling `Convert2Md()` multiple times
- Corrected serval typos. Ignore the rest of the change log.

## v1.2.1

- Added missing python dependency

## v1.2.0

- **Added python bindings**
- Added new option: `includeTable`.

## v1.1.5

- Added more command line options to the executable

## v1.1.4

- Releases now include deb files

## v1.1.3

The user can now test his own Markdown files. Simply specify to the test program as argument.

## v1.1.2

- Add changes for v1.1.1
- Create releases when a new tag is added(automatically)

## v.1.1.1

- Fix windows build(by replacing get)

## v1.1.0

- Reworked command line program
- Renamed `AppendToMd` to `appendToMd`
- Renamed `AppendBlank` to `appendBlank`
- **Require *c++11* instead of *c++17*.** Only the tests require *c++17* now.
- Added more tests
- Fix typos in comments
- Improved documentation

## v1.0.1

- Fixed several bugs
- Added more tests: make test
- Updated documentation: make doc
- Added packaging: make package

## v1.0.0

Initial release. All basics work but `blockquote` needs a rework.

