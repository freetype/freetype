/***************************************************************************/
/*                                                                         */
/*  otload.c                                                               */
/*                                                                         */
/*    OpenType Layout loader (body).                                       */
/*                                                                         */
/*  Copyright 1996-1999 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used        */
/*  modified and distributed under the terms of the FreeType project       */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <otlayout.h>
#include <otload.h>

#include <tterrors.h>


  /***************************
   * Script related functions
   ***************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Script_List                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Script list.                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target script list.                                  */
  /*    system :: The current system object.                               */
  /*                                                                       */
  LOCAL_FUNC
  void  OTL_Free_Script_List( OTL_Script_List*  list,
                              FT_System         system )
  {
    if ( list )
    {
      if ( list->scripts )
      {
        OTL_Script*  script       = list->scripts;
        OTL_Script*  script_limit = script + list->num_scripts;


        for ( ; script < script_limit; script++ )
        {
          if ( script->langsys )
          {
            OTL_LangSys*  langsys       = script->langsys;
            OTL_LangSys*  langsys_limit = langsys + script->num_langsys;


            for ( ; langsys < langsys_limit; langsys++ )
            {
              FREE( langsys->feature_indices );
              langsys->num_feature_indices = 0;
            }
            FREE( script->langsys );
          }
          script->langsys_default = NULL;
          script->num_langsys     = 0;
        }
        FREE( list->scripts );
      }
      list->num_scripts = 0;
    }
  }


  static
  TT_Error  Load_OTL_LangSys_List( OTL_Script*  script,
                                   FT_Stream    stream,
                                   TT_ULong     default_offset )
  {
    FT_System  system = stream->system;
    TT_Error   error;

    TT_UShort     n, count;
    OTL_LangSys*  langsys;


    /* read the langsys tags and offsets */
    {
      count   = script->num_langsys;
      langsys = script->langsys;

      if ( ACCESS_Frame( 6L * count ) )
        goto Exit;

      for ( n = 0; n < count; n++, langsys++ )
      {
        TT_ULong  offset;


        langsys->lang_tag = GET_ULong();
        offset            = GET_UShort();

        if ( langsys->lang_offset == default_offset )
          script->langsys_default = langsys;

        langsys->lang_offset = offset + script->script_offset;
      }

      FORGET_Frame();
    }

    /* now read each langsys record */
    {
      count   = script->num_langsys;
      langsys = script->langsys;

      for ( n = 0; n < count; n++, langsys++ )
      {
        TT_UShort  num_feature_indices, i;


        if ( FILE_Seek( langsys->lang_offset ) ||
             ACCESS_Frame( 8L )                )
          goto Exit;

        langsys->lookup_order        = GET_ULong();
        langsys->req_feature_index   = GET_UShort();
        langsys->num_feature_indices = GET_UShort();

        FORGET_Frame();

        num_feature_indices = langsys->num_feature_indices;

        if ( ALLOC_ARRAY ( langsys->feature_indices,
                           num_feature_indices, TT_UShort ) ||
             ACCESS_Frame( num_feature_indices * 2L )       )
          goto Exit;

        for ( i = 0; i < num_feature_indices; i++ )
          langsys->feature_indices[i] = GET_UShort();

        FORGET_Frame();
      }
    }

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Script_List                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Script List from a font resource.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target script list.                                  */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  OTL_Load_Script_List( OTL_Script_List*  list,
                                  FT_Stream         stream )
  {
    FT_System  system = stream->system;
    TT_Error   error;

    TT_ULong     start_pos;
    TT_UShort    num_scripts;
    OTL_Script*  scripts;


    start_pos = FILE_Pos();

    if ( READ_UShort( list->num_scripts ) )
      goto Exit;

    num_scripts = list->num_scripts;

    /* Allocate the scripts table, read their tags and offsets */
    {
      TT_UShort  n;


      if ( ALLOC_ARRAY( list->scripts, num_scripts, OTL_Script ) ||
           ACCESS_Frame( num_scripts * 6L )                      )
        goto Exit;

      scripts = list->scripts;

      for ( n = 0; n < num_scripts; n++ )
      {
        scripts[n].script_tag    = GET_ULong();
        scripts[n].script_offset = GET_UShort() + start_pos;
      }

      FORGET_Frame();
    }

    /* now read each script in the table */
    {
      TT_UShort    n;
      OTL_Script*  script = scripts;


      for ( n = num_scripts; n > 0; n--, script++ )
      {
        TT_ULong  default_langsys_offset;


        if ( FILE_Seek  ( script->script_offset )            ||
             READ_ULong ( default_langsys_offset )           ||
             READ_UShort( script->num_langsys )              ||
             ALLOC_ARRAY( script->langsys,
                          script->num_langsys, OTL_LangSys ) )
          goto Exit;

        /* read the corresponding langsys list */
        error = Load_OTL_LangSys_List(
                     script,
                     stream,
                     default_langsys_offset + script->script_offset );
        if ( error )
          goto Exit;
      }
    }

  Exit:
    if ( error )
      OTL_Free_Script_List( list, system );

    return error;
  }


  /*********************************
   * Feature List related functions
   *********************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Features_List                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Features list.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target feature list.                                 */
  /*    system :: The current system object.                               */
  /*                                                                       */
  LOCAL_FUNC
  void  OTL_Free_Features_List( OTL_Feature_List*  list,
                                FT_System          system )
  {
    if ( list )
    {
      if ( list->features )
      {
        OTL_Feature*  feature       = list->features;
        OTL_Feature*  feature_limit = feature + list->num_features;


        for ( ; feature < feature_limit; feature++ )
        {
          FREE( feature->lookups );
          feature->num_lookups = 0;
        }

        FREE( list->features );
      }
      list->num_features = 0;
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Feature_List                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Feature List from a font resource.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target feature list.                                 */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  OTL_Load_Feature_List( OTL_Feature_List*  list,
                                   FT_Stream          stream )
  {
    FT_System  system = stream->system;
    TT_Error   error;
    TT_ULong   start_pos;
    TT_UShort  num_features, n;


    start_pos = FILE_Pos();

    if ( READ_UShort( num_features ) )
      goto Exit;

    /* allocate the features array and read their tag and offset */
    {
      OTL_Feature*  feature;
      OTL_Feature*  feature_limit;


      if ( ALLOC_ARRAY ( list->features, num_features, OTL_Feature ) ||
           ACCESS_Frame( num_features * 6L )                         )
        goto Exit;

      list->num_features = num_features;
      feature            = list->features;
      feature_limit      = feature + num_features;

      for ( ; feature < feature_limit; feature++ )
      {
        feature->feature_tag    = GET_ULong();
        feature->feature_offset = GET_UShort() + start_pos;
      }

      FORGET_Frame();
    }

    /* now read each feature */
    {
      OTL_Feature*  feature;
      OTL_Feature*  feature_limit;


      feature            = list->features;
      feature_limit      = feature + num_features;

      for ( ; feature < feature_limit; feature++ )
      {
        TT_UShort   num_lookups;
        TT_UShort*  lookup;
        TT_UShort*  lookup_limit;


        if ( FILE_Seek   ( feature->feature_offset  ) ||
             READ_ULong  ( feature->feature_params )  ||
             READ_UShort ( num_lookups )              ||
             ALLOC_ARRAY ( feature->lookups,
                           num_lookups, TT_UShort )   ||
             ACCESS_Frame( num_lookups * 2L )         )
          goto Exit;

        feature->num_lookups = num_lookups;
        lookup               = feature->lookups;
        lookup_limit         = lookup + num_lookups;

        for ( ; lookup < lookup_limit; lookup++ )
          lookup[0] = GET_UShort();

        FORGET_Frame();
      }
    }

  Exit:
    if ( error )
      OTL_Free_Feature_List( list, system );

    return error;
  }


  /********************************
   * Lookup List related functions
   ********************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Iterate_Lookup_List                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Scans an OpenType Lookup List.  This can be used to load the       */
  /*    Lookup sub-tables in a GSUB or GPOS loader.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list     :: The source list.                                       */
  /*    iterator :: The iterator -- a function which is called on each     */
  /*                element of the list.                                   */
  /*    closure  :: User-specified data which is passed to each iterator   */
  /*                with the lookup element pointer.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    If one iterator call returns a non-zero `result', the list parsing */
  /*    is aborted and the value is returned to the caller.  Otherwise,    */
  /*    the function returns 0 when the list has been parsed completely.   */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  OTL_Iterate_Lookup_List( OTL_Lookup_List*     list,
                                     OTL_Lookup_Iterator  iterator,
                                     void*                closure )
  {
    int  result = 0;


    if ( list->lookups )
    {
      OTL_Lookup*  lookup = list->lookups;
      OTL_Lookup*  limit  = lookup + list->num_lookups;


      for ( ; lookup < limit; lookup++ )
      {
        result = iterator( lookup, closure );
        if ( result )
          break;
      }
    }

    return 0;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Lookup_List                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Lookup list.  Uses a destructor called   */
  /*    to destroy the Lookup sub-tables.                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list       :: The target lookup list.                              */
  /*    system     :: The current system object.                           */
  /*    destructor :: A destructor function called on each lookup element. */
  /*                  Can be used to destroy sub-tables.  Ignored if NULL. */
  /*                                                                       */
  LOCAL_FUNC
  void  OTL_Free_Lookup_List( OTL_Lookup_List*       list,
                              FT_System              system,
                              OTL_Lookup_Destructor  destroy )
  {
    if ( list )
    {
      if ( list->lookups )
      {
        OTL_Lookup*  lookup = list->lookups;
        OTL_Lookup*  limit  = lookup + list->num_lookups;


        for ( ; lookup < limit; lookup++ )
        {
          if ( destroy )
            destroy( lookup, system );

          FREE( lookup->subtable_offsets );
          lookup->num_subtables = 0;
        }

        FREE( list->lookups );
      }
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Lookup_List                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Lookup List from a font resource.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    list   :: The target lookup list.                                  */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function does NOT load the lookup sub-tables.  Instead, it    */
  /*    stores the file offsets of the particular table in each lookup     */
  /*    element.  It is up to the caller to load these sub-tables.  This   */
  /*    can be done more easily with OTL_Iterate_Lookup_List().            */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  OTL_Load_Lookup_List( OTL_Lookup_List*  list,
                                  FT_Stream         stream )
  {
    FT_System  system = stream->system;
    TT_Error   error;

    TT_UShort  num_lookups;
    TT_ULong   start_pos;


    start_pos = GET_ULong();

    if ( READ_UShort( num_lookups ) )
      goto Exit;

    /* allocate the lookups array and read their tags and offset */
    {
      TT_UShort  n;


      if ( ALLOC_ARRAY ( list->lookups, num_lookups, OTL_Lookup ) ||
           ACCESS_Frame( num_lookups * 2L )                       )
        goto Exit;

      list->num_lookups = num_lookups;
      for ( n = 0; n < num_lookups; n++ )
        list->lookups[n].lookup_offset = start_pos + GET_UShort();

      FORGET_Frame();
    }

    /* now read each lookup table                               */
    /* NOTE that we don't load the sub-tables here, but simply  */
    /* store their file offsets in the `subtable_offsets' array */
    {
      OTL_Lookup*  lookup       = list->lookups;
      OTL_Lookup*  lookup_limit = lookup + num_lookups;


      for ( ; lookup < lookup_limit; lookup++ )
      {
        TT_UShort   n, num_subtables;
        TT_ULong*  offsets;


        if ( FILE_Seek   ( lookup->lookup_offset ) ||
             ACCESS_Frame( 6L )                    )
          goto Exit;

        lookup->lookup_type   = GET_UShort();
        lookup->lookup_flag   = GET_UShort();
        lookup->num_subtables = GET_UShort();

        num_subtables = lookup->num_subtables;

        FORGET_Frame();

        if ( ALLOC_ARRAY ( lookup->subtable_offsets,
                           num_subtables, TT_ULong ) ||
             ACCESS_Frame( num_subtables * 2L )      )
          goto Exit;

        offsets = lookup->subtable_offsets;
        for ( n = 0; n < num_subtables; n++ )
          offsets[n] = lookup->lookup_offset + GET_UShort();

        FORGET_Frame();
      }
    }

  Exit:
    if ( error )
      OTL_Free_Lookup_List( list, system, 0 );

    return error;
  }


  /* generic sub-table freeing and loading */

  static
  void  Free_SubTable( OTL_SubTable*  subtable,
                       FT_System      system )
  {
    if ( subtable )
    {
      switch ( subtable->format )
      {
      case 1:
        {
          OTL_SubTable1*  st = &subtable->set.format1;


          FREE( st->indices );
          st->num_indices = 0;
          FREE( st );
        }
        break;

      case 2:
        {
          OTL_SubTable2*  st = &subtable->set.format2;


          FREE( st->ranges );
          st->num_ranges = 0;
          FREE( st );
        }
        break;

      default:
        break;
      }

      FREE( subtable );
    }
  }


  static
  TT_Error  Load_SubTable( OTL_SubTable*  subtable,
                           FT_Stream      stream )
  {
    FT_System  system = stream->system;
    TT_Error   error;


    if ( READ_UShort( subtable->format ) )
      goto Exit;

    switch ( subtable->format )
    {
    case 1:
      {
        OTL_SubTable1*  st = &subtable->set.format1;
        TT_UShort       num_indices, n;


        if ( READ_UShort ( num_indices )                         ||
             ALLOC_ARRAY ( st->indices, num_indices, TT_UShort ) ||
             ACCESS_Frame( num_indices * 2L )                    )
          goto Exit;

        st->num_indices = num_indices;
        for ( n = 0; n < num_indices; n++ )
          st->indices[n] = GET_UShort();

        FORGET_Frame();
      }

    case 2:
      {
        OTL_SubTable2*      st = &subtable->set.format2;
        TT_UShort           num_ranges, n;
        OTL_SubTable2_Rec*  range;


        if ( READ_UShort( num_ranges )                                ||
             ALLOC_ARRAY( st->ranges, num_ranges, OTL_SubTable2_Rec ) ||
             ACCESS_Frame( num_ranges * 6L )                          )
          goto Exit;

        st->num_ranges = num_ranges;
        range          = st->ranges;
        for ( ; num_ranges > 0; num_ranges--, range++ )
        {
          range->start = GET_UShort();
          range->end   = GET_UShort();
          range->data  = GET_UShort();
        }

        FORGET_Frame();
      }
      break;

    default:
      error = TT_Err_Invalid_File_Format;
    }

  Exit:
    if ( error )
      Free_SubTable( subtable, system );

    return error;
  }


  /*****************************
   * Coverage related functions
   *****************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Coverage                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Coverage table.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    coverage :: The target coverage.                                   */
  /*    system   :: The current system object.                             */
  /*                                                                       */
  LOCAL_FUNC
  void  OTL_Free_Coverage( OTL_Coverage*  coverage,
                           FT_System      system )
  {
    Free_SubTable( (OTL_SubTable*)coverage, system );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Coverage                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Coverage table from a font resource.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    coverage :: The target coverage.                                   */
  /*    stream   :: The input stream.                                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  OTL_Load_Coverage( OTL_Coverage*  coverage,
                               FT_Stream      stream )
  {
    return Load_SubTable( (OTL_SubTable*)coverage, stream );
  }


  /*************************************
   * Class Definition related functions
   *************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Class_Def                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Class Definition table.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    class_def :: The target class definition.                          */
  /*    system    :: The current system object.                            */
  /*                                                                       */
  LOCAL_FUNC
  void  OTL_Free_Class_Def( OTL_Class_Def*  class_def,
                            FT_System       system )
  {
    Free_SubTable( (OTL_SubTable*)class_def, system );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Class_Def                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Class Definition table from a resource.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    class_def :: The target class definition.                          */
  /*    stream    :: The input stream.                                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  OTL_Load_Class_Def( OTL_Class_Def*  class_def,
                                FT_Stream       stream )
  {
    return OTL_Load_SubTable( (OTL_SubTable*)class_def, stream );
  }


  /*************************************
   * Device related functions
   *************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Free_Device                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Releases a given OpenType Layout Device table.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    device  :: The target device table.                                */
  /*    system  :: The current system object.                              */
  /*                                                                       */
  LOCAL_FUNC
  void  OTL_Free_Device( OTL_Device*  device,
                         FT_System    system )
  {
    if ( device )
    {
      FREE( device->delta_values );
      FREE( device );
    }
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    OTL_Load_Device                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads an OpenType Device table from a font resource.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    device :: The target device table.                                 */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  OTL_Load_Device( OTL_Device*  device,
                             FT_Stream    stream )
  {
    FT_System   system = stream->system;
    TT_Error    error;
    TT_UShort*  deltas;
    TT_UShort   num_deltas, num_values;


    if ( ACCESS_Frame( 6L ) )
      goto Exit;

    device->start_size   = GET_UShort();
    device->end_size     = GET_UShort();
    device->delta_format = GET_UShort();

    FORGET_Frame();

    num_deltas = device->end_size - device->start_size + 1;

    switch ( device->delta_format )
    {
    case 1:
      num_values = ( num_deltas + 7 ) >> 3;
      break;

    case 2:
      num_values = ( num_deltas + 3 ) >> 2;
      break;

    case 3:
      num_values = ( num_deltas + 1 ) >> 1;
      break;

    default:
      error = TT_Err_Invalid_File_Format;
      goto Exit;
    }

    if ( ALLOC_ARRAY( deltas, num_values, TT_UShort ) )
      goto Exit;

    if ( !ACCESS_Frame( num_values * 2L ) )
    {
      TT_UShort  n;


      for ( n = 0; n < num_values; n++ )
        deltas[n] = GET_UShort();

      FORGET_Frame();

      device->delta_values = deltas;
    }
    else
      FREE( deltas );

  Exit:
    return error;
  }


/* END */
