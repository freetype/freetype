#
#  test_tomarkdown.py
#
#    Tests for markdown formatter (tomarkdown.py).
#
#  Copyright 2018 by
#  Nikhil Ramakrishnan.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

"""Unit tests for `tomarkdown`.

This module contains tests for functions in `tomarkdown.py`.
"""

import content
import sources
import tomarkdown
import utils

# Create test objects
# create context and processor
source_processor  = sources.SourceProcessor()
content_processor = content.ContentProcessor()
# Names
project_title  = 'Test Docs'
project_prefix = 'test'
# retrieve the list of files to process
file_list = utils.make_file_list( ['./tests/assets/*.c'] )
for filename in file_list:
    source_processor.parse_file( filename )
    content_processor.parse_sources( source_processor )
# process sections
content_processor.finish()

formatter = tomarkdown.MdFormatter( content_processor,
                                    project_title,
                                    project_prefix )

def test_html_quote():
    test_string = '7 & 9 < 4 & 5 but 12 & 15 > 4 & 5'
    expt_string = '7 &amp; 9 &lt; 4 &amp; 5 but 12 &amp; 15 &gt; 4 &amp; 5'
    assert tomarkdown.html_quote(test_string) == expt_string

def test_normalize_url():
    global formatter
    url = 'protocol://test-url-with/[square-brackets]?and-query'
    expected_url = 'protocol://test-url-with/(square-brackets)?and-query'
    assert formatter.normalize_url( url ) == expected_url

def test_slugify():
    global formatter
    name     = 'FT_HAS_MULTIPLE_MASTERS'
    expected = 'ft_has_multiple_masters'
    assert formatter.slugify( name ) == expected

def test_slugify2():
    global formatter
    name     = 'FT_GetFilePath_From_Mac_ATS_Name'
    expected = 'ft_getfilepath_from_mac_ats_name'
    assert formatter.slugify( name ) == expected

def test_slugify3():
    global formatter
    name     = 'default-script'
    expected = 'default-script'
    assert formatter.slugify( name ) == expected

def test_make_section_url():
    global formatter
    expected_url = '../test-outline_processing/index.html'

    section = list(formatter.sections)[0]
    out_url = formatter.make_section_url( section, code = True )
    assert out_url == expected_url

def test_make_chapter_url():
    global formatter
    expected_text = '[Support API](test-toc.md#support-api)'

    section = list(formatter.sections)[0]
    out_text = formatter.make_chapter_url( section.chapter.title )

    assert out_text == expected_text

# eof
