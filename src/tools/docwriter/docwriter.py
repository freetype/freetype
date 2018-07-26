#!/usr/bin/env python
#
#  docwriter.py
#
#    Convert source code markup to Markdown documentation.
#
#  Copyright 2002-2018 by
#  David Turner.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

#
# This program is a re-write of the original DocMaker tool used to generate
# the API Reference of the FreeType font rendering engine by converting
# in-source comments into structured HTML.
#
# This new version is capable of outputting XML/Markdown data as well as
# accepting more liberal formatting options.  It also uses regular expression
# matching and substitution to speed up operation significantly.
#

"""This libaray is used to Convert source code markup to Markdown
documentation."""

from __future__ import print_function

import argparse
import logging
import sys

import check
import content
import sources
import tomarkdown
import utils

logger = logging.getLogger()
log_level = logging.INFO


def setup_logger(level=logging.INFO):
    """Set up the logger."""
    logger.propagate = False
    stream = logging.StreamHandler()
    log_format = logging.Formatter("%(levelname)-7s -  %(message)s")
    stream.setFormatter(log_format)
    logger.addHandler(stream)

    logger.setLevel(level)


def main():
    """Main program loop."""

    global output_dir
    global log_level

    parser = argparse.ArgumentParser(description="DocWriter Usage information")
    parser.add_argument(
        "files",
        nargs="+",
        help="list of source files to parse, wildcards are allowed",
    )
    parser.add_argument(
        "-t",
        "--title",
        metavar="T",
        help="set project title, as in '-t \"My Project\"'",
    )
    parser.add_argument(
        "-o",
        "--output",
        metavar="DIR",
        required=True,
        help="set output directory, as in '-o mydir'",
    )
    parser.add_argument(
        "-p",
        "--prefix",
        metavar="PRE",
        help="set documentation prefix, as in '-p ft2'",
    )
    group = parser.add_mutually_exclusive_group()
    group.add_argument(
        "-q",
        "--quiet",
        help="run quietly, show only errors",
        action="store_true",
    )
    group.add_argument(
        "-v", "--verbose", help="increase output verbosity", action="store_true"
    )
    args = parser.parse_args()

    # process options
    project_title = "Project"
    project_prefix = None
    output_dir = None

    if args.title:
        project_title = args.title

    if args.output:
        utils.output_dir = args.output

    if args.prefix:
        project_prefix = args.prefix

    if args.quiet:
        log_level = logging.ERROR

    if args.verbose:
        log_level = logging.DEBUG

    # set up the logger
    setup_logger(level=log_level)
    log = logging.getLogger("docwriter")

    # check all packages
    status = check.check()
    if status != 0:
        sys.exit(3)

    utils.check_output()

    # create context and processor
    source_processor = sources.SourceProcessor()
    content_processor = content.ContentProcessor()

    # retrieve the list of files to process
    file_list = utils.make_file_list(args.files)
    for filename in file_list:
        source_processor.parse_file(filename)
        content_processor.parse_sources(source_processor)

    # process sections
    content_processor.finish()

    # clean up directory
    log.info("Cleaning output directory")
    utils.clean_markdown_dir()

    formatter = tomarkdown.MdFormatter(
        content_processor, project_title, project_prefix
    )

    # build the docs
    utils.build_message()

    formatter.toc_dump()
    formatter.index_dump()
    formatter.section_dump_all()


# if called from the command line
if __name__ == "__main__":
    main()

# eof
