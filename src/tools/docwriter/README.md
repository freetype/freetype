[![Build Status](https://travis-ci.com/nikramakrishnan/freetype-docwriter.svg?branch=master)](https://travis-ci.com/nikramakrishnan/freetype-docwriter)
[![Code Health](https://landscape.io/github/nikramakrishnan/freetype-docwriter/master/landscape.svg?style=flat)](https://landscape.io/github/nikramakrishnan/freetype-docwriter/master)

# FreeType Docwriter

Markdown documentation generator for the FreeType library.

## Setup Instructions

1.  Clone this repository.
2.  Clone the freetype2 repository from [here](http://git.savannah.gnu.org/cgit/freetype/freetype2.git/).
3.  Convert the `include/` folder to markdown using the 
    [freetype-docs](https://github.com/nikramakrishnan/freetype-docs/tree/markdown) repository.
5.  Copy files from `include_mark/`.
6.  Run:

    ```bash
    python -B docwriter.py --prefix=ft2 --title=FreeType-2.9.1 --output=./docs/reference \
    ./include_mark/freetype/*.h ./include_mark/freetype/config/*.h ./include_mark/freetype/cache/*.h
    ```

## Usage Information

```
docwriter [-h] [-t T] -o DIR [-p PRE] [-q | -v] files [files ...]

DocWriter Usage information

positional arguments:
  files                 list of source files to parse, wildcards are allowed

optional arguments:
  -h, --help            show this help message and exit
  -t T, --title T       set project title, as in '-t "My Project"'
  -o DIR, --output DIR  set output directory, as in '-o mydir'
  -p PRE, --prefix PRE  set documentation prefix, as in '-p ft2'
  -q, --quiet           run quietly, show only errors
  -v, --verbose         increase output verbosity
```

## Running Tests

There are two possible test scenarios:

1. Running tests on both py27 and py36 (using tox - requires both python versions installed).
2. Running tests on the currently installed Python version.

They are detailed below.

### Test using Tox

To test on both py27 and py36:

1.  Make sure `tox` is installed:
    ```bash
    pip install tox
    ```

2.  Ensure both py27 and py36 are installed.

3.  Run tests:
    ```bash
    tox
    ```

### Test on single python version

To test on current python version using pytest:

1.  Make sure `pytest` is installed:
    ```bash
    pip install pytest
    ```

2.  Run tests:
    ```bash
    python -m pytest
    ```

## License

This library is licensed under the [FreeType License](https://www.freetype.org/license.html).

## History

This library was originally written by David Turner as `docmaker` which collected and presented
documentation in HTML. It has since been modified multiple times, including a major refactor
to allow multiple output formats. The current `docwriter` is the biggest rewrite, with lots of
changes, additions etc. that allow it to be more flexible, readable, maintainable and usable.
