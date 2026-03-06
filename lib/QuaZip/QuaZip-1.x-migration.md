**QuaZip 0.x -> 1.x migration guide**

This guide contains important information mostly for package
maintainers, although it can also be useful for developers
as well.

**TL;DR**

QuaZip 1.0 is supposed to be installed in parallel with 0.x,
**not** replace 0.x. Package maintainers should keep it in mind.
The developers should switch to 1.0 in their new releases if
they use CMake and/or want new features (although there are only few).

**What happened in 1.0**

QuaZip 1.0 introduced some important changes:

- modern CMake support was introduced;
- include paths and binary names were changed;
- qmake support was ditched completely.

This change was intended, thoroughly discussed and planned.
The reasons for this change were:

- CMake support was a mess because at that time I didn't understand
it at all, and was just blindly accepted all CMake-related PRs and
patches, which lead to numerous inconsistencies between qmake and CMake
builds;
- maintaining both qmake and CMake support in a consistent manner
would be next to impossible, as CMake installs some metadata
that is very useful, and there is no easy way to generate
it with qmake;
- old CMake style is a mess in itself, while modern CMake, even if
not at all easy to learn, at least leads to consistent, clean
and easy to use (for the users of the library) configuration
files.

It was thus impossible to make 1.0 a drop-in replacement for 0.x.
Specifically, since 0.x has this qmake/CMake inconsistency
(that even changed between versions in an unreasonable manner),
it wouldn't be possible to make 1.0 both consistent and 0.x-compatible.
Consistency was chosen over compatibility.

To avoid possible conflicts with 0.x, however, it was decided to make
1.0 co-installable with 0.x, by using different include paths and binary
names. Binary names had to be changed anyway because they were inconsistent
for qmake and CMake, and made no sense at all.

**The differences**

QuaZip 0.x uses this “scheme”:

- the includes went either to `include/quazip` or `include/quazip5`
depending both on qmake/CMake and Qt4/Qt5 usage (only the CMake+Qt5
combination would yield `quazip5`);
- the binary base name was either `quazip` or `quazip5`, again,
depending on qmake/CMake *and* Qt4/Qt5 choice;
- the CMake config files were installed only when building with CMake;
- the CMake package name was `QuaZip` (for Qt4) or `QuaZip5` (for Qt5);
- the CMake target name was the same as the binary name.

QuaZip 1.x uses this scheme:

- the includes go to `include/QuaZip-QtX-Y/quazip`, where `include` is
the installation prefix include path (`/usr/include` or something),
`X` is the major version of Qt, and `Y` is the full version of QuaZip,
e. g. `include/QuaZip-Qt5-1.0/quazip`;
- the binary base name is `quazipZ-qtX`, where `Z` is the major version of
QuaZip, e. g. `libquazip1-qt5.so.1.0.0`;
- the CMake package name is `QuaZip-QtX`, e. g. `QuaZip-Qt5`;
- the CMake target name is `QuaZip::QuaZip`.

This can't possibly conflict with any crazy 0.x setup, no matter
qmake or CMake. CMake users get the easy modern CMake way
(`find_package(QuaZip-Qt5)`, `target_link_libraries(... QuaZip::QuaZip)`)
of using QuaZip, and includes can be either in the `#include <quazipfile.h>`
or in the `#include <quazip/quazipfile.h>` style (which some qmake users
used), as both `include/QuaZip-QtX-Y` and `include/QuaZip-QtX-Y/quazip`
include paths are added by default. This ensures source
compatibility between 0.x and 1.x without any annoying search-and-replace.

**The intended use**

QuaZip 1.0 should be installed along with 0.x. Whatever applications
were built with 0.x should continue to happily use 0.x. When application
developers decide so, they should switch to QuaZip 1.0 in their new
releases (and preferably switch to CMake as well).

Package maintainers should *not* consider 1.0 an upgrade from 0.x,
but rather an independent package, pretty much the same way as Qt4 and Qt5
are separate packages. The same goes for future major
versions such as 2.0, whenever they are released. Or at least that's
the current plan.
