#!/usr/bin/env python
#
# DocMaker is a very simple program used to generate HTML documentation
# from the source files of the FreeType packages.
#
# I should really be using regular expressions to do this, but hey,
# i'm too lazy right now, and the damn thing seems to work :-)
#   - David
#

import fileinput, sys, string

html_header = """
<html>
<header>
<title>FreeType 2 API Reference</title>
<basefont face="Georgia, Arial, Helvetica, Geneva">
<style content="text/css">
  P { text-align=justify }
  H1 { text-align=center }
  H2 { text-align=center }
  LI { text-align=justify }
</style>
</header>
<body text="#000000"
      bgcolor="#FFFFFF"
      link="#0000EF"
      vlink="#51188E"
      alink="#FF0000">
<center><h1>FreeType 2 API Reference</h1></center>
"""

html_footer = """
</body>
</html>
"""

code_header = """
<font color=blue><pre>
"""

code_footer = """
</pre></font>
"""

para_header = "<p>"
para_footer = "</p>"

block_header = """<center><hr width="550"><table width="550"><tr><td>"""
block_footer = "</table></center>"

source_header = """<center><table width="550"><tr bgcolor="#D6E8FF" width="100%"><td><pre>
"""
source_footer = """</pre></table></center>
<br><br>
"""

# The FreeType 2 reference is extracted from the source files. These contain
# various comment blocks that follow one of the following formats:
#
#  /**************************
#   *
#   *  FORMAT1
#   *
#   *
#   *
#   *
#   *************************/
#
#  /**************************/
#  /*                        */
#  /*  FORMAT2               */
#  /*                        */
#  /*                        */
#  /*                        */
#  /*                        */
#
#  /**************************/
#  /*                        */
#  /*  FORMAT3               */
#  /*                        */
#  /*                        */
#  /*                        */
#  /*                        */
#  /**************************/
#
# Each block contains a list of markers, each one can be followed by
# some arbitrary text or a list of fields. Here's an example:
#
#    <Struct>
#       MyStruct
#
#    <Description>
#       this structure holds some data
#
#    <Fields>
#       x :: horizontal coordinate
#       y :: vertical coordinate
#
#
# This example defines three markers: 'Struct', 'Description' & 'Fields'
# The first two markers contain arbitrary text, while the last one contains
# a list of field
#
# each field is simple of the format:  WORD :: TEXT....
#
# Note that typically, each comment block is followed by some source
# code declaration that may need to be kept in the reference..
#
# Note that markers can alternatively be written as "@MARKER:"
# instead of "<MAKRER>". All marker identifiers are converted to
# lower case during parsing, in order to simply sorting..
#
# We associate with each block the following source lines that do not
# begin with a comment. For example, the following:
#
#   /**********************************
#    *
#    * <mytag>  blabla
#    *
#    */
#
#    bla_bla_bla
#     bilip_bilip
#
#   /* - this comment acts as a separator - */
#
#     blo_blo_blo
#
#
#  will only keep the first two lines of sources with
#  the "blabla" block
#
#  However, the comment will be kept, with following source lines
#  if it contains a starting '#' or '@' as in:
#
#     /*@.....*/
#     /*#.....*/
#     /* @.....*/
#     /* #.....*/
#



##############################################################################
#
# The DocCode class is used to store source code lines
#
#   self.lines contains a set of source code lines that will
#   be dumped as HTML in a <PRE> tag.
#
#   the object is filled line by line by the parser, it strips the
#   leading "margin" space from each input line before storing it
#   in self.lines
#
class DocCode:

    def __init__( self, margin = 0 ):
        self.lines  = []
        self.margin = margin

    def add( self, line ):
        # remove margin whitespace
        if string.strip( line[: self.margin] ) == "":
            line = line[self.margin :]
        self.lines.append( line )


    def dump( self ):
        for line in self.lines:
            print "--" + line
        print ""

    def get_identifier( self ):
        # this function should never be called
        return "UNKNOWN_CODE_IDENTIFIER!!"

    def dump_html( self ):

        # clean the last empty lines
        l = len( self.lines ) - 1
        while l > 0 and string.strip( self.lines[l - 1] ) == "":
            l = l - 1

        print code_header
        for line in self.lines[0 : l]:
            print line
        print code_footer


##############################################################################
#
# The DocParagraph is used to store text paragraphs
# self.words is simply a list of words for the paragraph
#
# the paragraph is filled line by line by the parser..
#
class DocParagraph:

    def __init__( self ):
        self.words = []

    def add( self, line ):
        # get rid of unwanted spaces in the paragraph
        #
        # the following line is the same as
        #
        #   self.words.extend( string.split( line ) )
        #
        # but older Python versions don't have the `extend' attribute
        #
        last = len(self.words)
        self.words[last:last] = string.split( line )

    # this function is used to retrieve the first word of a given
    # paragraph..
    def get_identifier( self ):
        if self.words:
            return self.words[0]

        # should never happen
        return "UNKNOWN_PARA_IDENTIFIER!!"


    def dump( self ):

        max_width = 50
        cursor    = 0
        line      = ""

        for word in self.words:

            if cursor + len( word ) + 1 > max_width:
                print line
                cursor = 0
                line = ""

            line   = line + word + " "
            cursor = cursor + len( word ) + 1

        if cursor > 0:
            print line

        #print "§" #for debugging only


    def dump_html( self ):

        print para_header
        self.dump()
        print para_footer


###########################################################################
#
# DocContent is used to store the content of a given marker.
#
# the "self.items" list contains (field,elements) record, where
# "field" corresponds to a given structure fields or function
# parameter (indicated by a "::"), or NULL for a normal section
# of text/code
#
# hence, the following example:
#
#   <MyMarker>
#      this is an example of what can be put in a content section,
#
#      a second line of example text
#
#      x :: a simple test field, with some content
#      y :: even before, this field has some code content
#           {
#             y = x+2;
#           }
#
# should be stored as
#     [ ( None, [ DocParagraph, DocParagraph] ),
#       ( "x",  [ DocParagraph ] ),
#       ( "y",  [ DocParagraph, DocCode ] ) ]
#
# in self.items
#
# the DocContent object is entirely built at creation time, you must
# pass a list of input text lines lin the "lines_list" parameter..
#
#
class DocContent:

    def __init__( self, lines_list ):
        self.items  = []
        code_mode   = 0
        code_margin = 0
        text        = []
        paragraph   = None   # represents the current DocParagraph
        code        = None   # represents the current DocCode

        elements    = []     # the list of elements for the current field,
                             # contains DocParagraph or DocCode objects

        field       = None   # the current field

        for aline in lines_list:

            if code_mode == 0:
                line   = string.lstrip( aline )
                l      = len( line )
                margin = len( aline ) - l

                # if the line is empty, this is the end of the current
                # paragraph
                if l == 0 or line == '{':

                    if paragraph:
                        elements.append( paragraph )
                        paragraph = None

                    if line == "":
                        continue

                    code_mode   = 1
                    code_margin = margin
                    code        = None
                    continue

                words = string.split( line )

                # test for a field delimiter on the start of the line, i.e.
                # the token `::'
                #
                if len( words ) >= 2 and words[1] == "::":

                    # start a new field - complete current paragraph if any
                    if paragraph:
                        elements.append( paragraph )
                        paragraph = None

                    # append previous "field" to self.items
                    self.items.append( ( field, elements ) )

                    # start new field and elements list
                    field    = words[0]
                    elements = []
                    words    = words[2 :]

                # append remaining words to current paragraph
                if len( words ) > 0:
                    line = string.join( words )
                    if not paragraph:
                        paragraph = DocParagraph()
                    paragraph.add( line )

            else:
                # we're in code mode..
                line = aline

                # the code block ends with a line that has a single '}' on it
                # that is located at the same column that the opening
                # accolade..
                if line == " " * code_margin + '}':

                    if code:
                        elements.append( code )
                        code = None

                    code_mode   = 0
                    code_margin = 0

                # otherwise, add the line to the current paragraph
                else:
                    if not code:
                        code = DocCode()
                    code.add( line )

        if paragraph:
            elements.append( paragraph )

        if code:
            elements.append( code )

        self.items.append( ( field, elements ) )


    def get_identifier( self ):
        if self.items:
            item = self.items[0]
            for element in item[1]:
                return element.get_identifier()

        # should never happen
        return "UNKNOWN_CONTENT_IDENTIFIER!!"


    def dump( self ):
        for item in self.items:
            field = item[0]
            if field:
                print "<field " + field + ">"

            for element in item[1]:
                element.dump()

            if field:
                print "</field>        "

    def dump_html( self ):

        n        = len( self.items )
        in_table = 0

        for i in range( n ):
            item  = self.items[i]
            field = item[0]

            if not field:

                if in_table:
                    print "</td></tr></table>"
                    in_table = 0

                for element in item[1]:
                    element.dump_html()
            else:
                if not in_table:
                    print "<table cellpadding=4><tr valign=top><td>"
                    in_table = 1
                else:
                    print "</td></tr><tr valign=top><td>"

                print "<b>" + field + "</b></td><td>"

                for element in item[1]:
                    element.dump_html()

        if in_table:
            print "</td></tr></table>"


######################################################################################
#
#
# The DocBlock class is used to store a given comment block. It contains
# a list of markers, as well as a list of contents for each marker.
#
#   "self.items" is a list of ( marker, contents ) elements, where
#   'marker' is a lowercase marker string, and 'contents' is a DocContent
#   object
#
#   "self.source" is simply a list of text lines taken from the
#   uncommented source itself..
#
#   finally, "self.identifier" is  a simple identifier used to
#   uniquely identify the block
#
class DocBlock:

    def __init__( self, block_line_list = [], source_line_list = [] ):
        self.items  = []                 # current ( marker, contents ) list
        self.identifier = None
        marker      = None               # current marker
        content     = []                 # current content lines list
        alphanum    = string.letters + string.digits + "_"

        for line in block_line_list:
            line2  = string.lstrip( line )
            l      = len( line2 )
            margin = len( line ) - l

            if l > 3:
                ender = None
                if line2[0] == '<':
                    ender = '>'
                elif line2[0] == '@':
                    ender = ':'

                if ender:
                    i = 1
                    while i < l and line2[i] in alphanum:
                        i = i + 1
                    if i < l and line2[i] == ender:
                        if marker and content:
                            self.add( marker, content )
                        marker  = line2[1 : i]
                        content = []
                        line2   = string.lstrip( line2[i + 1 :] )
                        l       = len( line2 )
                        line    = " " * margin + line2

            content.append( line )

        if marker and content:
            self.add( marker, content )

        self.source = []
        if self.items:
            self.source = source_line_list


    # this function is used to add a new element to self.items
    #  'marker' is a marker string, or None
    #  'lines' is a list of text lines used to compute a list of
    #          DocContent objects
    #
    def add( self, marker, lines ):

        # remove the first and last empty lines from the content list
        l = len( lines )
        if l > 0:
            i = 0
            while l > 0 and string.strip( lines[l - 1] ) == "":
                l = l - 1
            while i < l and string.strip( lines[i] ) == "":
                i = i + 1
            lines = lines[i : l]
            l     = len( lines )

        # add a new marker only if its marker and its content list aren't empty
        if l > 0 and marker:
            content = DocContent(lines)
            self.items.append( ( string.lower(marker), content ) )
            if not self.identifier:
                self.identifier = content.get_identifier()



    def dump( self ):
        for i in range( len( self.items ) ):
            print "[" + self.items[i][0] + "]"
            content = self.items[i][1]
            content.dump()

    def dump_html( self ):

	types = [ 'type', 'struct', 'functype', 'function', 'constant',
                  'enum', 'macro' ]

        if not self.items:
              return

        # start of a block
        print block_header

        print "<h2>" + self.identifier + "</h2>"

        # print source code
        if not self.source:
            return

        lines = self.source
        l     = len( lines ) - 1
        while l >= 0 and string.strip( lines[l] ) == "":
            l = l - 1
        print source_header
        for line in lines[0 : l + 1]:
            print  line
        print source_footer

        # dump each (marker,content) element
        for element in self.items:

            marker  = element[0]
            content = element[1]

            if marker == "description":
                print "<ul>"
                content.dump_html()
                print "</ul>"

            elif not (marker in types):
                print "<h4>" + marker + "</h4>"
                print "<ul>"
                content.dump_html()
                print "</ul>"

            print ""

        print block_footer


# filter a given list of DocBlocks. Returns a new list
# of DocBlock objects that only contains element whose
# "type" (i.e. first marker) is in the "types" parameter
#
def filter_blocks( block_list, types ):

    new_list = []
    for block in block_list:
        if block.items:
            element = block.items[0]
            marker  = element[0]
            if marker in types:
                new_list.append( block )

    return new_list


# perform a lexicographical comparison of two DocBlock
# objects. Returns -1, 0 or 1
#
def block_lexicographical_compare( b1, b2 ):
    if not b1.identifier:
        return -1
    if not b2.identifier:
        return 1

    id1 = string.lower(b1.identifier)
    id2 = string.lower(b2.identifier)
    if id1 < id2:
        return -1
    elif id1 == id2:
        return 0
    else:
        return 1


def block_make_list( source_block_list ):
    list = []

    for block in source_block_list:
        docblock = DocBlock( block[0], block[1] )
        list.append( docblock )

    return list


# dump a list block as a single HTML page
#
def dump_html_1( block_list ):

    print html_header

    for block in block_list:
        block.dump_html()

    print html_footer




def make_block_list():
    """parse a file and extract comments blocks from it"""

    list   = []
    block  = []
    format = 0

    # we use "format" to store the state of our parser:
    #
    #  0 - wait for beginning of comment
    #  1 - parse comment format 1
    #  2 - parse comment format 2
    #
    #  4 - wait for beginning of source (or comment ??)
    #  5 - process source
    #

    comment     = []
    source      = []
    state       = 0

    for line in fileinput.input():

        l = len( line )
        if l > 0 and line[l - 1] == '\012':
            line = line[0 : l - 1]

        # stripped version of the line
        line2 = string.strip( line )
        l     = len( line2 )

        # if this line begins with a comment and we are processing some
        # source, exit to state 0
        #
        # unless we encounter something like:
        #
        #    /*@.....
        #    /*#.....
        #
        #    /* @.....
        #    /* #.....
        #
        if format >= 4 and l > 2 and line2[0 : 2] == '/*':
            if l < 4 or ( line2[3] != '@' and line2[3:4] != ' @' and
                          line2[3] != '#' and line2[3:4] != ' #'):
                list.append( ( block, source ) )
                format = 0

        if format == 0:  #### wait for beginning of comment ####

            if l > 3 and line2[0 : 3] == '/**':
                i = 3
                while i < l and line2[i] == '*':
                    i = i + 1

                if i == l:
                    # this is '/**' followed by any number of '*', the
                    # beginning of a Format 1 block
                    #
                    block  = []
                    source = []
                    format = 1

                elif i == l - 1 and line2[i] == '/':
                    # this is '/**' followed by any number of '*', followed
                    # by a '/', i.e. the beginning of a Format 2 or 3 block
                    #
                    block  = []
                    source = []
                    format = 2

        ##############################################################
        #
        # FORMAT 1
        #
        elif format == 1:

            # if the line doesn't begin with a "*", something went
            # wrong, and we must exit, and forget the current block..
            if l == 0 or line2[0] != '*':
                block  = []
                format = 0

            # otherwise, we test for an end of block, which is an
            # arbitrary number of '*', followed by '/'
            else:
                i = 1
                while i < l and line2[i] == '*':
                    i = i + 1

                # test for the end of the block
                if i < l and line2[i] == '/':
                    if block != []:
                        format = 4
                    else:
                        format = 0
                else:
                    # otherwise simply append line to current block
                    block.append( line2[i:] )

                continue

        ##############################################################
        #
        # FORMAT 2
        #
        elif format == 2:

            # if the line doesn't begin with '/*' and end with '*/',
            # this is the end of the format 2 format
            if l < 4 or line2[: 2] != '/*' or line2[-2 :] != '*/':
                if block != []:
                    format = 4
                else:
                    format = 0
            else:
                # remove the start and end comment delimiters, then
                # right-strip the line
                line2 = string.rstrip( line2[2 : -2] )

                # check for end of a format2 block, i.e. a run of '*'
                if string.count( line2, '*' ) == l - 4:
                    if block != []:
                        format = 4
                    else:
                        format = 0
                else:
                    # otherwise, add the line to the current block
                    block.append( line2 )

                continue



        if format >= 4:  #### source processing ####

            if l > 0:
                format = 5

            if format == 5:
                source.append( line )


    if format >= 4:
        list.append( [block, source] )

    return list



# This function is only used for debugging
#
def dump_block_list( list ):
    """dump a comment block list"""
    for block in list:
        print "----------------------------------------"
        for line in block[0]:
            print line
        for line in block[1]:
            print line

    print "---------the end-----------------------"



def main( argv ):
    """main program loop"""
    sys.stderr.write( "extracting comment blocks from sources...\n" )
    list = make_block_list()
    list = block_make_list(list)

    list2 = filter_blocks( list, ['type','macro','enum','constant', 'functype'] )
    #list2 = list
    list2.sort( block_lexicographical_compare )

    dump_html_1( list2 )
    #dump_doc_blocks( list )
    #dump_block_lists( list )
    #dump_html_1( list )


# If called from the command line
if __name__ == '__main__':
    main( sys.argv )


# eof
