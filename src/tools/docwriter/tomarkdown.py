#
#  tomarkdown.py
#
#    A sub-class container of the `Formatter' class to produce Markdown.
#
#  Copyright 2018 by
#  Nikhil Ramakrishnan.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

# The parent class is contained in file `formatter.py'.

"""Subclass of `formatter` to generate Markdown.

This module subclasses `formatter` and implements syntax-specific
routines to build markdown output.
"""

import logging
import os
import re
import time

import mistune

from formatter import Formatter
import siteconfig
import sources

log = logging.getLogger( __name__ )

#---------------------------------------------------------------
# Begin initial configuration

# Docs Config.
api_ref_text = "API Reference"
docs_author  = "FreeType Contributors"

# Breadcrumbs Navigation config.
md_crumbs_sep = " &raquo; "

md_header_1 = """\
[FreeType](//www.freetype.org) &raquo; \
"""

md_header_2 = """\
[Docs](../) &raquo; \
"""

md_line_sep = """

-------------------------------\

"""

# Heading Default Text.
md_api_ref = """\
 API Reference
"""

# Chapter header/inter/footer.
chapter_header = """\
## \
"""

chapter_footer = ''

# Synopsis text
section_synopsis_header = '''
## Synopsis\
'''
section_synopsis_footer = ''

# Description header/footer.
description_header = ""
description_footer = ""

# Source code extracts header/footer.
source_header = """
<div class = "codehilite">
<pre>\
"""
source_footer = """\
</pre>
</div>\
"""

code_header   = "```"
code_footer   = "```"

# Source language keyword coloration and styling.
keyword_prefix = '<span class="keyword">'
keyword_suffix = '</span>'

# HTML paragraph header and footer.
para_header = "<p>"
para_footer = "</p>"

# General Markdown.
md_newline  = "\n"
md_h1       = "# "
md_h2       = "## "
md_h3       = "### "
md_h4       = "<h4>"
md_h4_inter = "</h4>"

md_hr = """\
<hr>
"""

# End of initial configuration
#---------------------------------------------------------------

def  html_quote( line ):
    """Change HTML special characters to their codes.

    Follows ISO 8859-1 Characters changed: `&`, `<` and `>`.
    """
    result = line
    if "`" not in result:
        result = line.replace( "&", "&amp;" )
        result = result.replace( "<", "&lt;"  )
        result = result.replace( ">", "&gt;"  )
    return result

################################################################
##
##  MARKDOWN FORMATTER CLASS
##
class  MdFormatter( Formatter ):

    def  __init__( self, processor, project_title, file_prefix ):
        Formatter.__init__( self, processor )

        if file_prefix:
            file_prefix = file_prefix + "-"
        else:
            file_prefix = ""

        self.headers        = processor.headers
        self.project_title  = project_title
        self.file_prefix    = file_prefix
        self.toc_filename   = self.file_prefix + "toc.md"
        self.index_filename = self.file_prefix + "index.md"
        self.markdown       = mistune.Markdown()
        self.config         = siteconfig.SiteConfig()

        self.md_index_header = (
            md_header_1 + md_header_2
            + "Global Index"
            + md_line_sep + md_h1
            + project_title + md_api_ref
        )

        self.md_toc_header = (
            md_header_1 + md_header_2
            + "Table of Contents"
            + md_line_sep + md_h1
            + project_title + md_api_ref
        )

        self.time_footer = (
            '<div class="timestamp">generated on '
            + time.asctime( time.gmtime() ) + " UTC"
            + "</div>" )

        self.columns = 3

        self.site_name        = project_title + " " + api_ref_text
        self.site_description = api_ref_text + " Documentation for " + project_title
        self.site_author      = docs_author

        # Set site config
        self.config.set_site_info( self.site_name, self.site_description,
                                   self.site_author )
        # Add toc and index
        self.config.add_single_page( "TOC", self.toc_filename )
        self.config.add_single_page( "Index", self.index_filename )

    def normalize_url( self, url ):
        # normalize url, following RFC 3986
        url = url.replace( "[", "(" )
        url = url.replace( "]", ")" )
        return url

    def slugify( self, name ):
        """Slugify a cross-reference.

        Python markdown uses a similar approach to process links so we
        need to do this in order to have valid cross-references.
        """
        name = name.lower().strip()
        name = name.replace( " ",  "-")
        return name

    def  make_section_url( self, section, code = False ):
        if code:
            return "../" + self.file_prefix + section.name + "/index.html"
        return self.file_prefix + section.name + ".md"

    def  make_block_url( self, block, name = None, code = False ):
        if name == None:
            name = block.name

        name = self.slugify( name )

        try:
            # if it is a field def, link to its parent section
            section_url = self.make_section_url( block.section, code )
        except Exception:
            # we already have a section
            section_url = self.make_section_url( block, code )

        return section_url + "#" + name

    def make_chapter_url( self, chapter ):
        chapter = ' '.join( chapter )
        slug_chapter = self.slugify( chapter )
        chapter_url = ( "[" + chapter + "]("
                        + self.toc_filename + "#" + slug_chapter + ")"
                      )
        return chapter_url

    def  make_md_word( self, word ):
        """Analyze a simple word to detect cross-references and markup."""
        # handle cross-references
        m = sources.re_crossref.match( word )
        if m:
            try:
                name = m.group( 'name' )
                rest = m.group( 'rest' )
                block = self.identifiers[name]
                url   = self.make_block_url( block, code = True )
                # display `foo[bar]' as `foo'
                name = re.sub( r'\[.*\]', '', name )
                # normalize url
                url = self.normalize_url( url )
                try:
                    # for sections, display title
                    url = ( '&lsquo;<a href="' + url + '">'
                            + block.title + '</a>&rsquo;'
                            + rest )
                except Exception:
                    url = ( '<a href="' + url + '">'
                            + name + '</a>'
                            + rest )

                return url
            except Exception:
                # we detected a cross-reference to an unknown item
                log.warn( "Undefined cross reference '%s'.", name )
                return '?' + name + '?' + rest

        return html_quote( word )

    def  make_md_para( self, words, in_html = False ):
        """Convert words of a paragraph into tagged Markdown text.

        Also handle cross references.
        """
        line = ""
        if words:
            line = self.make_md_word( words[0] )
            for word in words[1:]:
                line = line + " " + self.make_md_word( word )
            # handle hyperlinks
            line = sources.re_url.sub( r'<\1>', line )
            # convert '...' quotations into real left and right single quotes
            line = re.sub( r"(^|\W)'(.*?)'(\W|$)",
                           r'\1&lsquo;\2&rsquo;\3',
                           line )
            # convert tilde into non-breaking space
            line = line.replace( "~", "&nbsp;" )

        # Return
        if in_html:
            # If we are in an HTML tag, return with newline after para
            return line + md_newline
        # Otherwise return a Markdown paragraph
        return md_newline + line

    def  make_md_code( self, lines, lang ):
        """Convert a code sequence to markdown."""
        if not lang:
            lang = ''
        line = code_header + lang + '\n'
        for l in lines:
            # NOTE Markdown REQUIRES all special chars in code blocks
            line = line + l.rstrip() + '\n'

        return line + code_footer

    def  make_md_items( self, items, in_html = False ):
        """Convert a field's content into markdown."""
        lines = []
        for item in items:
            if item.lines:
                lines.append( self.make_md_code( item.lines, item.lang ) )
            else:
                lines.append( self.make_md_para( item.words, in_html ) )

        return '\n'.join( lines )

    def  print_md_items( self, items, in_html = False ):
        content = self.make_md_items( items, in_html )
        if in_html:
            # Parse markdown in content
            content = self.markdown( content ).rstrip()
        print( content )

    def print_md_para( self, words, in_html = False ):
        content = self.make_md_para( words, in_html )
        if in_html:
            # Parse markdown in content
            content = self.markdown( content ).rstrip()
        return content

    def  print_html_field( self, field ):
        if field.name:
            print( '<table><tr valign="top"><td><b>'
                   + field.name
                   + "</b></td><td>" )

        print( self.make_md_items( field.items ) )

        if field.name:
            print( "</td></tr></table>" )

    def  source_quote( self, line, block_name = None ):
        result = ""
        while line:
            m = sources.re_source_crossref.match( line )
            if m:
                name   = m.group( 2 )
                prefix = html_quote( m.group( 1 ) )
                length = len( m.group( 0 ) )

                if name == block_name:
                    # this is the current block name, if any
                    result = result + prefix + '<b>' + name + '</b>'
                    # result = result + prefix + name
                
                # Keyword highlighting
                elif sources.re_source_keywords.match( name ):
                    # this is a C keyword
                    result = ( result + prefix
                               + keyword_prefix + name + keyword_suffix )

                elif name in self.identifiers:
                    # this is a known identifier
                    block = self.identifiers[name]
                    iden  = block.name

                    # link to a field ID if possible
                    try:
                        for markup in block.markups:
                            if markup.tag == 'values':
                                for field in markup.fields:
                                    if field.name:
                                        iden = name

                        result = ( result + prefix
                                    + '<a href="'
                                    + self.make_block_url( block, iden, code = True )
                                    + '">' + name + '</a>' )
                    except Exception:
                        # sections don't have `markups'; however, we don't
                        # want references to sections here anyway
                        result = result + html_quote( line[:length] )

                else:
                    result = result + html_quote( line[:length] )

                line = line[length:]
            else:
                result = result + html_quote( line )
                line   = []

        return result

    def  print_md_field_list( self, fields ):
        is_long = False
        for field in fields:
            # if any field name is longer than
            # 25 chars change to long table
            if len( field.name ) > 25:
                is_long = True
                break
            # if any line has a code sequence
            # change to long table
            for item in field.items:
                if item.lines:
                    is_long = True
                    break
        if is_long:
            print( '<table class="fields long">' )
        else:
            print( '<table class="fields">' )
        for field in fields:
            print( '<tr><td class="val" id="' + self.slugify( field.name ) + '">'
                   + field.name
                   + '</td><td class="desc">' )
            self.print_md_items( field.items, in_html = True )
            print( "</td></tr>" )
        print( "</table>" )

    def  print_md_markup( self, markup ):
        table_fields = []
        for field in markup.fields:
            if field.name:
                # We begin a new series of field or value definitions.  We
                # record them in the `table_fields' list before outputting
                # all of them as a single table.
                table_fields.append( field )
            else:
                if table_fields:
                    self.print_md_field_list( table_fields )
                    table_fields = []

                self.print_md_items( field.items )

        if table_fields:
            self.print_md_field_list( table_fields )

    #
    # formatting the index
    #
    def  index_enter( self ):
        print( self.md_index_header )
        self.index_items = {}

    def  index_name_enter( self, name ):
        block = self.identifiers[name]
        url   = self.make_block_url( block )
        self.index_items[name] = url

    def  index_exit( self ):
        # `block_index' already contains the sorted list of index names
        letter  = ''
        for bname in self.block_index:
            if letter != bname[0].upper():
                # print letter heading
                letter = bname[0].upper()
                print( '\n' + md_h3 + letter + '\n' )
            url   = self.index_items[bname]
            # display `foo[bar]' as `foo (bar)'
            bname = bname.replace( "[", " (" )
            bname = bname.replace( "]", ")"  )
            # normalize url
            url   = self.normalize_url( url )
            line  = ( '[' + bname + ']' + '(' + url + ')' + '  ' )
            print( line )

        # TODO Remove commented code once the above is ready
        # count   = len( self.block_index )
        # rows  = ( count + self.columns - 1 ) // self.columns

        # print( '<table class="index">' )
        # for r in range( rows ):
        #     line = "<tr>"
        #     for c in range( self.columns ):
        #         i = r + c * rows
        #         if i < count:
        #             bname = self.block_index[r + c * rows]
        #             url   = self.index_items[bname]
        #             # display `foo[bar]' as `foo (bar)'
        #             bname = bname.replace( "[", " (" )
        #             bname = bname.replace( "]", ")"  )
        #             # normalize url
        #             url = self.normalize_url( url )
        #             line  = ( line + '<td><a href="' + url + '">'
        #                       + bname + '</a></td>' )
        #         else:
        #             line = line + '<td></td>'
        #     line = line + "</tr>"
        #     print( line )

        # print( "</table>" )

        print( md_line_sep )
        print( self.time_footer )

        self.index_items = {}

    def  index_dump( self, index_filename = None ):
        if index_filename == None:
            index_filename = self.file_prefix + "index.md"

        Formatter.index_dump( self, index_filename )

    #
    # Formatting the table of contents and
    # config file for MkDocs.
    #
    def  toc_enter( self ):
        print( self.md_toc_header )
        print( "# Table of Contents" )

    def  toc_chapter_enter( self, chapter ):
        print( chapter_header + " ".join( chapter.title ) + md_newline )
        print( '<table class="toc">' )
        # add a chapter
        self.config.start_chapter( " ".join( chapter.title ) )

    def  toc_section_enter( self, section ):
        print( '<tr><td class="link">'
               + '<a href="'
               + self.make_section_url( section, code = True ) + '">'
               + section.title + '</a></td><td class="desc">' )
        print( self.print_md_para( section.abstract, in_html = True ) )
        # add section to chapter
        self.config.add_chapter_page( section.title,
                                      self.make_section_url( section ) )

    def  toc_section_exit( self, section ):
        print( "</td></tr>" )

    def  toc_chapter_exit( self, chapter ):
        print( "</table>" )
        #print( chapter_footer )
        # End the chapter
        self.config.end_chapter()

    def  toc_index( self, index_filename ):
        print( chapter_header
               + '[Global Index](' + index_filename + ')'
            )

    def  toc_exit( self ):
        print( md_line_sep )
        print( self.time_footer )
        # Build and flush MkDocs config
        self.config.build_config()

    def  toc_dump( self, toc_filename = None, index_filename = None ):
        if toc_filename == None:
            toc_filename = self.file_prefix + "toc.md"

        if index_filename == None:
            index_filename = self.file_prefix + "index.md"

        Formatter.toc_dump( self, toc_filename, index_filename )

    #
    # formatting sections
    #
    def  section_enter( self, section ):
        if section.chapter:
            print( md_header_1 + md_header_2
                   +  self.make_chapter_url( section.chapter.title )
                   +  md_crumbs_sep + section.title
                   + md_line_sep )
        else:
            # this should never happen!
            log.warn( "No chapter name for Section '%s'.", section.title )

        # Print section title
        print( md_h1 + section.title )

        # print section synopsis
        print( section_synopsis_header )
        #print( section_synopsis_footer )

        #print( description_header )
        print( self.make_md_items( section.description ) )
        print( description_footer )

    def  block_enter( self, block ):
        
        # place anchor if needed
        if block.name:
            url = block.name
            # display `foo[bar]' as `foo'
            name = re.sub( r'\[.*\]', '', block.name )
            # normalize url
            url = self.normalize_url( url )
            print( md_h2 + name + md_newline )

        # dump the block C source lines now
        if block.code:
            header = ''
            for f in self.headers.keys():
                header_filename = os.path.normpath( block.source.filename )
                if header_filename.find( os.path.normpath( f ) ) >= 0:
                    header = self.headers[f] + ' (' + f + ')'
                    break

            # Warn if header macro not found
            # if not header:
            #     log.warn(
            #     "No header macro for"
            #     " '%s'.", block.source.filename )

            if header:
                print( 'Defined in ' + header + '.' )

            print( source_header )
            for l in block.code:
                print( self.source_quote( l, block.name ) )
            print( source_footer )

    def  markup_enter( self, markup, block ):
        if markup.tag == "description":
            print( description_header )
        else:
            print( md_h4 + markup.tag + md_h4_inter )

        self.print_md_markup( markup )

    def  markup_exit( self, markup, block ):
        if markup.tag == "description":
            print( description_footer )
        else:
            print( "" )

    def  block_exit( self, block ):
        print( md_hr )

    def  section_exit( self, section ):
        pass

    def  section_dump_all( self ):
        log.debug( "Building markdown pages for sections." )
        for section in self.sections:
            self.section_dump( section,
                               self.file_prefix + section.name + '.md' )

# eof
