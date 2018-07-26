#
#  siteconfig.py
#
#    Build site configuration and write to mkdocs.yml.
#
#  Copyright 2018 by
#  Nikhil Ramakrishnan.
#
#  This file is part of the FreeType project, and may only be used,
#  modified, and distributed under the terms of the FreeType project
#  license, LICENSE.TXT.  By continuing to use, modify, or distribute
#  this file you indicate that you have read the license and
#  understand and accept it fully.

"""Module to generate Mkdocs config.

This module contains routines to generate the configuration file
`mkdocs.yml` required by Mkdocs to build static HTML documentation
from markdown.

More information can be found at:

<https://www.mkdocs.org/user-guide/configuration/>
"""

from __future__ import print_function

import datetime
import logging

import yaml

import utils

log = logging.getLogger( __name__ )

# Config file name
config_filename = "mkdocs.yml"

# Docs directory and site directory
docs_dir = "markdown"
site_dir = "site"

# Basic site configuration default values
site_name        = "FreeType API Reference"
site_description = "API Reference documentation for FreeType"
site_author      = "FreeType Contributors"
use_dir_url      = False

# Theme configuration default values
theme_conf             = {}
theme_conf['name']     = "material"
theme_conf['logo']     = "images/favico.ico"
theme_conf['language'] = "en"
theme_conf['favicon']  = "images/favico.ico"
theme_conf['palette']  = {}
theme_conf['font']     = {}

# Theme palette
theme_conf['palette']['primary'] = "green"
theme_conf['palette']['accent']  = "green"

# Theme fonts
theme_conf['font']['text'] = "Noto Serif"
theme_conf['font']['code'] = "Roboto Mono"

# Markdown extensions
md_extensions = '''\
markdown_extensions:
  - toc:
      permalink: true
  - pymdownx.superfences:
      disable_indented_code_blocks: true
  - codehilite:
      guess_lang: false
  - pymdownx.betterem:
      smart_enable: all
  - pymdownx.magiclink
  - pymdownx.smartsymbols
'''

# Extra scripts
extra_scripts = '''\
extra_css:
  - 'stylesheets/extra.css'

extra_javascript:
  - 'javascripts/extra.js'
'''

# Other config
year     = datetime.datetime.utcnow().year
var_dict = { 'year': year }
other_config = '''\
copyright: Copyright {year} \
<a href = "https://www.freetype.org/license.html">\
The FreeType Project</a>.
'''
other_config = other_config.format( **var_dict )

def add_config( yml_string, config_name ):
    config = None
    try:
        config = yaml.safe_load( yml_string )
    except yaml.scanner.ScannerError:
        log.warn( "Malformed '%s' config, ignoring.", config_name )
    return config

def build_extras():
    # Parse all configurations and save as Python objects
    global md_extensions, yml_extra, yml_other
    md_extensions = add_config( md_extensions, "markdown_extensions" )
    yml_extra     = add_config( extra_scripts, "extra scripts" )
    yml_other     = add_config( other_config, "other" )


class  Chapter( object ):
    def __init__( self, title ):
        self.title = title
        self.pages = []

    def add_page( self, section_title, filename ):
        """Add a page to the chapter."""
        cur_page = {}
        cur_page[section_title] = filename
        self.pages.append( cur_page )

    def get_pages( self ):
        """Get dict of pages in the chapter."""
        conf = {}
        conf[self.title] = self.pages
        return conf


class  SiteConfig( object ):
    """Site configuration generator class.

    This class is used to generate site configuration based on supplied
    and default values.
    """
    def __init__( self ):
        self.site_config   = {}
        self.pages         = []
        self.chapter       = None
        self.sections      = []
        self.md_extensions = []

        # Set configurations
        self.site_name   = site_name
        self.site_desc   = site_description
        self.site_author = site_author
        self.docs_dir    = docs_dir
        self.site_dir    = site_dir
        self.theme_conf  = theme_conf
        self.use_dir_url = use_dir_url

    def set_site_info( self, name, description = None, author = None ):
        """Set the basic site information."""
        if name:
            self.site_name = name
        else:
            # Site name is required, throw warning and revert to default
            log.warn( "Site name not specified, reverting to default." )

        if description:
            self.site_desc = description
        if author:
            self.site_author = author

    def add_single_page( self, section_title, filename ):
        """Add a single page to the list of pages."""
        cur_page = {}
        cur_page[section_title] = filename
        self.pages.append( cur_page )

    def add_chapter_page( self, section_title, filename ):
        """Add a page to a chapter.

        Chapter must be set first using `start_chapter()` If not set,
        `add_single_page()` will be called internally.
        """
        if self.chapter:
            self.chapter.add_page( section_title, filename )
        else:
            log.warn( "Section '%s' added without starting chapter.",
                      section_title )
            self.add_single_page( section_title, filename )

    def start_chapter( self, chap ):
        """Start a chapter."""
        if self.chapter:
            self.end_chapter()

        self.chapter = Chapter( chap )

    def end_chapter( self ):
        """Explicitly end a chapter."""
        if self.chapter:
            chap_pages = self.chapter.get_pages()
            self.pages.append( chap_pages )
            self.chapter = None

    def build_site_config( self ):
        """Add basic Project information to config."""
        self.site_config['site_name'] = self.site_name
        if site_description:
            self.site_config['site_description'] = self.site_desc
        if site_author:
            self.site_config['site_author'] = self.site_author
        if docs_dir:
            self.site_config['docs_dir'] = self.docs_dir
        if site_dir:
            self.site_config['site_dir'] = self.site_dir
        if use_dir_url is not None:
            self.site_config['use_directory_urls'] = self.use_dir_url

    def build_theme_config( self ):
        # internal: build theme config
        if theme_conf != {}:
            self.site_config['theme'] = self.theme_conf

    def build_pages( self ):
        # internal: build pages config
        if self.pages != []:
            self.site_config['pages'] = self.pages

    def populate_config( self, data ):
        # internal: Add a given not None object to site_config
        if data:
            self.site_config.update( data )

    def write_config( self, name ):
        """Write all values in site_config to output stream."""
        if self.site_config != {}:
            print( "# " + name )
            print( yaml.dump( self.site_config, default_flow_style=False ) )
            self.site_config.clear()

    def write_config_order( self, name, order ):
        """Write all values in site_config to output stream in order."""
        if self.site_config != {}:
            print( "# " + name )
            for key in order:
                if key in self.site_config:
                    temp_config = {}
                    temp_config[key] = self.site_config[key]
                    print( yaml.dump( temp_config, default_flow_style=False ).rstrip() )
                    self.site_config.pop( key, None )

            if self.site_config != {}:
                # Print remaining values
                print( yaml.dump( self.site_config, default_flow_style=False ).rstrip() )

            # print an empty line
            print()
            self.site_config.clear()

    def build_config( self ):
        """Build the YAML configuration."""
        # End chapter if started
        self.end_chapter()

        # Open yml file
        output = utils.open_output( config_filename, config = True )

        # Build basic site info
        self.build_site_config()
        order = ['site_name', 'site_author', 'docs_dir', 'site_dir']
        self.write_config_order( "Project information", order )

        # Build theme configuration
        self.build_theme_config()
        self.write_config( "Configuration" )

        # Build pages
        self.build_pages()
        self.write_config( "Pages" )

        # Build extra scripts
        build_extras()

        # Add extra CSS and Javascript
        self.populate_config( yml_extra )
        self.write_config( "Customization" )

        # Add Markdown extensions
        self.populate_config( md_extensions )
        self.write_config( "Extensions" )

        # Add other options
        self.populate_config( yml_other )
        self.write_config( "Other Options" )

        # Close the file
        utils.close_output( output )

# eof
