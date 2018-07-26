#
#  test_utils.py
#
#    Tests for utility functions (utils.py).
#
#  Copyright 2018 by
#  Nikhil Ramakrishnan.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

"""Unit tests for `utils`.

This module contains tests for functions in `utils.py`.
"""

import utils
import sys

def test_index_key():
    test_dict = {"hello": "world", "foo": "bar", "FOO": "BAZ",
                 "HELLO": "WORLD", "zzz": "sleep"}
    # expected output
    out_list  = ["FOO", "foo", "HELLO", "hello", "zzz"]
    block_index = test_dict.keys()
    block_index = sorted( block_index, key = utils.index_key )
    assert block_index == out_list

def test_sort_order_list():
    input_list = ["z", "b", "a"]
    order_list = ["b", "c", "d"]
    # expected output
    expected   = ["b", "c", "d", "z", "a"]

    out_list = utils.sort_order_list(input_list, order_list)
    assert out_list == expected

def test_output( tmpdir ):
    # check if sys.stdout is diverting to file
    # this tests both open_output and close_output
    utils.output_dir = str( tmpdir )
    old_std = sys.stdout
    out = utils.open_output("test.txt", config=True)
    assert sys.stdout != old_std
    utils.close_output( out )
    assert sys.stdout == old_std

def test_make_file_list( tmpdir ):
    utils.output_dir = tmpdir
    f1   = tmpdir.join( "test1.c" )
    f2   = tmpdir.join( "test2.c" )
    f3   = tmpdir.join( "test3.txt" )
    f1.write( "foo" )
    f2.write( "bar" )
    f3.write( "baz" )
    args = [str( tmpdir + '/*.c' )]
    expected = ['test1.c', 'test2.c']

    out_list = utils.make_file_list( args )
    out_list = [f for f in out_list]
    for i in range( len( expected ) ):
        assert expected[i] in out_list[i]

# eof
