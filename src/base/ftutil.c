#include <ft2build.h>
#include FT_INTERNAL_DEBUG_H
#include FT_INTERNAL_UTIL_H


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_memory


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****               M E M O R Y   M A N A G E M E N T               *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /* documentation is in ftmemory.h */

  FT_BASE_DEF( FT_Error )
  FT_Alloc( FT_Memory  memory,
            FT_Long    size,
            void*     *P )
  {
    FT_Assert( P != 0 );

    if ( size > 0 )
    {
      *P = memory->alloc( memory, size );
      if ( !*P )
      {
        FT_ERROR(( "FT_Alloc:" ));
        FT_ERROR(( " Out of memory? (%ld requested)\n",
                   size ));

        return FT_Err_Out_Of_Memory;
      }
      MEM_Set( *P, 0, size );
    }
    else
      *P = NULL;

    FT_TRACE7(( "FT_Alloc:" ));
    FT_TRACE7(( " size = %ld, block = 0x%08p, ref = 0x%08p\n",
                size, *P, P ));

    return FT_Err_Ok;
  }


  /* documentation is in ftmemory.h */

  FT_BASE_DEF( FT_Error )
  FT_Realloc( FT_Memory  memory,
              FT_Long    current,
              FT_Long    size,
              void**     P )
  {
    void*  Q;


    FT_Assert( P != 0 );

    /* if the original pointer is NULL, call FT_Alloc() */
    if ( !*P )
      return FT_Alloc( memory, size, P );

    /* if the new block if zero-sized, clear the current one */
    if ( size <= 0 )
    {
      FT_Free( memory, P );
      return FT_Err_Ok;
    }

    Q = memory->realloc( memory, current, size, *P );
    if ( !Q )
      goto Fail;

    if ( size > current )
      memset( (char*)Q + current, 0, size - current );

    *P = Q;
    return FT_Err_Ok;

  Fail:
    FT_ERROR(( "FT_Realloc:" ));
    FT_ERROR(( " Failed (current %ld, requested %ld)\n",
               current, size ));
    return FT_Err_Out_Of_Memory;
  }


  /* documentation is in ftmemory.h */

  FT_BASE_DEF( void )
  FT_Free( FT_Memory  memory,
           void**     P )
  {
    FT_TRACE7(( "FT_Free:" ));
    FT_TRACE7(( " Freeing block 0x%08p, ref 0x%08p\n",
                P, P ? *P : (void*)0 ));

    if ( P && *P )
    {
      memory->free( memory, *P );
      *P = 0;
    }
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****            D O U B L Y   L I N K E D   L I S T S              *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_list

  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( FT_ListNode )
  FT_List_Find( FT_List  list,
                void*    data )
  {
    FT_ListNode  cur;


    cur = list->head;
    while ( cur )
    {
      if ( cur->data == data )
        return cur;

      cur = cur->next;
    }

    return (FT_ListNode)0;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Add( FT_List      list,
               FT_ListNode  node )
  {
    FT_ListNode  before = list->tail;


    node->next = 0;
    node->prev = before;

    if ( before )
      before->next = node;
    else
      list->head = node;

    list->tail = node;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Insert( FT_List      list,
                  FT_ListNode  node )
  {
    FT_ListNode  after = list->head;


    node->next = after;
    node->prev = 0;

    if ( !after )
      list->tail = node;
    else
      after->prev = node;

    list->head = node;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Remove( FT_List      list,
                  FT_ListNode  node )
  {
    FT_ListNode  before, after;


    before = node->prev;
    after  = node->next;

    if ( before )
      before->next = after;
    else
      list->head = after;

    if ( after )
      after->prev = before;
    else
      list->tail = before;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Up( FT_List      list,
              FT_ListNode  node )
  {
    FT_ListNode  before, after;


    before = node->prev;
    after  = node->next;

    /* check whether we are already on top of the list */
    if ( !before )
      return;

    before->next = after;

    if ( after )
      after->prev = before;
    else
      list->tail = before;

    node->prev       = 0;
    node->next       = list->head;
    list->head->prev = node;
    list->head       = node;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( FT_Error )
  FT_List_Iterate( FT_List            list,
                   FT_List_Iterator   iterator,
                   void*              user )
  {
    FT_ListNode  cur   = list->head;
    FT_Error     error = FT_Err_Ok;


    while ( cur )
    {
      FT_ListNode  next = cur->next;


      error = iterator( cur, user );
      if ( error )
        break;

      cur = next;
    }

    return error;
  }


  /* documentation is in ftlist.h */

  FT_EXPORT_DEF( void )
  FT_List_Finalize( FT_List             list,
                    FT_List_Destructor  destroy,
                    FT_Memory           memory,
                    void*               user )
  {
    FT_ListNode  cur;


    cur = list->head;
    while ( cur )
    {
      FT_ListNode  next = cur->next;
      void*        data = cur->data;


      if ( destroy )
        destroy( memory, data, user );

      FREE( cur );
      cur = next;
    }

    list->head = 0;
    list->tail = 0;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****                    G L Y P H   L O A D E R                    *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* The glyph loader is a simple object which is used to load a set of    */
  /* glyphs easily.  It is critical for the correct loading of composites. */
  /*                                                                       */
  /* Ideally, one can see it as a stack of abstract `glyph' objects.       */
  /*                                                                       */
  /*   loader.base     Is really the bottom of the stack.  It describes a  */
  /*                   single glyph image made of the juxtaposition of     */
  /*                   several glyphs (those `in the stack').              */
  /*                                                                       */
  /*   loader.current  Describes the top of the stack, on which a new      */
  /*                   glyph can be loaded.                                */
  /*                                                                       */
  /*   Rewind          Clears the stack.                                   */
  /*   Prepare         Set up `loader.current' for addition of a new glyph */
  /*                   image.                                              */
  /*   Add             Add the `current' glyph image to the `base' one,    */
  /*                   and prepare for another one.                        */
  /*                                                                       */
  /* The glyph loader is now a base object.  Each driver used to           */
  /* re-implement it in one way or the other, which wasted code and        */
  /* energy.                                                               */
  /*                                                                       */
  /*************************************************************************/


  /* create a new glyph loader */
  FT_BASE_DEF( FT_Error )
  FT_GlyphLoader _New( FT_Memory         memory,
                      FT_GlyphLoader   *aloader )
  {
    FT_GlyphLoader   loader;
    FT_Error         error;


    if ( !ALLOC( loader, sizeof ( *loader ) ) )
    {
      loader->memory = memory;
      *aloader       = loader;
    }
    return error;
  }


  /* rewind the glyph loader - reset counters to 0 */
  FT_BASE_DEF( void )
  FT_GlyphLoader _Rewind( FT_GlyphLoader   loader )
  {
    FT_GlyphLoad*  base    = &loader->base;
    FT_GlyphLoad*  current = &loader->current;


    base->outline.n_points   = 0;
    base->outline.n_contours = 0;
    base->num_subglyphs      = 0;

    *current = *base;
  }


  /* reset the glyph loader, frees all allocated tables */
  /* and starts from zero                               */
  FT_BASE_DEF( void )
  FT_GlyphLoader _Reset( FT_GlyphLoader   loader )
  {
    FT_Memory memory = loader->memory;


    FREE( loader->base.outline.points );
    FREE( loader->base.outline.tags );
    FREE( loader->base.outline.contours );
    FREE( loader->base.extra_points );
    FREE( loader->base.subglyphs );

    loader->max_points    = 0;
    loader->max_contours  = 0;
    loader->max_subglyphs = 0;

    FT_GlyphLoader _Rewind( loader );
  }


  /* delete a glyph loader */
  FT_BASE_DEF( void )
  FT_GlyphLoader _Done( FT_GlyphLoader   loader )
  {
    if ( loader )
    {
      FT_Memory memory = loader->memory;


      FT_GlyphLoader _Reset( loader );
      FREE( loader );
    }
  }


  /* re-adjust the `current' outline fields */
  static void
  FT_GlyphLoader _Adjust_Points( FT_GlyphLoader   loader )
  {
    FT_Outline*  base    = &loader->base.outline;
    FT_Outline*  current = &loader->current.outline;


    current->points   = base->points   + base->n_points;
    current->tags     = base->tags     + base->n_points;
    current->contours = base->contours + base->n_contours;

    /* handle extra points table - if any */
    if ( loader->use_extra )
      loader->current.extra_points =
        loader->base.extra_points + base->n_points;
  }


  FT_BASE_DEF( FT_Error )
  FT_GlyphLoader _Create_Extra( FT_GlyphLoader   loader )
  {
    FT_Error   error;
    FT_Memory  memory = loader->memory;


    if ( !ALLOC_ARRAY( loader->base.extra_points,
                       loader->max_points, FT_Vector ) )
    {
      loader->use_extra = 1;
      FT_GlyphLoader _Adjust_Points( loader );
    }
    return error;
  }


  /* re-adjust the `current' subglyphs field */
  static void
  FT_GlyphLoader _Adjust_Subglyphs( FT_GlyphLoader   loader )
  {
    FT_GlyphLoad* base    = &loader->base;
    FT_GlyphLoad* current = &loader->current;


    current->subglyphs = base->subglyphs + base->num_subglyphs;
  }


  /* Ensure that we can add `n_points' and `n_contours' to our glyph. this */
  /* function reallocates its outline tables if necessary.  Note that it   */
  /* DOESN'T change the number of points within the loader!                */
  /*                                                                       */
  FT_BASE_DEF( FT_Error )
  FT_GlyphLoader _Check_Points( FT_GlyphLoader   loader,
                               FT_UInt          n_points,
                               FT_UInt          n_contours )
  {
    FT_Memory    memory  = loader->memory;
    FT_Error     error   = FT_Err_Ok;
    FT_Outline*  base    = &loader->base.outline;
    FT_Outline*  current = &loader->current.outline;
    FT_Bool      adjust  = 1;

    FT_UInt      new_max, old_max;


    /* check points & tags */
    new_max = base->n_points + current->n_points + n_points;
    old_max = loader->max_points;

    if ( new_max > old_max )
    {
      new_max = ( new_max + 7 ) & -8;

      if ( REALLOC_ARRAY( base->points, old_max, new_max, FT_Vector ) ||
           REALLOC_ARRAY( base->tags,   old_max, new_max, FT_Byte   ) )
       goto Exit;

      if ( loader->use_extra &&
           REALLOC_ARRAY( loader->base.extra_points, old_max,
                          new_max, FT_Vector ) )
       goto Exit;

      adjust = 1;
      loader->max_points = new_max;
    }

    /* check contours */
    old_max = loader->max_contours;
    new_max = base->n_contours + current->n_contours +
              n_contours;
    if ( new_max > old_max )
    {
      new_max = ( new_max + 3 ) & -4;
      if ( REALLOC_ARRAY( base->contours, old_max, new_max, FT_Short ) )
        goto Exit;

      adjust = 1;
      loader->max_contours = new_max;
    }

    if ( adjust )
      FT_GlyphLoader _Adjust_Points( loader );

  Exit:
    return error;
  }


  /* Ensure that we can add `n_subglyphs' to our glyph. this function */
  /* reallocates its subglyphs table if necessary.  Note that it DOES */
  /* NOT change the number of subglyphs within the loader!            */
  /*                                                                  */
  FT_BASE_DEF( FT_Error )
  FT_GlyphLoader _Check_Subglyphs( FT_GlyphLoader   loader,
                                  FT_UInt          n_subs )
  {
    FT_Memory  memory = loader->memory;
    FT_Error   error  = FT_Err_Ok;
    FT_UInt    new_max, old_max;

    FT_GlyphLoad*  base    = &loader->base;
    FT_GlyphLoad*  current = &loader->current;


    new_max = base->num_subglyphs + current->num_subglyphs + n_subs;
    old_max = loader->max_subglyphs;
    if ( new_max > old_max )
    {
      new_max = ( new_max + 1 ) & -2;
      if ( REALLOC_ARRAY( base->subglyphs, old_max, new_max, FT_SubGlyph ) )
        goto Exit;

      loader->max_subglyphs = new_max;

      FT_GlyphLoader _Adjust_Subglyphs( loader );
    }

  Exit:
    return error;
  }


  /* prepare loader for the addition of a new glyph on top of the base one */
  FT_BASE_DEF( void )
  FT_GlyphLoader _Prepare( FT_GlyphLoader   loader )
  {
    FT_GlyphLoad*  current = &loader->current;


    current->outline.n_points   = 0;
    current->outline.n_contours = 0;
    current->num_subglyphs      = 0;

    FT_GlyphLoader _Adjust_Points   ( loader );
    FT_GlyphLoader _Adjust_Subglyphs( loader );
  }


  /* add current glyph to the base image - and prepare for another */
  FT_BASE_DEF( void )
  FT_GlyphLoader _Add( FT_GlyphLoader   loader )
  {
    FT_GlyphLoad*  base    = &loader->base;
    FT_GlyphLoad*  current = &loader->current;

    FT_UInt        n_curr_contours = current->outline.n_contours;
    FT_UInt        n_base_points   = base->outline.n_points;
    FT_UInt        n;


    base->outline.n_points =
      (short)( base->outline.n_points + current->outline.n_points );
    base->outline.n_contours =
      (short)( base->outline.n_contours + current->outline.n_contours );

    base->num_subglyphs += current->num_subglyphs;

    /* adjust contours count in newest outline */
    for ( n = 0; n < n_curr_contours; n++ )
      current->outline.contours[n] =
        (short)( current->outline.contours[n] + n_base_points );

    /* prepare for another new glyph image */
    FT_GlyphLoader _Prepare( loader );
  }


  FT_BASE_DEF( FT_Error )
  FT_GlyphLoader _Copy_Points( FT_GlyphLoader   target,
                              FT_GlyphLoader   source )
  {
    FT_Error  error;
    FT_UInt   num_points   = source->base.outline.n_points;
    FT_UInt   num_contours = source->base.outline.n_contours;


    error = FT_GlyphLoader _Check_Points( target, num_points, num_contours );
    if ( !error )
    {
      FT_Outline*  out = &target->base.outline;
      FT_Outline*  in  = &source->base.outline;


      MEM_Copy( out->points, in->points,
                num_points * sizeof ( FT_Vector ) );
      MEM_Copy( out->tags, in->tags,
                num_points * sizeof ( char ) );
      MEM_Copy( out->contours, in->contours,
                num_contours * sizeof ( short ) );

      /* do we need to copy the extra points? */
      if ( target->use_extra && source->use_extra )
        MEM_Copy( target->base.extra_points, source->base.extra_points,
                  num_points * sizeof ( FT_Vector ) );

      out->n_points   = (short)num_points;
      out->n_contours = (short)num_contours;

      FT_GlyphLoader _Adjust_Points( target );
    }

    return error;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****                          S T R E A M S                        *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_stream

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_new_input_stream                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Creates a new input stream object from an FT_Open_Args structure.  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The function expects a valid `astream' parameter.                  */
  /*                                                                       */
  FT_EXPORT_DEF( FT_Error )
  FT_Stream_Open( FT_Library     library,
                  FT_Open_Args*  args,
                  FT_Stream*     astream )
  {
    FT_Error   error;
    FT_Memory  memory;
    FT_Stream  stream;


    if ( !library )
      return FT_Err_Invalid_Library_Handle;

    if ( !args )
      return FT_Err_Invalid_Argument;

    *astream = 0;
    memory   = library->memory;
    if ( ALLOC( stream, sizeof ( *stream ) ) )
      goto Exit;

    stream->memory = memory;

    /* now, look at the stream flags */
    if ( args->flags & ft_open_memory )
    {
      error = 0;
      FT_New_Memory_Stream( library,
                            (FT_Byte*)args->memory_base,
                            args->memory_size,
                            stream );
    }
    else if ( args->flags & ft_open_pathname )
    {
      error = FT_Stream_New( args->pathname, stream );
      stream->pathname.pointer = args->pathname;
    }
    else if ( ( args->flags & ft_open_stream ) && args->stream )
    {
      /* in this case, we do not need to allocate a new stream object */
      /* since the caller is responsible for closing it himself       */
      FREE( stream );
      stream = args->stream;
    }
    else
      error = FT_Err_Invalid_Argument;

    if ( error )
      FREE( stream );

    *astream = stream;

  Exit:
    return error;
  }


  /* documentation is in ftobjs.h */

  FT_EXPORT_DEF( void )
  FT_Stream_Done( FT_Stream  stream )
  {
    if ( stream && stream->close )
    {
      stream->close( stream );
      stream->close = 0;
    }
  }


  static void
  ft_done_stream( FT_Stream*  astream,
                  FT_Int      external )
  {
    FT_Stream  stream = *astream;


    if ( stream->close )
      stream->close( stream );

    if ( !external )
    {
      FT_Memory  memory = stream->memory;


      FREE( stream );
    }
    *astream = 0;
  }

