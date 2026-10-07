# Shared GSUB lookup coverage regression

The two synthetic variable fonts have Latin cmap glyphs and GSUB lookups
shared by `cyrl` and `latn`.  One font also has Cyrillic cmap glyphs.  The
base `rvrn` feature is empty; its substitutions occur in two
FeatureVariations records covering different parts of the weight axis.

The test verifies that Latin substitutes inherit the Latin base glyphs'
style, including outputs from both variation records, a chained
substitution, a ligature, and a feature registered only under `DFLT`.
Cyrillic substitutes inherit the Cyrillic style only in the font with
Cyrillic cmap glyphs.  An unreachable substitution keeps its fallback
style, and a dedicated small-cap style remains distinct from the default
style.  Loading the Latin substitutes must preserve their classification
after blue-zone initialization.

Before the fix, the shared lookup outputs get claimed by Cyrillic, which
is processed first.  When the font has no Cyrillic blue-zone characters,
metric initialization disables hinting for those outputs.  This also
disables stem darkening for glyphs such as Google Sans's `G.BRACKET.18`
and `y.BRACKET.18`, as reported in issue #1441.

`make-fonts.py` uses only Python's standard library and generates the fonts
at build time.  No external font or Python package is needed for this test.
The test is enabled for linked or dynamically loaded HarfBuzz builds;
dynamic builds skip it if no usable HarfBuzz library is available.

From the source root:

```sh
meson setup out -Dtests=enabled -Dharfbuzz=enabled
meson test -C out issue-1441 --print-errorlogs
```

Use `-Dharfbuzz=dynamic` to exercise the dynamic HarfBuzz bridge.
The generator, driver, and generated data are provided under the FreeType
project license, `LICENSE.TXT`.
