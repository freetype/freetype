#
#  utils.py
#
#    Auxiliary functions for the `docmaker' tool (library file).
#
#  Copyright 2002-2018 by
#  David Turner.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

"""Utility functions for Docwriter.

This module provides various utility functions for Docwriter.
"""

import glob
import itertools
import logging
import os
import sys

log = logging.getLogger( __name__ )

# current output directory
#
output_dir = None
markdown_dir = "markdown"

def build_message():
    """Print build message to console."""
    path = os.path.join( output_dir, markdown_dir )
    path = os.path.normpath(path)
    log.info("Building markdown documentation to directory: %s", path)

def  index_key( s ):
    """Generate a sorting key.

    We want lexicographical order (primary key) except that capital
    letters are sorted before lowercase ones(secondary key).

    The primary key is implemented by lowercasing the input.  The
    secondary key is simply the original data appended, character by
    character.  For example, the sort key for `FT_x` is `fFtT__xx`,
    while the sort key for `ft_X` is `fftt__xX`.  Since ASCII codes of
    uppercase letters are numerically smaller than the codes of
    lowercase letters, `fFtT__xx` gets sorted before `fftt__xX`.
    """
    return " ".join( itertools.chain( *zip( s.lower(), s ) ) )


def  sort_order_list( input_list, order_list ):
    """Sort `input_list`, placing the elements of `order_list' in front."""
    new_list = order_list[:]
    for name in input_list:
        if not name in order_list:
            new_list.append( name )
    return new_list


def  open_output( filename, config = False ):
    """Divert standard output to a given project documentation file.

    Use `output_dir` to determine the filename location if necessary and
    save the old stdout handle in a tuple that is returned by this function.

    If `config` is set to True, file is written to the parent directory.
    This is because MkDocs (and other generators) require configuration
    files to be in the parent directory.
    """
    if output_dir and output_dir != "":
        if not config:
            filename = output_dir + os.sep + markdown_dir + os.sep + filename
        else:
            filename = output_dir + os.sep + filename

    old_stdout = sys.stdout
    new_file   = open( filename, "w" )
    sys.stdout = new_file

    return ( new_file, old_stdout )


def  close_output( output ):
    """Close the output that was returned by `open_output`."""
    output[0].close()
    sys.stdout = output[1]


def  check_output():
    """Check if output directory is valid."""
    global output_dir
    if output_dir:
        if output_dir != "":
            if not os.path.isdir( output_dir ):
                log.error( "Argument"
                           " '%s' is not a valid directory.",
                           output_dir )
                sys.exit( 2 )
        else:
            output_dir = None


def  file_exists( pathname ):
    """Check that a given file exists."""
    result = 1
    try:
        file_handle = open( pathname, "r" )
        file_handle.close()
    except Exception:
        result = None
        log.error( "%s couldn't be accessed.", pathname )

    return result

def clean_markdown_dir( ):
    """Remove markdown and yml files from a directory."""
    directory = output_dir + os.sep + markdown_dir
    if not os.path.exists(directory):
        return

    for entry in os.listdir(directory):

        # Don't remove hidden files from the directory.
        if entry.startswith('.'):
            continue
        path = os.path.join(directory, entry)
        if os.path.isdir(path):
            continue

        if entry.endswith('.md') or entry.endswith('.yml'):
            os.unlink(path)

def  make_file_list( args = None ):
    """Build a list of input files from a list or command-line arguments."""
    file_list = []

    if not args:
        args = sys.argv[1:]

    for pathname in args:
        if pathname.find( '*' ) >= 0:
            newpath = glob.glob( pathname )
            newpath.sort()  # sort files -- this is important because
                            # of the order of files
        else:
            newpath = [pathname]

        file_list.extend( newpath )

    if len( file_list ) == 0:
        file_list = None
    else:
        # now filter the file list to remove non-existing ones
        file_list = filter( file_exists, file_list )

    return file_list

# eof
