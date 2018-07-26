#
#  test_parse.py
#
#    Tests for docwriter parsing (sources.py and content.py).
#
#  Copyright 2018 by
#  Nikhil Ramakrishnan.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

"""Docwriter parse tests.

The tests in this module use the `SourceProcessor` and
`ContentProcessor` classes to test file and content parsing.
"""

import content
import sources
import utils

# create context and processor
source_processor  = sources.SourceProcessor()
content_processor = content.ContentProcessor()

def test_parse_file():
    # retrieve the list of files to process
    file_list = utils.make_file_list( ['./tests/assets/*.c'] )
    for filename in file_list:
        source_processor.parse_file( filename )
    # get blocks
    blocks = source_processor.blocks
    count  = len( blocks )

    # there must be 12 blocks in file
    assert count == 12

def test_parse_source():
    # retrieve the list of files to process
    file_list = utils.make_file_list( ['./tests/assets/*.c'] )
    for filename in file_list:
        source_processor.parse_file( filename )
        content_processor.parse_sources( source_processor )
    # process sections
    content_processor.finish()
    # get headers
    headers = content_processor.headers
    # expected values
    expected_key = 'freetype/ftbbox.h'
    expected_val = 'FT_BBOX_H'

    assert headers[expected_key] == expected_val

# eof
