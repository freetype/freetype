#
#  installation instructions for Unix systems
#  this file is FreeType-specific
#

  # Unix installation and deinstallation targets.
  install: $(PROJECT_LIBRARY)
	  $(MKINSTALLDIRS) $(libdir)                       \
                           $(includedir)/freetype/config   \
                           $(includedir)/freetype/internal \
                           $(includedir)/freetype/cache
	  $(LIBTOOL) --mode=install $(INSTALL) $(PROJECT_LIBRARY) $(libdir)
	  -for P in $(PUBLIC_H) ; do                     \
            $(INSTALL_DATA) $$P $(includedir)/freetype ; \
          done
	  -for P in $(BASE_H) ; do                                \
            $(INSTALL_DATA) $$P $(includedir)/freetype/internal ; \
          done
	  -for P in $(CONFIG_H) ; do                            \
            $(INSTALL_DATA) $$P $(includedir)/freetype/config ; \
          done
	  -for P in $(BASE_H) ; do                                \
            $(INSTALL_DATA) $$P $(includedir)/freetype/cache ; \
          done

  uninstall:
	  -$(LIBTOOL) --mode=uninstall $(RM) $(libdir)/$(LIBRARY).$A
	  -$(DELETE) $(includedir)/freetype/config/*
	  -$(DELDIR) $(includedir)/freetype/config
	  -$(DELETE) $(includedir)/freetype/internal/*
	  -$(DELDIR) $(includedir)/freetype/internal
	  -$(DELETE) $(includedir)/freetype/*
	  -$(DELDIR) $(includedir)/freetype


  # Unix cleaning and distclean rules.
  #
  clean_project_unix:
	  -$(DELETE) $(BASE_OBJECTS) $(OBJ_M) $(OBJ_S)
	  -$(DELETE) $(patsubst %.$O,%.$(SO),$(BASE_OBJECTS) $(OBJ_M) $(OBJ_S)) \
                     $(CLEAN)

  distclean_project_unix: clean_project_unix
	  -$(DELETE) $(PROJECT_LIBRARY)
	  -$(DELETE) $(OBJ_DIR)/.libs/*
	  -$(DELDIR) $(OBJ_DIR)/.libs
	  -$(DELETE) *.orig *~ core *.core $(DISTCLEAN)

endif



# EOF
