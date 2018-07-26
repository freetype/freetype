#
#  formatter.py
#
#    Convert parsed content blocks to a structured document (library file).
#
#  Copyright 2002-2018 by
#  David Turner.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

"""Base formatter class.

The purpose of this module is to convert a content processor's data into
specific documents (i.e., table of contents, global index, and individual
API reference indices).

You need to sub-class it to output anything sensible.  For example, the
module `tomarkdown` contains the definition of the `MdFormatter' sub-class
to output Markdown.
"""

import logging

import utils

log = logging.getLogger( __name__ )

################################################################
##
##  FORMATTER CLASS
##
class  Formatter( object ):

    def  __init__( self, processor ):
        self.processor   = processor
        self.identifiers = {}
        self.chapters    = processor.chapters
        self.sections    = processor.sections.values()
        self.block_index = []

        # store all blocks in a dictionary
        self.blocks = []
        for section in self.sections:
            for block in section.blocks.values():
                self.add_identifier( block.name, block )

                # add enumeration values to the index, since this is useful
                for markup in block.markups:
                    if markup.tag == 'values':
                        for field in markup.fields:
                            self.add_identifier( field.name, block )

        self.block_index = self.identifiers.keys()
        self.block_index = sorted( self.block_index, key = str.lower )

        # also add section names to dictionary (without making them appear
        # in the index)
        for section in self.sections:
            self.add_identifier( section.name, section )

    def  add_identifier( self, name, block ):
        if name in self.identifiers:
            # duplicate name!
            log.warn( "Duplicate definition for"
                      " '%s' in %s, "
                      "previous definition in %s",
                      name,
                      block.location(),
                      self.identifiers[name].location() )
        else:
            self.identifiers[name] = block

    #
    # formatting the table of contents
    #
    def  toc_enter( self ):
        pass

    def  toc_chapter_enter( self, chapter ):
        pass

    def  toc_section_enter( self, section ):
        pass

    def  toc_section_exit( self, section ):
        pass

    def  toc_chapter_exit( self, chapter ):
        pass

    def  toc_index( self, index_filename ):
        pass

    def  toc_exit( self ):
        pass

    def  toc_dump( self, toc_filename = None, index_filename = None ):
        output = None
        if toc_filename:
            output = utils.open_output( toc_filename )

        log.debug( "Building table of contents in %s.", toc_filename )
        self.toc_enter()

        for chap in self.processor.chapters:

            self.toc_chapter_enter( chap )

            for section in chap.sections:
                self.toc_section_enter( section )
                self.toc_section_exit( section )

            self.toc_chapter_exit( chap )

        self.toc_index( index_filename )

        self.toc_exit()

        if output:
            utils.close_output( output )

    #
    # formatting the index
    #
    def  index_enter( self ):
        pass

    def  index_name_enter( self, name ):
        pass

    def  index_name_exit( self, name ):
        pass

    def  index_exit( self ):
        pass

    def  index_dump( self, index_filename = None ):
        output = None
        if index_filename:
            output = utils.open_output( index_filename )

        log.debug("Building index in %s.", index_filename )
        self.index_enter()

        for name in self.block_index:
            self.index_name_enter( name )
            self.index_name_exit( name )

        self.index_exit()

        if output:
            utils.close_output( output )

    #
    # formatting a section
    #
    def  section_enter( self, section ):
        pass

    def  block_enter( self, block ):
        pass

    def  markup_enter( self, markup, block ):
        pass

    def  field_enter( self, field, markup = None, block = None ):
        pass

    def  field_exit( self, field, markup = None, block = None ):
        pass

    def  markup_exit( self, markup, block ):
        pass

    def  block_exit( self, block ):
        pass

    def  section_exit( self, section ):
        pass

    def  section_dump( self, section, section_filename = None ):
        output = None
        log.debug( "Building page %s.", section_filename )
        if section_filename:
            output = utils.open_output( section_filename )

        self.section_enter( section )

        for name in section.block_names:
            skip_entry = 0
            try:
                block = self.identifiers[name]
                # `block_names' can contain field names also,
                # which we filter out
                for markup in block.markups:
                    if markup.tag == 'values':
                        for field in markup.fields:
                            if field.name == name:
                                skip_entry = 1
            except Exception:
                skip_entry = 1   # this happens e.g. for `/empty/' entries

            if skip_entry:
                continue

            self.block_enter( block )

            for markup in block.markups[1:]:   # always ignore first markup!
                self.markup_enter( markup, block )

                for field in markup.fields:
                    self.field_enter( field, markup, block )
                    self.field_exit( field, markup, block )

                self.markup_exit( markup, block )

            self.block_exit( block )

        self.section_exit( section )

        if output:
            utils.close_output( output )

    def  section_dump_all( self ):
        log.debug( "Building markdown pages for sections." )
        for section in self.sections:
            self.section_dump( section )

# eof
