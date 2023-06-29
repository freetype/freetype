# FreeType ![Badge Version]

*A freely available software library to render fonts.*

---

 **[❮ Website ❯][Website]**
 **[❮ Install ❯][INSTALL]**
 **[❮ Changes ❯][CHANGES]**
 **[❮ Documentation ❯][Documentation]**
 **[❮ API ❯][API]**
 **[❮ License ❯][LICENSE]**

---

**FreeType** is a **C** library, designed to be small, efficient, highly
customizable, and portable while capable of producing high-quality
output (glyph images) of most vector and bitmap font formats.

---
![alt text](https://freetype.org/image/fond3.png "Title Text")

# Documentation
The FreeType 2 API reference for the latest release, along with additional
documentation can be found online at:

>https://freetype.org/freetype2/docs/documentation.html

Refer to [DOCGUIDE] for more information.


# Repo Mirrors

FreeType's official git repository is located at

>https://gitlab.freedesktop.org/freetype

from which the 'freetype.git' and 'freetype-demos.git' repositories
can be cloned in the usual way.

FreeType:

```bash
git clone https://gitlab.freedesktop.org/freetype/freetype.git
```

FreeType-demos:
```bash
git clone https://gitlab.freedesktop.org/freetype/freetype-demos.git
```

If you want to use the Savannah mirror instead, you have to do a
slightly different incantation because the repository names contain
digit '2' for historical reasons.

```bash
git clone https://git.savannah.nongnu.org/git/freetype/freetype2.git
```

```bash
git clone https://git.savannah.nongnu.org/git/freetype/freetype2-demos.git
```


# Compiling FreeType
FreeType supports compilation via meson, GNU make and CMake

Refer to [INSTALL] for more information.


# Licensing
FreeType is dual-licensed under the FTL and GPLv2.

Refer to [LICENSE] for more information.


# Reporting Issues
Please submit bug reports at

>https://gitlab.freedesktop.org/freetype/freetype/-/issues

If you have suggestions for improving FreeType, they should be sent
to the `freetype-devel` mailing list.


# Improving FreeType
For instructions on compiling FreeType, see [INSTALL].

Please send merge requests to our gitlab repo at:

>https://gitlab.freedesktop.org/freetype/freetype/

Alternatively, you can send patches to the `freetype-devel` mailing list.
Details on the process can be found here:

>https://www.freetype.org/developer.html#patches

Any non-trivial contribution should first be discussed with the maintainers
via the `freetype-devel` mailing list.


# Contact
The preferred way of communication with the FreeType team is using
mailing lists.

*↳* [*How do I subscribe?*][Contact]


| Email                        | Details                       |
|------------------------------|-------------------------------|
| freetype@nongnu.org          | General use and discussion    |
| freetype-devel@nongnu.org    | Engine internals, Porting etc |
| freetype-announce@nongnu.org | Announcements                 |
| freetype-commit@nongnu.org   | Git repository track          |

**The lists are moderated**


# Code of Conduct
Please note that this project is released with a Contributor Code of
Conduct (CoC). By participating in this project you agree to abide by
its terms, which you can find in the following link:

>https://www.freedesktop.org/wiki/CodeOfConduct

CoC issues may be raised to the project maintainers at the following
address:

 - wl@gnu.org
 - apodtele@gmail.com

---
```
Copyright (C) 2006-2023 by
David Turner, Robert Wilhelm, and Werner Lemberg.

This file is part of the FreeType project, and may only be used,
modified, and distributed under the terms of the FreeType project
license, LICENSE.TXT. By continuing to use, modify, or distribute
this file you indicate that you have read the license and understand
and accept it fully.
```


<!-------------------------------------------------------------------------->

[Website]: https://www.freetype.org
[Issues]: https://gitlab.freedesktop.org/freetype/freetype/-/issues
[Contact]: https://www.freetype.org/contact.html
[Merge Request]: https://gitlab.freedesktop.org/freetype/freetype/-/merge_requests
[Patches]: https://www.freetype.org/developer.html#patches
[Documentation]: https://freetype.org/freetype2/docs/documentation.html
[Releases]: https://download.savannah.gnu.org/releases/freetype/
[API]: https://freetype.org/freetype2/docs/reference/index.html

[INSTALL]: ./docs/INSTALL
[CHANGES]: ./docs/CHANGES
[LICENSE]: ./LICENSE.TXT
[DOCGUIDE]: ./docs/DOCGUIDE

[Badge Version]: https://gitlab.freedesktop.org/freetype/freetype/-/badges/release.svg
