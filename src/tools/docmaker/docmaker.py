#!/usr/bin/env python
#
#  DocMaker 0.2 (c) 2002 David Turner <david@freetype.org>
#
# This program is a re-write of the original DocMaker took used
# to generate the API Reference of the FreeType font engine
# by converting in-source comments into structured HTML
#
# This new version is capable of outputting XML data, as well
# as accepts more liberal formatting options
#
# It also uses regular expression matching and substitution
# to speed things significantly
#

from sources import *
from content import *
from tohtml  import *

import sys, os, time, string, glob, getopt


def file_exists( pathname ):
    """checks that a given file exists"""
    result = 1
    try:
        file = open( pathname, "r" )
        file.close()
    except:
        result = None
        sys.err.write( pathname + " couldn't be accessed\n" )

    return result


def make_file_list( args = None ):
    """builds a list of input files from command-line arguments"""

    file_list = []
    # sys.stderr.write( repr( sys.argv[1 :] ) + '\n' )

    if not args:
        args = sys.argv[1 :]

    for pathname in args:
        if string.find( pathname, '*' ) >= 0:
            newpath = glob.glob( pathname )
            newpath.sort()  # sort files -- this is important because
                            # of the order of files
        else:
            newpath = [pathname]
            
        last = len( file_list )
        file_list[last : last] = newpath

    if len( file_list ) == 0:
        file_list = None
    else:
        # now filter the file list to remove non-existing ones
        file_list = filter( file_exists, file_list )
    
    return file_list



def usage():
    print "\nDocMaker 0.2 Usage information\n"
    print "  docmaker [options] file1 [ file2 ... ]\n"
    print "using the following options:\n"
    print "  -h : print this page"
    

def main( argv ):
    """main program loop"""

    try:
        opts, args = getopt.getopt( argv[1:],"h", [ "help" ] )

    except getopt.GetoptError:
        usage()
        sys.exit( 2 )

    if args == []:
        usage()
        sys.exit( 1 )

    # process options
    #
    for opt in opts:
        if opt[0] in ( "-h", "--help" ):
            usage()
            sys.exit( 0 )

    # create context and processor
    source_processor  = SourceProcessor()
    content_processor = ContentProcessor()

    # retrieve the list of files to process
    file_list = make_file_list()
    for filename in file_list:
        source_processor.parse_file( filename )
        content_processor.parse_sources( source_processor )
        
    # process sections
    content_processor.finish()

    formatter = HtmlFormatter( content_processor, "Example", "zz" )

    formatter.toc_dump()
    formatter.index_dump()
    formatter.section_dump_all()


# if called from the command line
#
if __name__ == '__main__':
    main( sys.argv )


# eof
