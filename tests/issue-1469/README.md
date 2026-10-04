# Variable composite component-count regression

This test covers the signed-short counter fixed by 73720c7c99 in
`load_truetype_glyph`.  The component loop appends four phantom points after
copying its component positions.  Counts 32765–32767 cross the signed-short
index boundary while appending those points; 32764 is the adjacent safe
control.

`make-fonts.py` uses only Python's standard library.  It creates four small
synthetic TrueType variable fonts without reading any external font.  Each
has two glyphs: an empty glyph and a composite containing repeated
references to it.  The 'wght' axis spans 0 to 1, with a default of 0 and
empty 'gvar' records.  No actual outline geometry, instructions, or nonzero
deltas are needed to reach the counter code.

The generated fonts are about 197KB each.  Almost all of that is the
required six-byte component record repeated roughly 32767 times.  The fonts
are generated at build time so these repetitive binary files need not be
committed.

The C driver uses public FreeType APIs and checks both the returned glyph
format and the expected component count.  It first loads each font without
calling the variation API, then with `wght=1`.  The inactive control must
omit that call because older FreeType versions set `FT_IS_VARIATION` even
for an explicit default coordinate.  `FT_LOAD_NO_RECURSE` avoids assembling
component outlines; the vulnerable phantom-point code runs before that early
return.

From the source root:

```sh
python3 tests/scripts/download-test-fonts.py
CC=clang \
  meson setup out \
        -Dtests=enabled \
        -Dauto_features=disabled \
        -Db_sanitize=address \
        -Dbuildtype=debugoptimized \
        -Ddefault_library=static
meson test -C out --print-errorlogs
```

The font-generation target is part of the default build and an explicit test
dependency.  The test command builds the library, driver, and four fixtures,
then runs the complete test suite; no separate generation or compile command
is needed.  The download step prepares the existing upstream tests' fonts;
issue-1469 itself uses only its generated data.  To select only this
regression, append `issue-1469` to the test command.

The test has a 180-second allowance for its large component-count boundaries
on sanitizer builds.  A single case can also be run as:

```sh
out/tests/issue-1469 out/tests/issue-1469-32767.ttf 32767 65536
```

For deterministic identification of all three pre-fix boundary violations,
build the library and driver with Clang's
`-fsanitize=implicit-integer-truncation` in addition to ASan, and stop on
its first diagnostic.  It detects the 32768-to--32768 index conversion
itself.  ASan alone can miss a write half a megabyte before an allocation if
that address happens to be mapped and unpoisoned; ordinary crash behavior is
likewise allocator-dependent.

The fixed library must load all eight cases successfully.  This is a
regression test for an already-fixed parser bug, with no browser dependency
or execution payload.  These source files and their synthetic generated data
are provided under the FreeType project license, `LICENSE.TXT`.
