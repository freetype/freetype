/***************************************************************************/
/*                                                                         */
/*  ttload.c                                                               */
/*                                                                         */
/*    TrueType tables loader (body).                                       */
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
/*                                                                         */
/*  WARNING: This file should not be compiled directly; it is meant to be  */
/*           included in the source of several font drivers (i.e., the TTF */
/*           and OTF drivers).                                             */
/*                                                                         */
/***************************************************************************/


#include <ftdebug.h>

#include <ttload.h>
#include <tttags.h>
#include <ttcmap.h>


/* required by the tracing mode */
#undef  FT_COMPONENT
#define FT_COMPONENT      trace_ttload


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_LookUp_Table                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Looks for a TrueType table by name.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A face object handle.                                      */
  /*    tag  :: The  searched tag.                                         */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Index of table if found, -1 otherwise.                             */
  /*                                                                       */
  EXPORT_FUNC
  FT_Long  TT_LookUp_Table( TT_Face   face,
                            TT_ULong  tag  )
  {
    TT_Long  i, found;


    FT_TRACE4(( "TT_LookUp_Table( %08lx, %c%c%c%c )\n",
                  (TT_Long)face,
                  (TT_Char)(tag >> 24),
                  (TT_Char)(tag >> 16),
                  (TT_Char)(tag >> 8),
                  (TT_Char)(tag) ));

    found = -1;
    for ( i = 0; i < face->num_tables; i++ )
      if ( face->dir_tables[i].Tag == tag )
      {
        found = i;
        break;
      }

    if ( found == -1 )
    {
      FT_TRACE4(( "    Could not find table!\n" ));
    }

    return found;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Collection                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the TTC table directory into face table.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A face object handle.                                    */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  static
  TT_Error  TT_Load_Collection( TT_Face    face,
                                FT_Stream  stream )
  {
    TT_Long    n;
    TT_Error   error;
    FT_Memory  memory = stream->memory;


    FT_TRACE3(( "TT_Load_Collection( %08lx )\n", (TT_Long)face ));

    if ( FILE_Seek   ( 0L )  ||
         ACCESS_Frame( 12L ) )
      goto Exit;

    face->ttc_header.Tag      = GET_Tag4();
    face->ttc_header.version  = GET_Long();
    face->ttc_header.DirCount = GET_Long(); /* see comment in tttypes.h */

    FORGET_Frame();

    if ( face->ttc_header.Tag != TTAG_ttcf )
    {
      face->ttc_header.Tag      = 0;
      face->ttc_header.version  = 0;
      face->ttc_header.DirCount = 0;

      face->ttc_header.TableDirectory = NULL;

      FT_TRACE3(( "skipped.\n" ));

      error = TT_Err_File_Is_Not_Collection;
      goto Exit;
    }

    if ( ALLOC_ARRAY( face->ttc_header.TableDirectory,
                      face->ttc_header.DirCount,
                      TT_ULong )                        ||
         ACCESS_Frame( face->ttc_header.DirCount * 4L ) )
      goto Exit;

    for ( n = 0; n < face->ttc_header.DirCount; n++ )
      face->ttc_header.TableDirectory[n] = GET_ULong();

    FORGET_Frame();

    FT_TRACE3(( "collections directory loaded.\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Directory                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the table directory into a face object.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face      :: A handle to the target face object.                   */
  /*    stream    :: The input stream.                                     */
  /*    faceIndex :: The index of the TrueType font, if we're opening a    */
  /*                 collection.                                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Directory( TT_Face    face,
                               FT_Stream  stream,
                               TT_Long    faceIndex )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;

    TT_UShort    n, limit;
    TT_TableDir  tableDir;

    TT_TableDirEntry*  entry;


    FT_TRACE2(( "TT_Load_Directory( %08lx, %ld )\n",
              (TT_Long)face, faceIndex ));

    error = TT_Load_Collection( face, stream );
    if ( error )
    {
      /* if this is a `traditional' error, exit now */
      if ( error != TT_Err_File_Is_Not_Collection )
        goto Exit;

      /* the file isn't a collection, exit if we're asking */
      /* for a collected font                              */

      /* Note that we don't exit during font format checking (i.e., */
      /* faceIndex is -1)                                           */
      if ( faceIndex > 0 )
        goto Exit;

      /* Now skip to the beginning of the file */
      if ( FILE_Seek( 0 ) )
        goto Exit;
    }
    else
    {
      /* The file is a collection. Check the font index */
      if ( faceIndex >= face->ttc_header.DirCount )
      {
        error = TT_Err_Bad_Argument;
        goto Exit;
      }

      /* if we're checking the font format, exit immediately */
      if ( faceIndex < 0 )
        goto Exit;

      /* select a TrueType font in the ttc file   */
      if ( FILE_Seek( face->ttc_header.TableDirectory[faceIndex] ) )
        goto Exit;
    }

    if ( ACCESS_Frame( 12L ) )
      goto Exit;

    tableDir.version   = GET_Long();
    tableDir.numTables = GET_UShort();

    tableDir.searchRange   = GET_UShort();
    tableDir.entrySelector = GET_UShort();
    tableDir.rangeShift    = GET_UShort();

    FORGET_Frame();

    FT_TRACE2(( "-- Tables count   : %12u\n",  tableDir.numTables ));
    FT_TRACE2(( "-- Format version : %08lx\n", tableDir.version ));

    /* Check that we have a `sfnt' format there                        */
    /* We must also be able to accept Mac/GX fonts, as well as OT ones */

    if ( tableDir.version != 0x00010000 &&
         tableDir.version != TTAG_true  &&
         tableDir.version != TTAG_OTTO  )
    {
      FT_TRACE2(( "[not a valid TTF or OTF font]" ));
      error = TT_Err_Invalid_File_Format;
      goto Exit;
    }

    /* if we're performing a font format check, exit immediately */
    /* with success                                              */
    if ( faceIndex < 0 )
      goto Exit;

    face->num_tables = tableDir.numTables;

    if ( ALLOC_ARRAY( face->dir_tables,
                      face->num_tables,
                      TT_TableDirEntry ) )
      goto Exit;

    if ( ACCESS_Frame( face->num_tables * 16L ) )
      goto Exit;

    limit = face->num_tables;
    entry = face->dir_tables;

    for ( n = 0; n < limit; n++ )
    {                      /* loop through the tables and get all entries */
      entry->Tag      = GET_Tag4();
      entry->CheckSum = GET_ULong();
      entry->Offset   = GET_Long();
      entry->Length   = GET_Long();

      FT_TRACE2(( "  %c%c%c%c  -  %08lx  -  %08lx\n",
                (TT_Char)(entry->Tag >> 24),
                (TT_Char)(entry->Tag >> 16),
                (TT_Char)(entry->Tag >> 8 ),
                (TT_Char)(entry->Tag),
                entry->Offset,
                entry->Length ));
      entry++;
    }

    FORGET_Frame();

    FT_TRACE2(( "Directory loaded\n\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Any                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads any font table into client memory.  Used by the              */
  /*    TT_Get_Font_Data() API function.                                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: The face object to look for.                             */
  /*                                                                       */
  /*    tag    :: The tag of table to load.  Use the value 0 if you  want  */
  /*              to access the whole font file, else set this parameter   */
  /*              to a valid TrueType table tag that you can forge with    */
  /*              the MAKE_TT_TAG macro.                                   */
  /*                                                                       */
  /*    offset :: The starting offset in the table (or the file if         */
  /*              tag == 0).                                               */
  /*                                                                       */
  /*    length :: The address of the decision variable:                    */
  /*                                                                       */
  /*                If length == NULL:                                     */
  /*                  Loads the whole table.  Returns an error if          */
  /*                  `offset' == 0!                                       */
  /*                                                                       */
  /*                If *length == 0:                                       */
  /*                  Exits immediately; returning the length of the given */
  /*                  table or of the font file, depending on the value of */
  /*                  `tag'.                                               */
  /*                                                                       */
  /*                If *length != 0:                                       */
  /*                  Loads the next `length' bytes of table or font,      */
  /*                  starting at offset `offset' (in table or font too).  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    buffer :: The address of target buffer.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Any( TT_Face   face,
                         TT_ULong  tag,
                         TT_Long   offset,
                         void*     buffer,
                         TT_Long*  length )
  {
    TT_Error   error;
    FT_Stream  stream;
    TT_Long    table;
    TT_ULong   size;


    if ( tag != 0 )
    {
      /* look for tag in font directory */
      table = TT_LookUp_Table( face, tag );
      if ( table < 0 )
      {
        error = TT_Err_Table_Missing;
        goto Exit;
      }

      offset += face->dir_tables[table].Offset;
      size    = face->dir_tables[table].Length;
    }
    else
    /* tag = 0 -- the use want to access the font file directly */
    {
      size = face->root.stream->size;
    }

    if ( length && *length == 0 )
    {
      *length = size;

      return TT_Err_Ok;
    }

    if ( length )
      size = *length;

    stream = face->root.stream;
    (void)FILE_Read_At( offset, buffer, size );

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Header                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the TrueType font header.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Header( TT_Face    face,
                            FT_Stream  stream )
  {
    TT_Error  error;

    TT_Long     table;
    TT_Header*  header;


    FT_TRACE2(( "Load_TT_Header( %08lx )\n", (TT_Long)face ));

    if ( ( table = TT_LookUp_Table( face, TTAG_head ) ) < 0 )
    {
      FT_TRACE0(( "Font Header is missing!\n" ));
      error = TT_Err_Header_Table_Missing;

      goto Exit;
    }

    if ( FILE_Seek( face->dir_tables[table].Offset ) ||
         ACCESS_Frame( 54L ) )
      goto Exit;

    header = &face->header;

    header->Table_Version = GET_ULong();
    header->Font_Revision = GET_ULong();

    header->CheckSum_Adjust = GET_Long();
    header->Magic_Number    = GET_Long();

    header->Flags        = GET_UShort();
    header->Units_Per_EM = GET_UShort();

    header->Created [0] = GET_Long();
    header->Created [1] = GET_Long();
    header->Modified[0] = GET_Long();
    header->Modified[1] = GET_Long();

    header->xMin = GET_Short();
    header->yMin = GET_Short();
    header->xMax = GET_Short();
    header->yMax = GET_Short();

    header->Mac_Style       = GET_UShort();
    header->Lowest_Rec_PPEM = GET_UShort();

    header->Font_Direction      = GET_Short();
    header->Index_To_Loc_Format = GET_Short();
    header->Glyph_Data_Format   = GET_Short();

    FORGET_Frame();

    FT_TRACE2(( "    Units per EM : %8u\n", header->Units_Per_EM ));
    FT_TRACE2(( "    IndexToLoc   : %8d\n", header->Index_To_Loc_Format ));
    FT_TRACE2(( "Font Header Loaded.\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_MaxProfile                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the maximum profile into a face object.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_MaxProfile( TT_Face    face,
                                FT_Stream  stream )
  {
    TT_Error  error;

    TT_Long         i;
    TT_MaxProfile*  maxProfile = &face->max_profile;


    FT_TRACE2(( "Load_TT_MaxProfile( %08lx )\n", (TT_Long)face ));

    if ( ( i = TT_LookUp_Table( face, TTAG_maxp ) ) < 0 )
    {
      error = TT_Err_Max_Profile_Missing;

      goto Exit;
    }

    if ( FILE_Seek( face->dir_tables[i].Offset ) ||
         ACCESS_Frame( 32L )                    )
      goto Exit;

    /* read frame data into face table */
    maxProfile->version               = GET_ULong();
    maxProfile->numGlyphs             = GET_UShort();

    maxProfile->maxPoints             = GET_UShort();
    maxProfile->maxContours           = GET_UShort();
    maxProfile->maxCompositePoints    = GET_UShort();
    maxProfile->maxCompositeContours  = GET_UShort();

    maxProfile->maxZones              = GET_UShort();
    maxProfile->maxTwilightPoints     = GET_UShort();

    maxProfile->maxStorage            = GET_UShort();
    maxProfile->maxFunctionDefs       = GET_UShort();
    maxProfile->maxInstructionDefs    = GET_UShort();
    maxProfile->maxStackElements      = GET_UShort();
    maxProfile->maxSizeOfInstructions = GET_UShort();
    maxProfile->maxComponentElements  = GET_UShort();
    maxProfile->maxComponentDepth     = GET_UShort();

    FORGET_Frame();

    /* XXX: an adjustment that is necessary to load certain */
    /*       broken fonts like `Keystrokes MT' :-(          */
    /*                                                      */
    /*   We allocate 64 function entries by default when    */
    /*   the maxFunctionDefs field is null.                 */

    if ( maxProfile->maxFunctionDefs == 0 )
      maxProfile->maxFunctionDefs = 64;

    face->root.num_glyphs = maxProfile->numGlyphs;

    face->root.max_points = MAX( maxProfile->maxCompositePoints,
                                 maxProfile->maxPoints );

    face->root.max_contours = MAX( maxProfile->maxCompositeContours,
                                   maxProfile->maxContours );

    face->max_components = (TT_ULong)maxProfile->maxComponentElements +
                           maxProfile->maxComponentDepth;

    FT_TRACE2(( "GASP loaded.\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Metrics                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the horizontal or vertical metrics table into a face object. */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face     :: A handle to the target face object.                    */
  /*    stream   :: The input stream.                                      */
  /*    vertical :: A boolean flag.  If set, load vertical metrics.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  static
  TT_Error  TT_Load_Metrics( TT_Face    face,
                             FT_Stream  stream,
                             TT_Bool    vertical )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;

    TT_UShort          n, num_shorts, num_longs;
    TT_Long            table;

    TT_LongMetrics**   longs;
    TT_ShortMetrics**  shorts;

    TT_LongMetrics*    long_metric;

    FT_TRACE2(( "TT_Load_%s_Metrics( %08lx )\n",
              vertical ? "Vertical" : "Horizontal", (TT_Long)face ));

    if ( vertical )
    {
      /* The table is optional, quit silently if it wasn't found       */
      /* XXX: Some fonts have a valid vertical header with a non-null  */
      /*      `number_of_VMetrics' fields, but no corresponding `vmtx' */
      /*      table to get the metrics from (e.g. mingliu).            */
      /*                                                               */
      /*      For safety, we set the field to 0!                       */
      /*                                                               */
      table = TT_LookUp_Table( face, TTAG_vmtx );
      if ( table < 0 )
      {
        /* Set the number_Of_VMetrics to 0! */
        FT_TRACE2(( "  no vertical header in file.\n" ));
        face->vertical.number_Of_VMetrics = 0;
        error = TT_Err_Ok;

        goto Exit;
      }

      num_longs = face->vertical.number_Of_VMetrics;
      longs     = (TT_LongMetrics**)&face->vertical.long_metrics;
      shorts    = (TT_ShortMetrics**)&face->vertical.short_metrics;
    }
    else
    {
      if ( ( table = TT_LookUp_Table( face, TTAG_hmtx ) ) < 0 )
      {
        FT_ERROR(( " no horizontal metrics in file!\n" ));
        error = TT_Err_Hmtx_Table_Missing;

        goto Exit;
      }

      num_longs = face->horizontal.number_Of_HMetrics;
      longs     = (TT_LongMetrics**)&face->horizontal.long_metrics;
      shorts    = (TT_ShortMetrics**)&face->horizontal.short_metrics;
    }

    num_shorts = face->max_profile.numGlyphs - num_longs;

    if ( num_longs > face->max_profile.numGlyphs )  /* sanity check */
    {
      FT_ERROR(( " more metrics than glyphs!\n" ));
      error = TT_Err_Invalid_Horiz_Metrics;

      goto Exit;
    }

    if ( ALLOC_ARRAY( *longs,  num_longs,  TT_LongMetrics ) ||
         ALLOC_ARRAY( *shorts, num_shorts, TT_ShortMetrics ) )
      goto Exit;

    if ( FILE_Seek( face->dir_tables[table].Offset )   ||
         ACCESS_Frame( face->dir_tables[table].Length ) )
      goto Exit;

    long_metric = *longs;
    for ( n = 0; n < num_longs; n++ )
    {
      long_metric->advance = GET_UShort();
      long_metric->bearing = GET_Short();
      long_metric++;
    }

    for ( n = 0; n < num_shorts; n++ )
      (*shorts)[n] = GET_Short();

    FORGET_Frame();

    FT_TRACE2(( "loaded\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Metrics_Header                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the horizontal or vertical header in a face object.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face     :: A handle to the target face object.                    */
  /*    stream   :: The input stream.                                      */
  /*    vertical :: A boolean flag.  If set, load vertical metrics.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Metrics_Header( TT_Face    face,
                                    FT_Stream  stream,
                                    TT_Bool    vertical )
  {
    TT_Error  error;

    TT_Long         table;
    TT_HoriHeader*  header;


    FT_TRACE2(( vertical ? "Vertical header " : "Horizontal header " ));

    if ( vertical )
    {
      face->vertical_info = 0;

      /* The vertical header table is optional, so return quietly if */
      /* we don't find it.                                           */
      if ( ( table = TT_LookUp_Table( face, TTAG_vhea ) ) < 0 )
      {
        error = TT_Err_Ok;

        goto Exit;
      }

      face->vertical_info = 1;
      header = (TT_HoriHeader*)&face->vertical;
    }
    else
    {
      /* The horizontal header is mandatory, return an error if we */
      /* don't find it.                                            */
      if ( ( table = TT_LookUp_Table( face, TTAG_hhea ) ) < 0 )
      {
        error = TT_Err_Horiz_Header_Missing;

        goto Exit;
      }

      header = &face->horizontal;
    }

    if ( FILE_Seek( face->dir_tables[table].Offset ) ||
         ACCESS_Frame( 36L ) )
      goto Exit;

    header->Version   = GET_ULong();
    header->Ascender  = GET_Short();
    header->Descender = GET_Short();
    header->Line_Gap  = GET_Short();

    header->advance_Width_Max = GET_UShort();

    header->min_Left_Side_Bearing  = GET_Short();
    header->min_Right_Side_Bearing = GET_Short();
    header->xMax_Extent            = GET_Short();
    header->caret_Slope_Rise       = GET_Short();
    header->caret_Slope_Run        = GET_Short();

    header->Reserved[0] = GET_Short();
    header->Reserved[1] = GET_Short();
    header->Reserved[2] = GET_Short();
    header->Reserved[3] = GET_Short();
    header->Reserved[4] = GET_Short();

    header->metric_Data_Format = GET_Short();
    header->number_Of_HMetrics = GET_UShort();

    FORGET_Frame();

    header->long_metrics  = NULL;
    header->short_metrics = NULL;

    FT_TRACE2(( "loaded\n" ));

    /* Now try to load the corresponding metrics */

    error = TT_Load_Metrics( face, stream, vertical );

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Names                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the name records.                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Names( TT_Face  face, FT_Stream  stream )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;

    TT_UShort  i;
    TT_Long    table;
    TT_Long    storageSize;

    TT_NameTable*  names;
    TT_NameRec*    namerec;


    FT_TRACE2(( "Names " ));

    if ( ( table = TT_LookUp_Table( face, TTAG_name ) ) < 0 )
    {
      /* The name table is required so indicate failure. */
      FT_TRACE2(( "is missing!\n" ));
      error = TT_Err_Name_Table_Missing;

      goto Exit;
    }

    /* Seek to the beginning of the table and check the frame access. */
    /* The names table has a 6 byte header.                           */
    if ( FILE_Seek( face->dir_tables[table].Offset ) ||
         ACCESS_Frame( 6L ) )
      goto Exit;

    names = &face->name_table;

    /* Load the initial names data. */
    names->format         = GET_UShort();
    names->numNameRecords = GET_UShort();
    names->storageOffset  = GET_UShort();

    FORGET_Frame();

    /* Allocate the array of name records. */
    if ( ALLOC_ARRAY( names->names,
                      names->numNameRecords,
                      TT_NameRec )                   ||
         ACCESS_Frame( names->numNameRecords * 12L ) )
      goto Exit;

    /* Load the name records and determine how much storage is needed */
    /* to hold the strings themselves.                                */
    namerec     = names->names;
    storageSize = 0;

    for ( i = 0; i < names->numNameRecords; i++, namerec++ )
    {
      TT_Long  upper;


      namerec->platformID   = GET_UShort();
      namerec->encodingID   = GET_UShort();
      namerec->languageID   = GET_UShort();
      namerec->nameID       = GET_UShort();
      namerec->stringLength = GET_UShort();
      namerec->stringOffset = GET_UShort();

      upper = namerec->stringOffset + namerec->stringLength;
      if ( upper > storageSize ) storageSize = upper;
    }

    FORGET_Frame();

    if ( ALLOC( names->storage, storageSize )  ||
         FILE_Read_At( face->dir_tables[table].Offset + names->storageOffset,
                       (void*)names->storage, storageSize ) )
      goto Exit;

    /* Go through and assign the string pointers to the name records. */
    namerec = names->names;
    for ( i = 0; i < names->numNameRecords; i++, namerec++ )
      namerec->string = names->storage + namerec->stringOffset;

    /* Print Name Record Table in case of debugging */
#if 0
    namerec = names->names;
    for ( i = 0; i < names->numNameRecords; i++, namerec++ )
    {
      TT_UShort  j;


      FT_TRACE2(( "%d %d %x %d ",
                   namerec->platformID,
                   namerec->encodingID,
                   namerec->languageID,
                   namerec->nameID ));

      /* I know that M$ encoded strings are Unicode,            */
      /* but this works reasonable well for debugging purposes. */
      if ( namerec->string )
        for ( j = 0; j < namerec->stringLength; j++ )
        {
          TT_Char  c = *(namerec->string + j);

          if ( (TT_Byte)c < 128 )
            FT_TRACE2(( "%c", c ));
        }
    }
    FT_TRACE2(( "\n" ));
#endif

    FT_TRACE2(( "loaded\n" ));

  Exit:
    return error;

  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Free_Names                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Frees the name records.                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the target face object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Free_Names( TT_Face  face )
  {
    FT_Memory      memory = face->root.driver->memory;
    TT_NameTable*  names  = &face->name_table;


    /* free strings table */
    FREE( names->names );

    /* free strings storage */
    FREE( names->storage );

    names->numNameRecords = 0;
    names->format         = 0;
    names->storageOffset  = 0;

    return TT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_CMap                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the cmap directory in a face object.  The cmaps itselves are */
  /*    loaded on demand in the `ttcmap.c' module.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face    :: A handle to the target face object.                     */
  /*    stream  :: A handle to the input stream.                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*     TrueType error code.  0 means success.                            */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_CMap( TT_Face    face,
                          FT_Stream  stream )
  {
    TT_Error  error;
    FT_Memory memory = stream->memory;

    TT_Long    i, off, cur_off, table_start;
    TT_UShort  n, limit;

    TT_CMapDir        cmap_dir;
    TT_CharMap        charmap;
    TT_CMapTable*     cmap;


    FT_TRACE2(( "CMaps " ));

    if ( ( i = TT_LookUp_Table( face, TTAG_cmap ) ) < 0 )
    {
      error = TT_Err_CMap_Table_Missing;

      goto Exit;
    }

    table_start = face->dir_tables[i].Offset;

    if ( ( FILE_Seek( table_start ) ) ||
         ( ACCESS_Frame( 4L ) ) )           /* 4 bytes cmap header */
      goto Exit;

    cmap_dir.tableVersionNumber = GET_UShort();
    cmap_dir.numCMaps           = GET_UShort();

    FORGET_Frame();

    off = FILE_Pos();  /* save offset to cmapdir[] which follows */

    /* save space in face table for cmap tables */
    if ( ALLOC_ARRAY( face->charmaps,
                      cmap_dir.numCMaps,
                      TT_CharMapRec ) )
      goto Exit;

    face->num_charmaps = cmap_dir.numCMaps;

    limit   = face->num_charmaps;
    charmap = face->charmaps;

    for ( n = 0; n < limit; n++ )
    {
      charmap->root.face = (FT_Face)face;
      cmap               = &charmap->cmap;

      if ( FILE_Seek( off )  ||
           ACCESS_Frame( 8L ) )
        goto Exit;

      cmap->loaded             = FALSE;
      cmap->platformID         = GET_UShort();
      cmap->platformEncodingID = GET_UShort();

      cur_off = GET_Long();

      FORGET_Frame();

      off = FILE_Pos();

      if ( FILE_Seek( table_start + cur_off ) ||
           ACCESS_Frame( 6L ) )
        goto Exit;

      cmap->format  = GET_UShort();
      cmap->length  = GET_UShort();
      cmap->version = GET_UShort();

      FORGET_Frame();

      cmap->offset = FILE_Pos();

      charmap++;
    }

    FT_TRACE2(( "loaded\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_OS2                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the OS2 table.                                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_OS2( TT_Face    face,
                         FT_Stream  stream )
  {
    TT_Error  error;

    TT_Long   i;
    TT_Int    j;
    TT_OS2*   os2;


    FT_TRACE2(( "OS/2 Table " ));

    /* We now support old Mac fonts where the OS/2 table doesn't  */
    /* exist.  Simply put, we set the `version' field to 0xFFFF   */
    /* and test this value each time we need to access the table. */
    if ( ( i = TT_LookUp_Table( face, TTAG_OS2 ) ) < 0 )
    {
      FT_TRACE2(( "is missing\n!" ));
        face->os2.version = 0xFFFF;
      error = TT_Err_Ok;

      goto Exit;
    }

    if ( FILE_Seek( face->dir_tables[i].Offset ) ||
         ACCESS_Frame( 78L ) )
      goto Exit;

    os2 = &face->os2;

    os2->version             = GET_UShort();
    os2->xAvgCharWidth       = GET_Short();
    os2->usWeightClass       = GET_UShort();
    os2->usWidthClass        = GET_UShort();
    os2->fsType              = GET_Short();
    os2->ySubscriptXSize     = GET_Short();
    os2->ySubscriptYSize     = GET_Short();
    os2->ySubscriptXOffset   = GET_Short();
    os2->ySubscriptYOffset   = GET_Short();
    os2->ySuperscriptXSize   = GET_Short();
    os2->ySuperscriptYSize   = GET_Short();
    os2->ySuperscriptXOffset = GET_Short();
    os2->ySuperscriptYOffset = GET_Short();
    os2->yStrikeoutSize      = GET_Short();
    os2->yStrikeoutPosition  = GET_Short();
    os2->sFamilyClass        = GET_Short();

    for ( j = 0; j < 10; j++ )
      os2->panose[j] = GET_Byte();

    os2->ulUnicodeRange1     = GET_ULong();
    os2->ulUnicodeRange2     = GET_ULong();
    os2->ulUnicodeRange3     = GET_ULong();
    os2->ulUnicodeRange4     = GET_ULong();

    for ( j = 0; j < 4; j++ )
      os2->achVendID[j] = GET_Byte();

    os2->fsSelection         = GET_UShort();
    os2->usFirstCharIndex    = GET_UShort();
    os2->usLastCharIndex     = GET_UShort();
    os2->sTypoAscender       = GET_Short();
    os2->sTypoDescender      = GET_Short();
    os2->sTypoLineGap        = GET_Short();
    os2->usWinAscent         = GET_UShort();
    os2->usWinDescent        = GET_UShort();

    FORGET_Frame();

    if ( os2->version >= 0x0001 )
    {
      /* only version 1 tables */

      if ( ACCESS_Frame( 8L ) )  /* read into frame */
        goto Exit;

      os2->ulCodePageRange1 = GET_ULong();
      os2->ulCodePageRange2 = GET_ULong();

      FORGET_Frame();
    }
    else
    {
      os2->ulCodePageRange1 = 0;
      os2->ulCodePageRange2 = 0;
    }

    FT_TRACE2(( "loaded\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Postscript                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the Postscript table.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_PostScript( TT_Face    face,
                                FT_Stream  stream )
  {
    TT_Error        error;
    TT_Long         i;
    TT_Postscript*  post = &face->postscript;


    FT_TRACE2(( "PostScript " ));

    if ( ( i = TT_LookUp_Table( face, TTAG_post ) ) < 0 )
      return TT_Err_Post_Table_Missing;

    if ( FILE_Seek( face->dir_tables[i].Offset ) ||
         ACCESS_Frame( 32L ) )
      return error;

    /* read frame data into face table */

    post->FormatType         = GET_ULong();
    post->italicAngle        = GET_ULong();
    post->underlinePosition  = GET_Short();
    post->underlineThickness = GET_Short();
    post->isFixedPitch       = GET_ULong();
    post->minMemType42       = GET_ULong();
    post->maxMemType42       = GET_ULong();
    post->minMemType1        = GET_ULong();
    post->maxMemType1        = GET_ULong();

    FORGET_Frame();

    /* we don't load the glyph names, we do that in another */
    /* module (ttpost).                                     */

    FT_TRACE2(( "loaded\n" ));

    return TT_Err_Ok;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Gasp                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the `GASP' table into a face object.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Gasp( TT_Face    face,
                          FT_Stream  stream )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;

    TT_Long        table;
    TT_UShort      num_ranges;
    TT_UShort      j;
    TT_GaspRange*  gaspranges;


    FT_TRACE2(( "TT_Load_Gasp( %08lx )\n", (TT_Long)face ));

    error = TT_Err_Ok;

    /* the gasp table is optional */
    if ( ( table = TT_LookUp_Table( face, TTAG_gasp ) ) < 0 )
      goto Exit;

    if ( FILE_Seek( face->dir_tables[table].Offset ) ||
         ACCESS_Frame( 4L ) )
      goto Exit;

    face->gasp.version   = GET_UShort();
    face->gasp.numRanges = GET_UShort();

    FORGET_Frame();

    num_ranges = face->gasp.numRanges;
    FT_TRACE3(( "number of ranges = %d\n", num_ranges ));

    if ( ALLOC_ARRAY( gaspranges, num_ranges, TT_GaspRange ) ||
         ACCESS_Frame( num_ranges * 4L ) )
      goto Exit;

    face->gasp.gaspRanges = gaspranges;

    for ( j = 0; j < num_ranges; j++ )
    {
      gaspranges[j].maxPPEM  = GET_UShort();
      gaspranges[j].gaspFlag = GET_UShort();

      FT_TRACE3(( " [max:%d flag:%d]",
                gaspranges[j].maxPPEM,
                gaspranges[j].gaspFlag ));
    }
    FT_TRACE3(( "\n" ));

    FORGET_Frame();

    FT_TRACE2(( "GASP loaded\n" ));

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Kern                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the first kerning table with format 0 in the font.  Only     */
  /*    accepts the first horizontal kerning table.  Developers should use */
  /*    the `ftxkern' extension to access other kerning tables in the font */
  /*    file, if they really want to.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: The input stream.                                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Kern( TT_Face    face,
                          FT_Stream  stream )
  {
    TT_Error   error;
    FT_Memory  memory = stream->memory;

    TT_Long    table;
    TT_UShort  n, num_tables, version;


    error = TT_Err_Ok;
    table = TT_LookUp_Table( face, TTAG_kern );
    if ( table < 0 )
      goto Exit;

    if ( FILE_Seek( face->dir_tables[table].Offset ) ||
         READ_UShort( version )                      ||
         READ_UShort( num_tables )                   )
      goto Exit;

    for ( n = 0; n < num_tables; n++ )
    {
      TT_UShort  coverage;


      if ( FILE_Skip( 4L )         ||
           READ_UShort( coverage ) )
        goto Exit;

      if ( coverage == 0x0001 )
      {
        TT_UShort        num_pairs, m;
        TT_Kern_0_Pair*  pair;


        /* found a horizontal format 0 kerning table ! */
        if ( READ_UShort( num_pairs ) ||
             FILE_Skip( 6 ) )
          goto Exit;

        /* allocate array of kerning pairs */
        if ( ALLOC_ARRAY( face->kern_pairs, num_pairs, TT_Kern_0_Pair ) )
          goto Exit;

        /* read the kerning pairs */
        if ( ACCESS_Frame( 6L * num_pairs ) )
          goto Exit;

        pair = face->kern_pairs;
        for ( m = 0; m < num_pairs; m++ )
        {
          pair->left  = GET_UShort();
          pair->right = GET_UShort();
          pair->value = GET_UShort();
          pair++;
        }

        FORGET_Frame();

        face->num_kern_pairs   = num_pairs;
        face->kern_table_index = n;
        goto Exit;
      }
    }

    /* no kern table found -- doesn't matter */
    face->kern_table_index = -1;
    face->num_kern_pairs   = 0;
    face->kern_pairs       = NULL;

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Load_Hdmx                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Loads the horizontal device metrics table.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face   :: A handle to the target face object.                      */
  /*    stream :: A handle to the input stream.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Load_Hdmx( TT_Face    face, 
                          FT_Stream  stream )
  {
    TT_Error  error;
    FT_Memory memory = stream->memory;

    TT_HdmxRec*  rec;
    TT_Hdmx      hdmx;
    TT_Long      table;
    TT_UShort    n;
	TT_Long      num_glyphs;
    TT_Long      record_size;

    hdmx.version     = 0;
    hdmx.num_records = 0;
    hdmx.records     = 0;

    face->hdmx = hdmx;

    error = TT_Err_Ok;

    /* ths table is optional */
    if ( ( table = TT_LookUp_Table( face, TTAG_hdmx ) ) < 0 )
      return error;

    if ( FILE_Seek( face->dir_tables[table].Offset )  ||
         ACCESS_Frame( 8L ) )
      goto Exit;

    hdmx.version     = GET_UShort();
    hdmx.num_records = GET_Short();
    record_size      = GET_Long();

    FORGET_Frame();

    /* Only recognize format 0 */
    if ( hdmx.version != 0 )
      goto Exit;

    if ( ALLOC( hdmx.records, sizeof ( TT_HdmxRec ) * hdmx.num_records ) )
      goto Exit;

    num_glyphs   = face->root.num_glyphs;
    record_size -= num_glyphs + 2;
    rec          = hdmx.records;

    for ( n = 0; n < hdmx.num_records; n++ )
    {
      /* read record */
      if ( READ_Byte( rec->ppem      ) ||
           READ_Byte( rec->max_width ) )
        goto Exit;

      if ( ALLOC( rec->widths, num_glyphs )  ||
           FILE_Read( rec->widths, num_glyphs ) )
        goto Exit;

      /* skip padding bytes */
      if ( record_size > 0 )
        if ( FILE_Skip( record_size ) )
          goto Exit;

      rec++;
    }

    face->hdmx = hdmx;

  Exit:
    return error;
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    TT_Free_Hdmx                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Frees the horizontal device metrics table.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the target face object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    TrueType error code.  0 means success.                             */
  /*                                                                       */
  LOCAL_FUNC
  TT_Error  TT_Free_Hdmx( TT_Face  face )
  {
    if ( face )
    {
      TT_UShort  n;
      FT_Memory  memory = face->root.driver->memory;


      for ( n = 0; n < face->hdmx.num_records; n++ )
        FREE( face->hdmx.records[n].widths );

      FREE( face->hdmx.records );
      face->hdmx.num_records = 0;
    }

    return TT_Err_Ok;
  }


/* END */
