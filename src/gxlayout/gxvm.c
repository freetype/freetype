/***************************************************************************/
/*                                                                         */
/*  gxvm.c                                                                 */
/*                                                                         */
/*    AAT/TrueTypeGX glyph substitution automaton (body).                  */
/*                                                                         */
/*  Copyright 2003 by                                                      */
/*  Masatake YAMATO and Redhat K.K.                                        */
/*                                                                         */
/*  This file may only be used,                                            */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* Development of the code in this file is support of                      */
/* Information-technology Promotion Agency, Japan.                         */
/***************************************************************************/

#include <ft2build.h>
#include FT_INTERNAL_DEBUG_H
#include FT_LAYOUT_H
#include FT_LIST_H
#include "gxvm.h"
#include "gxutils.h"
#include "gxerrors.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvm


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                        Ligature Stack                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#define LIGATURE_STACK_DEPTH 16
typedef struct LigatureStackRec_
{  
  FT_Char depth;
  FT_Char top;
  FT_Char ligatured_top;
  FT_ULong indexes[LIGATURE_STACK_DEPTH];
  FT_ULong ligatured_indexes[LIGATURE_STACK_DEPTH];
} LigatureStackRec, *LigatureStack;

static void ligstack_init(LigatureStack stack);
static void ligstack_push( LigatureStack stack, 
			   FT_ULong index );
static FT_ULong ligstack_pop( LigatureStack stack );
static void     ligstack_unify(LigatureStack stack);
static void     ligstack_ligatured_init(LigatureStack stack);
static void     ligstack_ligatured_push(LigatureStack stack,
				    FT_ULong index );

static FT_Bool  ligstack_ligatured_empty(LigatureStack stack);
static FT_ULong ligstack_ligatured_pop(LigatureStack stack);



static void
ligstack_init(LigatureStack stack)
{
  FT_Int i;
  stack->depth = LIGATURE_STACK_DEPTH;
  stack->top = -1;
  for ( i = 0; i < stack->depth; i++ )
    stack->indexes[i]   = 0;
  ligstack_ligatured_init( stack );
}

static void
ligstack_push( LigatureStack stack, FT_ULong index )
{
  stack->top++;
  if ( stack->top >= stack->depth )
    stack->top = 0;
  stack->indexes[stack->top] = index;
}

static FT_ULong
ligstack_pop( LigatureStack stack )
{
  FT_ULong index;
  
  index = stack->indexes[stack->top];
  stack->top--;
  if ( stack->top < 0 )
    stack->top= stack->depth - 1;
  return index;
}


static void
ligstack_unify(LigatureStack stack)
{
  FT_ULong index;
  while ( !ligstack_ligatured_empty ( stack ) )
    {
      index = ligstack_ligatured_pop( stack );
      ligstack_push( stack, index );
  }
}

static void
ligstack_ligatured_init(LigatureStack stack)
{
  stack->ligatured_top = -1;
}

static void
ligstack_ligatured_push(LigatureStack stack,
			FT_ULong index )
{
  stack->ligatured_top++;
  if ( stack->ligatured_top >= stack->depth )
    stack->ligatured_top 			 = 0;
  stack->ligatured_indexes[stack->ligatured_top] = index;
}

static FT_Bool
ligstack_ligatured_empty(LigatureStack stack)
{
  return (stack->ligatured_top >= 0)? 0: 1;
}

static FT_ULong
ligstack_ligatured_pop(LigatureStack stack)
{
  FT_ULong index;
  /* ??? */
  index = stack->ligatured_indexes[stack->ligatured_top];
  if ( stack->ligatured_top > -1 )
    stack->ligatured_top--;
  return index;
}


  
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                          Glyphs List                            ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#define INSERTION_GLYPHS_MAX 31
  typedef struct InsertionGlyphsRec_
  {
    FTL_GlyphRec glyphs[INSERTION_GLYPHS_MAX];
    FT_UShort length;
  } InsertionGlyphsRec, *InsertionGlyphs;

  typedef struct GlyphNodeRec_
  {
    FT_ListNodeRec root;
    FTL_GlyphRec   glyph;
  } GlyphNodeRec, *GlyphNode;


  typedef struct GlyphsListRec_
  {
    FT_ListRec       root;
    GlyphNode        current;
    GlyphNode        mark;
    FT_Memory        memory;
  } GlyphsListRec, *GlyphsList;

  typedef FT_Error 
  (* GList_InsertFunc)( GlyphsList glist, 
			InsertionGlyphs insertion,
			FT_Bool before );

  static void insertion_glyphs_init ( InsertionGlyphs glyphs );
  static void insertion_glyphs_push ( InsertionGlyphs glyphs, FT_UShort gid );
  static FT_Error insertion_glyphs_store ( InsertionGlyphs glyphs,
					   FT_Memory memory,
					   FT_List list );

  static void
  insertion_glyphs_init ( InsertionGlyphs glyphs )
  {
    glyphs->length = 0;
  }
  static void
  insertion_glyphs_push ( InsertionGlyphs glyphs, FT_UShort gid )
  {
    if (!( glyphs->length < INSERTION_GLYPHS_MAX ))
      glyphs->length = 0;
    glyphs->glyphs[glyphs->length].gid = gid;
  }

  static FT_Error
  insertion_glyphs_store ( InsertionGlyphs glyphs, 
			   FT_Memory memory,
			   FT_List list )
  {
    FT_UShort i;
    FT_Error error = FT_Err_Ok;
    GlyphNode gnode;
    
    for ( i = 0; i < glyphs->length; i++ )
      {
	if ( FT_NEW( gnode ) )
	  goto Failure;
	gnode->glyph = glyphs->glyphs[i];
	FT_List_Add( list, (FT_ListNode)gnode );
      }
    return error;
  Failure:
    FT_List_Finalize( list, NULL, memory, NULL );
    return error;
  }

  static FT_Error glist_init    ( FT_Memory memory, 
				  GlyphsList glist,
				  FTL_Glyphs_Array garray );
  static FT_Error glist_store   ( GlyphsList glist, 
				  FTL_Glyphs_Array garray );
  static void glist_finalize    ( GlyphsList glist );

/*
 * GList_InsertFunc
 */
  static FT_Error glist_insert_current      ( GlyphsList glist, 
					      InsertionGlyphs insertion,
					      FT_Bool before );
  static FT_Error glist_insert_mark         ( GlyphsList glist, 
					      InsertionGlyphs insertion,
					      FT_Bool before );

  static void      glist_set_mark    ( GlyphsList glist );
  static FTL_Glyph glist_get_next    ( GlyphsList glist );
  static FTL_Glyph glist_get_current ( GlyphsList glist );

/* glist function group local;
   - glist_insert_before_*
   - glist_insert_after_*
   - glist_insert */
  static FT_Error glist_insert_before_current( GlyphsList glist, InsertionGlyphs insertion );
  static FT_Error glist_insert_after_current ( GlyphsList glist, InsertionGlyphs insertion );
  static FT_Error glist_insert_before_mark   ( GlyphsList glist, InsertionGlyphs insertion );
  static FT_Error glist_insert_after_mark    ( GlyphsList glist, InsertionGlyphs insertion );
  static FT_Error glist_insert               ( GlyphsList glist, 
					       FT_ListNode before, 
					       FT_ListNode after, 
					       InsertionGlyphs insertion );

  static FT_Error
  glist_init    ( FT_Memory memory, GlyphsList glist, FTL_Glyphs_Array garray )
  {
    FT_Error error;
    FT_ULong i;
    GlyphNode glyph_node;
    
    ((FT_List)glist)->head = NULL;
    ((FT_List)glist)->tail = NULL;
    glist->mark 	   = NULL;
    glist->memory 	   = memory;

    for ( i = 0; i < garray->length; i++ )
      {
	if ( FT_NEW(glyph_node) )
	  goto Failure;
	glyph_node->glyph = garray->glyphs[i];
	FT_List_Add((FT_List)glist, (FT_ListNode)glyph_node);
      }
    glist->current = (GlyphNode)((FT_List)glist)->head;
    
    return FT_Err_Ok;
  Failure:
    FT_List_Finalize( (FT_List)glist, NULL, memory, NULL );
    return error;
  }

  static FT_Error
  glist_store   ( GlyphsList glist, FTL_Glyphs_Array garray )
  {
    FT_Error error;
    FT_ULong length = 0, i;
    FT_ListNode tmp;
    
    tmp = glist->root.head;
    while ( tmp )
      {
	length++;
	tmp = tmp->next;
      }

    if (( error = FTL_Set_Glyphs_Array_Length( garray, length ) ))
      return error;
    
    tmp = glist->root.head;
    for ( i = 0; i < length; i++ )
      {
	garray->glyphs[i] = ((GlyphNode)tmp)->glyph;
	tmp 		  = tmp->next;
      }
    return error;
  }

  static void
  glist_finalize( GlyphsList glist )
  {
    FT_Memory memory = glist->memory;
    FT_List_Finalize( (FT_List)glist, NULL, memory, NULL );
  }

  static FT_Error
  glist_insert_current      ( GlyphsList glist, 
			      InsertionGlyphs insertion,
			      FT_Bool before )
  {
    return ( before
	     ? glist_insert_before_current( glist, insertion )
	     : glist_insert_after_current( glist, insertion ));
  }

  static FT_Error
  glist_insert_mark         ( GlyphsList glist, 
			      InsertionGlyphs insertion,
			      FT_Bool before )
  {
    return ( before 
	     ? glist_insert_before_mark( glist, insertion )
	     : glist_insert_after_mark( glist, insertion ));
  }

  static FT_Error
  glist_insert_before_current( GlyphsList glist, InsertionGlyphs insertion )
  {
    FT_ListNode before, after;
    
    after = (FT_ListNode)glist->current;
    FT_ASSERT( after );
    before = after->prev;
    return glist_insert(glist, before, after, insertion);
  }

  static FT_Error
  glist_insert_after_current ( GlyphsList glist, InsertionGlyphs insertion )
  {
    FT_ListNode before, after;
    
    before = (FT_ListNode)glist->current; 
    FT_ASSERT( before );
    after = before->next;
    return glist_insert(glist, before, after, insertion);
  }

  static FT_Error
  glist_insert_before_mark   ( GlyphsList glist, InsertionGlyphs insertion )
  {
    FT_ListNode before, after;
    
    after = (FT_ListNode)glist->mark;
    FT_ASSERT( after );
    before = after->prev;
    return glist_insert(glist, before, after, insertion);
  }

  static FT_Error
  glist_insert_after_mark    ( GlyphsList glist, InsertionGlyphs insertion )
  {
    FT_ListNode before, after;
    
    before = (FT_ListNode)glist->mark; 
    FT_ASSERT( before );
    after = before->next;
    return glist_insert(glist, before, after, insertion);
  }

  static FT_Error
  glist_insert               ( GlyphsList glist, 
			       FT_ListNode before, 
			       FT_ListNode after, 
			       InsertionGlyphs insertion )
  {
    FT_Error error;
    FT_ListRec insertion_list = {NULL, NULL};
    FT_Memory memory 	= glist->memory;
    FT_ListNode head, tail;
    
    if (( error = insertion_glyphs_store( insertion,
					  memory,
					  &insertion_list ) ))
      return error;
    head = insertion_list.head;
    tail = insertion_list.tail;

    if ( (!head) && (!tail) )
      return FT_Err_Ok;
    
    FT_ASSERT( head );
    FT_ASSERT( tail );

    
    if ( before )
      {
	before->next = head;
	head->prev   = before;
      }
    else
      {
	glist->root.head = head;
	head->prev 	 = NULL;
      }

    if ( after )
      {
	after->prev = tail;
	tail->next  = after;
      }
    else
      {
	glist->root.tail = tail;
	tail->next 	 = NULL;
      }
    return FT_Err_Ok;
  }

  static void
  glist_set_mark        ( GlyphsList glist )
  {
    glist->mark = glist->current;
  }

  static FTL_Glyph
  glist_get_next    ( GlyphsList glist )
  {
    if ( !glist->current )
      return NULL;
    
    glist->current = ( GlyphNode )glist->current->root.next;
    return glist_get_current ( glist );
  }

  static FTL_Glyph
  glist_get_current ( GlyphsList glist )
  {
    return ( glist->current )? &(glist->current->glyph): NULL;
  }
  

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                        Glyph Metamorphosis                      ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


/*************************************************************************
 * Noncontextual
 *************************************************************************/ 
FT_LOCAL_DEF( FT_Error )
gx_noncontextual_subst( GX_MetamorphosisNoncontextualBody body,
			FTL_Glyphs_Array garray )
{
  FT_ULong i;
  GX_LookupTable lookup_table = &body->lookup_table;
  GX_LookupResultRec result;
  
  FT_TRACE2(("Enter noncontextual\n"));
  for ( i = 0; i < garray->length; i++ )
    {
      if ( garray->glyphs[i].gid == GX_DELETED_GLYPH_INDEX )
	continue ;
      result = gx_LookupTable_lookup ( lookup_table, garray->glyphs[i].gid );
      if ( result.value == NULL )
	continue;
      
      if ( result.firstGlyph == GX_LOOKUP_RESULT_NO_FIRST_GLYPH )
	{
	  FT_TRACE3(("	Substitution[%lu] %u to %u\n",
		     i, garray->glyphs[i].gid, result.value->raw.u));
	  garray->glyphs[i].gid = result.value->raw.u;
	}
      else
	{
	  FT_TRACE3(( "	Substitution[%d] %u to %u\n", 
		     i, garray->glyphs[i].gid,
		      result.value->extra.word[garray->glyphs[i].gid - result.firstGlyph] ));
	  garray->glyphs[i].gid = result.value->extra.word[garray->glyphs[i].gid - result.firstGlyph];
	}
    }
  FT_TRACE2(("Leave noncontextual\n"));
  return FT_Err_Ok;
}

	
/*************************************************************************
 * Contextual
 *************************************************************************/   
static FT_UShort contextual_lookup_glyph( GX_MetamorphosisContextualBody body,
					  FT_Short index, /* Was:FT_UShort, see gxtype.h */
					  FT_UShort gid );

FT_LOCAL_DEF( FT_Error )
gx_contextual_subst( GX_MetamorphosisContextualBody body,
		     GXL_Initial_State initial_state,
		     FTL_Glyphs_Array garray )
{
  FT_ULong glength  = garray->length;
  FTL_Glyph glyphs = garray->glyphs;
  FT_UShort current_state;

  FT_ULong current_glyph, mark_glyph;
  FT_Byte class_code, final_class;
  GX_EntrySubtable entry_subtable;
  
  FT_Short markOffset, currentOffset; /* Was:FT_UShort, see gxtype.h */
  FT_UShort tmp;
  
  FT_TRACE2(("Enter contextual\n"));

  current_glyph = 0;
  mark_glyph 	= 0;
  current_state = body->state_table.header.stateArray + (FT_Byte)initial_state;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;
  
  while ( current_glyph <= glength )
    {
      FT_TRACE3(("  glyph id: %u, glyph position: %lu/%lu\n", 
		 current_glyph == glength? 0xFFFF: glyphs[current_glyph].gid,
		 current_glyph, glength ));

      /* glyph -> class */
      class_code = ( current_glyph == glength )
	? final_class: gx_StateTable_get_class ( &body->state_table, 
						 glyphs[current_glyph].gid );
      FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));
      
      /* class -> entry */
      entry_subtable = gx_StateTable_get_entry_subtable ( &body->state_table,
							  current_state, 
							  class_code );

      /* action for entry */
      currentOffset = entry_subtable->glyphOffsets.contextual->currentOffset;
      markOffset    = entry_subtable->glyphOffsets.contextual->markOffset;
      if ( currentOffset )
	{
	  FT_TRACE3((" currentOffset is given: %d\n", currentOffset)); /* Was:FT_UShort, see gxtype.h. */
	  tmp = glyphs[current_glyph].gid;
	  glyphs[current_glyph].gid = contextual_lookup_glyph(body, currentOffset, tmp);
	  FT_TRACE3(("	Substitution[current %lu] %u to %u\n",
		     current_glyph, tmp, glyphs[current_glyph].gid ));
	}
      if ( markOffset )
	{
	  FT_TRACE3((" markOffset is given: %d\n", markOffset)); /* Was:FT_UShort, see gxtype.h. */
	  tmp = glyphs[mark_glyph].gid;
	  glyphs[mark_glyph].gid = contextual_lookup_glyph(body, markOffset, tmp);
	  FT_TRACE3(("	Substitution[current %lu] %u to %u\n",
		     mark_glyph, tmp, glyphs[current_glyph].gid ));
	}
      
      /* set mark */
      if ( entry_subtable->flags & GX_MORT_CONTEXTUAL_FLAGS_SET_MARK )
	{
	  mark_glyph = current_glyph;
	  FT_TRACE3(("	%ld is marked.\n", mark_glyph ));
	}

      /* advance */
      if ( !(entry_subtable->flags & GX_MORT_CONTEXTUAL_FLAGS_DONT_ADVANCE ) )
	{
	  FT_TRACE3(("	index++.\n"));
	  current_glyph++;
	}

      /* new state */
      current_state = entry_subtable->newState;
    }
  FT_TRACE2(("Leave contextual\n"));
  return FT_Err_Ok;
}

static FT_UShort
contextual_lookup_glyph( GX_MetamorphosisContextualBody body,
			 FT_Short offset, /* Was:FT_UShort, see gxtype.h */
			 FT_UShort gid )
{
  GX_MetamorphosisContextualSubstitutionTable substitution_table = &body->substitutionTable;
  FT_ULong index;
  
  FT_TRACE2(("offset: %u substitution_table->offset %u\n",
	     offset, substitution_table->offset ));
  index = (2 * offset - substitution_table->offset)/2 + gid;
  FT_TRACE2(("index: %lu < substitution_table->nGlyphIndexes: %lu\n",
	     index, substitution_table->nGlyphIndexes));
  FT_ASSERT( index < substitution_table->nGlyphIndexes );
  return substitution_table->glyph_indexes[index];
}

/*************************************************************************
 * Ligature
 *************************************************************************/ 
static FT_ULong * ligature_get_action( GX_MetamorphosisLigatureBody body,
				       FT_UShort action_offset );
static FT_UShort ligature_get_component( GX_MetamorphosisLigatureBody body,
					 FT_Long component_offset,
					 FT_UShort gid );

static FT_UShort ligature_get_ligature ( GX_MetamorphosisLigatureBody body,
					 FT_ULong action,
					 FT_Long  accumulator );
					 
FT_LOCAL_DEF( FT_Error )
gx_ligature_subst( GX_MetamorphosisLigatureBody body,
		   GXL_Initial_State initial_state,
		   FTL_Glyphs_Array garray )
{
  FT_ULong glength  = garray->length;
  FTL_Glyph glyphs = garray->glyphs;
  FT_UShort current_state;

  FT_ULong current_glyph, component_glyph;
  FT_Byte class_code, final_class;
  GX_EntrySubtable entry_subtable;

  LigatureStackRec ligstack;
  FT_UShort action_offset;
  FT_ULong *action;

  FT_UShort component;
  FT_Long component_offset;
  FT_Long component_accumulator;
  FT_UShort tmp;
  
  FT_TRACE2(("Enter ligature\n"));
  
  current_glyph = 0;
  current_state = body->state_table.header.stateArray + (FT_Byte)initial_state;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;
  ligstack_init(&ligstack);
  while ( current_glyph <= glength )
    {
      FT_TRACE3(("  glyph id: %u, glyph position: %lu/%lu\n", 
		 current_glyph == glength? 0xFFFF: glyphs[current_glyph].gid,
		 current_glyph, glength ));

      /* glyph -> class */
      class_code = ( current_glyph == glength )
	? final_class: gx_StateTable_get_class ( &body->state_table, 
						 glyphs[current_glyph].gid );
      FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));
      
      /* class -> entry */
      entry_subtable = gx_StateTable_get_entry_subtable ( &body->state_table,
							  current_state, 
							  class_code );

      /* action for entry */
      if ( entry_subtable->flags & GX_MORT_LIGATURE_FLAGS_SET_COMPONENT )
	{
	  FT_TRACE3(("	Push[current_glyph %lu] %lu to the stack\n",
		     current_glyph, glyphs[current_glyph] ));
	  ligstack_push(&ligstack, current_glyph);
	}
      
      /* action */
      action_offset = entry_subtable->flags & GX_MORT_LIGATURE_FLAGS_OFFSET;
      if ( action_offset ) 
	{
	  action = ligature_get_action( body, action_offset ) - 1;
	  component_accumulator = 0;
	  ligstack_ligatured_init( &ligstack );

	  do {
	    action++;
	    component_glyph  = ligstack_pop( &ligstack );
	    component_offset = (*action) & GX_MORT_LIGATURE_ACTION_OFFSET;
	    if ( component_offset ) 
	      {
		component = ligature_get_component( body, 
						    component_offset,
						    glyphs[component_glyph].gid );
		component_accumulator += component;
		tmp = ligature_get_ligature( body, 
					     *action,
					     component_accumulator );
		FT_TRACE3(("	Substitution[component %lu] %u to %u\n",
			   component_glyph, glyphs[component_glyph].gid, tmp ));
		glyphs[component_glyph].gid = tmp; 
		if (*action & ( GX_MORT_LIGATURE_ACTION_LAST | 
				GX_MORT_LIGATURE_ACTION_STORE ))
		    {
		      ligstack_ligatured_push( &ligstack, component_glyph );
		      component_accumulator = 0;
		    }
	      }
	  } while (! ((*action) & GX_MORT_LIGATURE_ACTION_LAST));
	  ligstack_unify ( &ligstack );
	}
      
      /* advance */
      if ( !(entry_subtable->flags & GX_MORT_LIGATURE_FLAGS_DONT_ADVANCE ) )
	{
	  FT_TRACE3(("	index++.\n"));
	  current_glyph++;
	}
      
      /* new state */
      current_state = entry_subtable->newState;
    }
  FT_TRACE2(("Leave ligature\n"));
  return FT_Err_Ok;
}

static FT_ULong *
ligature_get_action( GX_MetamorphosisLigatureBody body, 
		     FT_UShort action_offset )
{
  FT_UShort action_index;
  GX_MetamorphosisLigatureActionTable action_table = &body->ligActionTable;
  FT_ASSERT( ((action_offset - action_table->offset)%4 == 0) );
  action_index = (action_offset - action_table->offset)/4;
  return &(action_table->body[action_index]);
}

static FT_UShort
ligature_get_component( GX_MetamorphosisLigatureBody body,
			FT_Long component_offset,
			FT_UShort gid )
{
  FT_Long component_index;
  GX_MetamorphosisComponentTable component_table = &body->componentTable;
  
  component_offset = gx_sign_extend ( component_offset,
				      GX_MORT_LIGATURE_ACTION_OFFSET );
  component_index = (2 * component_offset - component_table->offset)/2 + gid;
  FT_ASSERT ( component_index < component_table->nComponent );
  return component_table->body[component_index];
}

static FT_UShort
ligature_get_ligature ( GX_MetamorphosisLigatureBody body,
			FT_ULong action,
			FT_Long  accumulator )
{
  FT_Long ligature_index;
  GX_MetamorphosisLigatureTable ligature_table = &body->ligatureTable;
  
  ligature_index = (accumulator - ligature_table->offset)/2;
  FT_ASSERT ( ligature_index < ligature_table->nLigature );

  if (action & ( GX_MORT_LIGATURE_ACTION_LAST
		 | GX_MORT_LIGATURE_ACTION_STORE ))
    return ligature_table->body[ligature_index];
  else
    return GX_DELETED_GLYPH_INDEX;
}

/*************************************************************************
 * Insertion
 *************************************************************************/ 

static FT_Error
insertion_insert ( GX_MetamorphosisInsertionList insertion,
		   FT_UShort  flags,
		   FT_UShort  count_mask,
		   FT_UShort  pos_mask,
		   GList_InsertFunc insert,
		   GlyphsList glist );

/* TODO: insert Tracer */
FT_LOCAL_DEF( FT_Error )
gx_insertion_subst( GX_MetamorphosisInsertionBody body,
		    GXL_Initial_State initial_state,
		    FTL_Glyphs_Array garray )
{
  FT_Error error;
  FT_Memory memory = garray->memory;
  
  GlyphsListRec glist;
  FTL_Glyph current_glyph;
  FT_UShort current_state;
  
  FT_Byte class_code, final_class;
  GX_EntrySubtable entry_subtable;
  
  GX_MetamorphosisInsertionList current_insertion;
  GX_MetamorphosisInsertionList marked_insertion;
  
  FT_TRACE2(("Enter insertion\n"));
  
  if (( error = glist_init(memory, &glist, garray) ))
    return error;

  current_state = body->state_table.header.stateArray + (FT_Byte)initial_state;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;
  
  do {
    current_glyph = glist_get_current(&glist);
    
    /* glyph -> class */
    class_code  = ( current_glyph == NULL) 
      ? final_class: gx_StateTable_get_class( &body->state_table,
					      current_glyph->gid );
    FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));
    
    /* class -> entry */
    entry_subtable = gx_StateTable_get_entry_subtable( &body->state_table,
						       current_state,
						       class_code );
    /* action for entry */
    current_insertion = &entry_subtable->glyphOffsets.insertion->currentInsertList;
    marked_insertion  = &entry_subtable->glyphOffsets.insertion->markedInsertList;
    if ( current_insertion->offset )
      {
	if (( error = insertion_insert ( current_insertion,
					 entry_subtable->flags,
					 GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_COUNT,
					 GX_MORT_INSERTION_FLAGS_CURRENT_INSERT_BEFORE,
					 glist_insert_current,
					 &glist ) ))
	  goto Failure;
      }
    
    if ( marked_insertion->offset )
      {
	if (( error = insertion_insert ( marked_insertion,
					 entry_subtable->flags,
					 GX_MORT_INSERTION_FLAGS_MARKED_INSERT_COUNT,
					 GX_MORT_INSERTION_FLAGS_MARKED_INSERT_BEFORE,
					 glist_insert_mark,
					 &glist ) ))
	  goto Failure;
      }
    
    /* set mark */
    if ( entry_subtable->flags & GX_MORT_INSERTION_FLAGS_SET_MARK )
      glist_set_mark( &glist );
    
    /* advance */
    if (!( entry_subtable->flags & GX_MORT_INSERTION_FLAGS_DONT_ADVANCE ))
      (void)glist_get_next( &glist );

    /* new state */
    current_state = entry_subtable->newState;
  } while ( current_glyph );
  
  FT_TRACE2(("Leave insertion\n"));
  if (( error = glist_store ( &glist, garray ) ))
    goto Failure;
  glist_finalize( &glist );
  return error;
 Failure:
  glist_finalize( &glist );
  return error;
}

static FT_Error
insertion_insert ( GX_MetamorphosisInsertionList insertion,
		   FT_UShort  flags,
		   FT_UShort  count_mask,
		   FT_UShort  pos_mask,
		   GList_InsertFunc insert,
		   GlyphsList glist )
{
  FT_Int count, i;
  FT_Bool before;
  InsertionGlyphsRec glyphs;
  
  
  count = gx_mask_zero_shift(flags, count_mask);
  before = (flags & pos_mask) ? 1: 0;
  
  insertion_glyphs_init ( &glyphs );
  for ( i = 0; i < count; i++ )
    insertion_glyphs_push( &glyphs, insertion->glyphcodes[i] );
  /* TODO: kashida like or split vowels */
  return insert( glist, &glyphs, before );
}

/*************************************************************************
 * Rearrangement
 *************************************************************************/ 
static void rearrangement_rearrange( GX_MetamorphosisRearrangementVerb verb,
				     FT_ULong first_glyph,
				     FT_ULong last_glyph,
				     FTL_Glyphs_Array garray );
FT_LOCAL( FT_Error )
gx_rearrangement_subst ( GX_MetamorphosisRearrangementBody body,
			 GXL_Initial_State initial_state,
			 FTL_Glyphs_Array garray )
{
  FT_ULong glength  = garray->length;
  FTL_Glyph glyphs = garray->glyphs;
  FT_UShort current_state;
  
  FT_ULong current_glyph, first_glyph, last_glyph;
  FT_Byte class_code, final_class;
  GX_EntrySubtable entry_subtable;

  GX_MetamorphosisRearrangementVerb verb;
  
  FT_TRACE2(("Enter rearrangement\n"));
  current_glyph = 0;
  first_glyph 	= 0;
  last_glyph 	= 0;
  current_state = body->state_table.header.stateArray + (FT_Byte)initial_state;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;

  while ( current_glyph <= glength )
    {
      FT_TRACE3(("  glyph id: %u, glyph position: %lu/%lu\n", 
		 current_glyph == glength? 0xFFFF: glyphs[current_glyph].gid,
		 current_glyph, glength ));

      /* glyph -> class */
      class_code = ( current_glyph == glength )
	? final_class: gx_StateTable_get_class ( &body->state_table, 
						 glyphs[current_glyph].gid ); 
      FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));

      /* class -> entry */
      entry_subtable = gx_StateTable_get_entry_subtable ( &body->state_table,
							  current_state, 
							  class_code );
      /* action for entry */
      if ( entry_subtable->flags & GX_MORT_REARRANGEMENT_FLAGS_MARK_FIRST )
	{
	  FT_TRACE3(( "	set %lu as first\n", current_glyph ));
	  first_glyph = current_glyph;
	}
      if ( entry_subtable->flags & GX_MORT_REARRANGEMENT_FLAGS_MARK_LAST )
	{
	  FT_TRACE3(( "	set %lu as last\n", current_glyph ));
	  last_glyph = current_glyph;
	}
      
      /* rearrange */
      verb = entry_subtable->flags & GX_MORT_REARRANGEMENT_FLAGS_VERB;
      FT_TRACE3(("	verb %d\n", verb));
      rearrangement_rearrange( verb, first_glyph, last_glyph, garray );
      
      /* advance */
      if ( !(entry_subtable->flags & GX_MORT_REARRANGEMENT_FLAGS_DONT_ADVANCE) )
	{
	  FT_TRACE3(("  index++.\n"));
	  current_glyph++;
	}

      /* new state */
      current_state = entry_subtable->newState; 
    }
  FT_TRACE2(("Leave rearrangement\n"));
  return FT_Err_Ok;
}

static void
rearrangement_rearrange( GX_MetamorphosisRearrangementVerb verb,
			 FT_ULong first_glyph,
			 FT_ULong last_glyph,
			 FTL_Glyphs_Array garray )
{
  FTL_Glyph glyphs = garray->glyphs;
  FT_ULong glength  = garray->length;
  
  FTL_GlyphRec A, B, C, D;
  FT_ULong x;


  /* Treat VERB_NO_CHANGE as a special case; return
     before "first_glyph <= last_glyph" assertion. */  
  if ( verb == GX_MORT_REARRANGEMENT_VERB_NO_CHANGE )
    return ;
  
  FT_ASSERT ( first_glyph <= last_glyph );
  FT_ASSERT ( first_glyph < glength );
  FT_ASSERT ( last_glyph  < glength );
  
  switch ( verb )
    {
    case GX_MORT_REARRANGEMENT_VERB_Ax2xA:
      A = glyphs[first_glyph];
      for ( x = first_glyph + 1; x <= last_glyph; x++ )
	glyphs[x - 1] 	 = glyphs[x];
      glyphs[last_glyph] = A;
      break;
      
    case GX_MORT_REARRANGEMENT_VERB_xD2Dx:
      D = glyphs[last_glyph];
      for ( x = last_glyph - 1; x >= first_glyph; x-- )
	glyphs[x + 1] = glyphs[x];
      glyphs[first_glyph] = D;
      break;
      
    case GX_MORT_REARRANGEMENT_VERB_AxD2DxA:
      A 		  = glyphs[first_glyph];
      D 		  = glyphs[last_glyph];
      glyphs[first_glyph] = D;
      glyphs[last_glyph]  = A;
      break;

    case GX_MORT_REARRANGEMENT_VERB_ABx2xAB:
      FT_ASSERT( first_glyph + 1 < glength );
      FT_ASSERT( first_glyph + 2 <= last_glyph );
      FT_ASSERT( last_glyph - 1  >= 0 );
      
      A = glyphs[first_glyph];
      B = glyphs[first_glyph + 1];
      for ( x = first_glyph + 2; x <= last_glyph; x++ )
	glyphs[x - 2] 	     = glyphs[x];
      glyphs[last_glyph - 1] = A;
      glyphs[last_glyph]     = B;
      break;
      
    case GX_MORT_REARRANGEMENT_VERB_ABx2xBA:
      FT_ASSERT( first_glyph + 1 < glength );
      FT_ASSERT( first_glyph + 2 <= last_glyph );
      FT_ASSERT( last_glyph - 1  >= 0 );
      
      A = glyphs[first_glyph];
      B = glyphs[first_glyph + 1];
      for ( x = first_glyph + 2; x <= last_glyph; x++ )
	glyphs[x - 2] = glyphs[x];
      
      glyphs[last_glyph - 1] = B;
      glyphs[last_glyph]     = A;
      break;

    case GX_MORT_REARRANGEMENT_VERB_xCD2CDx:
      FT_ASSERT( last_glyph - 1 >= 0 );
      FT_ASSERT( first_glyph 	<= last_glyph - 2 );
      FT_ASSERT( first_glyph + 1 < glength );
      C = glyphs[last_glyph - 1];
      D = glyphs[last_glyph];
      for ( x = last_glyph - 2; x >= first_glyph; x-- )
	glyphs[x + 2] 	  =  glyphs[x];

      glyphs[first_glyph]     = C;
      glyphs[first_glyph + 1] = D;
      break;
      
    case GX_MORT_REARRANGEMENT_VERB_xCD2DCx:
      FT_ASSERT( last_glyph - 1 >= 0 );
      FT_ASSERT( first_glyph 	<= last_glyph - 2 );
      FT_ASSERT( first_glyph + 1 < glength );
      C = glyphs[last_glyph - 1];
      D = glyphs[last_glyph];
      for ( x = last_glyph - 2; x >= first_glyph; x-- )
	glyphs[x + 2] 	  =  glyphs[x];

      glyphs[first_glyph]     = D;
      glyphs[first_glyph + 1] = C;
      break;

    case GX_MORT_REARRANGEMENT_VERB_AxCD2CDxA:
      FT_ASSERT( last_glyph - 2 >= 0 );
      FT_ASSERT( first_glyph + 1 < glength );
      A = glyphs[first_glyph];
      C = glyphs[last_glyph - 1];
      D = glyphs[last_glyph];
      for ( x = last_glyph - 2; x > first_glyph; x-- )
	glyphs[x + 1] = glyphs[x];
      glyphs[first_glyph] = C;
      glyphs[first_glyph + 1] = D;
      glyphs[last_glyph]      = A;
      break;
      
    case GX_MORT_REARRANGEMENT_VERB_AxCD2DCxA:
      FT_ASSERT( last_glyph - 2 >= 0 );
      FT_ASSERT( first_glyph + 1 < glength );
      A = glyphs[first_glyph];
      C = glyphs[last_glyph - 1];
      D = glyphs[last_glyph];
      
      for ( x = last_glyph - 2; x > first_glyph; x-- )
	glyphs[x + 1] = glyphs[x];
      
      glyphs[first_glyph]     = D;
      glyphs[first_glyph + 1] = C;
      glyphs[last_glyph]      = A;
      break;
      
    case GX_MORT_REARRANGEMENT_VERB_ABxD2DxAB:
      FT_ASSERT ( first_glyph + 2 < glength );
      FT_ASSERT ( last_glyph - 1 >= 0 );
      A = glyphs[first_glyph];
      B = glyphs[first_glyph + 1];
      D	= glyphs[last_glyph];
      for ( x = first_glyph + 2; x < last_glyph; x++ )
	glyphs[x - 2] 	     = glyphs[x];
      glyphs[first_glyph]    = D;
      glyphs[last_glyph - 1] = A;
      glyphs[last_glyph]     = B;
      break;
      
    case GX_MORT_REARRANGEMENT_VERB_ABxD2DxBA:
      FT_ASSERT ( first_glyph + 2 < glength );
      FT_ASSERT ( last_glyph - 1 >= 0 );
      A = glyphs[first_glyph];
      B = glyphs[first_glyph + 1];
      D	= glyphs[last_glyph];
      for ( x = first_glyph + 2; x < last_glyph; x++ )
	glyphs[x - 2] 	     = glyphs[x];
      glyphs[first_glyph]    = D;
      glyphs[last_glyph - 1] = B;
      glyphs[last_glyph]     = A;
      break;

    case GX_MORT_REARRANGEMENT_VERB_ABxCD2CDxAB:
      FT_ASSERT( first_glyph + 1 < glength );
      FT_ASSERT( last_glyph - 1 >= 0 );
      A = glyphs[first_glyph];
      B = glyphs[first_glyph + 1];
      C = glyphs[last_glyph - 1];
      D = glyphs[last_glyph];
      glyphs[first_glyph]     = C;
      glyphs[first_glyph + 1] = D;
      glyphs[last_glyph - 1]  = A;
      glyphs[last_glyph]      = B;
      break;

    case GX_MORT_REARRANGEMENT_VERB_ABxCD2CDxBA:
      FT_ASSERT( first_glyph + 1 < glength );
      FT_ASSERT( last_glyph - 1 >= 0 );
      A = glyphs[first_glyph];
      B = glyphs[first_glyph + 1];
      C = glyphs[last_glyph - 1];
      D = glyphs[last_glyph];
      glyphs[first_glyph]     = C;
      glyphs[first_glyph + 1] = D;
      glyphs[last_glyph - 1]  = B;
      glyphs[last_glyph]      = A;
      break;

    case GX_MORT_REARRANGEMENT_VERB_ABxCD2DCxAB:
      FT_ASSERT( first_glyph + 1 < glength );
      FT_ASSERT( last_glyph - 1 >= 0 );
      A = glyphs[first_glyph];
      B = glyphs[first_glyph + 1];
      C = glyphs[last_glyph - 1];
      D = glyphs[last_glyph];
      glyphs[first_glyph]     = D;
      glyphs[first_glyph + 1] = C;
      glyphs[last_glyph - 1]  = A;
      glyphs[last_glyph]      = B;
      break;
    case GX_MORT_REARRANGEMENT_VERB_ABxCD2DCxBA:
      FT_ASSERT( first_glyph + 1 < glength );
      FT_ASSERT( last_glyph - 1 >= 0 );
      A = glyphs[first_glyph];
      B = glyphs[first_glyph + 1];
      C = glyphs[last_glyph - 1];
      D = glyphs[last_glyph];
      glyphs[first_glyph]     = D;
      glyphs[first_glyph + 1] = C;
      glyphs[last_glyph - 1]  = B;
      glyphs[last_glyph]      = A;
      break;
    default:
      break;			/* TODO: Broken verb */
    }
}



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                   Extended Glyph Metamorphosis                  ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

/*************************************************************************
 * Contextual
 *************************************************************************/ 

static FT_UShort xcontextual_lookup_glyph( GX_XMetamorphosisContextualBody body,
					   FT_UShort lookup_table_index,
					   FT_UShort glyph );

FT_LOCAL_DEF( FT_Error )
gx_xcontextual_subst( GX_XMetamorphosisContextualBody body,
		      GXL_Initial_State initial_state,
		      FTL_Glyphs_Array garray )
{
  
  FT_ULong glength  = garray->length;
  FTL_Glyph glyphs = garray->glyphs;
  FT_UShort current_state;
  
  FT_ULong current_glyph, mark_glyph;
  FT_UShort class_code, final_class;
  GX_EntrySubtable entry_subtable;

  FT_UShort markIndex, currentIndex;
  FT_UShort tmp;
  
  FT_TRACE2(("Enter xcontextual\n"));
  
  current_glyph = 0;
  mark_glyph 	= 0;
  current_state = 0;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;
  
  while  ( current_glyph <= glength )
    {
      FT_TRACE3(("  glyph id: %u, glyph position: %lu/%lu\n", 
		 current_glyph == glength? 0xFFFF: glyphs[current_glyph].gid,
		 current_glyph, glength ));

      
      /* glyph -> class */
      class_code = ( current_glyph == glength )
	? final_class: gx_XStateTable_get_class ( &body->state_table,
						  glyphs[current_glyph].gid );
      FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));
      
      /* class -> entry */
      entry_subtable = gx_XStateTable_get_entry_subtable ( &body->state_table,
							   current_state, 
							   class_code );
      
      currentIndex = entry_subtable->glyphOffsets.contextual->currentOffset;
      markIndex    = entry_subtable->glyphOffsets.contextual->markOffset;
      if ( currentIndex != GX_MORX_NO_SUBSTITUTION )
	{
	  tmp = glyphs[current_glyph].gid;
	  glyphs[current_glyph].gid = xcontextual_lookup_glyph( body,
								currentIndex,
							        glyphs[current_glyph].gid );
	  FT_TRACE3(("	Substitution[current %lu] %u to %u\n",
		     current_glyph, tmp, glyphs[current_glyph].gid ));
	}
      if ( markIndex != GX_MORX_NO_SUBSTITUTION )
	{
	  tmp = glyphs[mark_glyph].gid;
	  glyphs[mark_glyph].gid = xcontextual_lookup_glyph( body,
							     markIndex,
							     glyphs[mark_glyph].gid );
	  FT_TRACE3(("	Substitution[mark %lu] %u to %u\n",
		     mark_glyph, tmp, glyphs[mark_glyph].gid ));
	}

      if ( entry_subtable->flags & GX_MORX_CONTEXTUAL_FLAGS_SET_MARK )
	{
	  mark_glyph = current_glyph;
	  FT_TRACE3(("	%ld is marked.\n", mark_glyph ));
	}

      /* advance */
      if  ( !(entry_subtable->flags & GX_MORX_CONTEXTUAL_FLAGS_DONT_ADVANCE ) )
	{
	  FT_TRACE3(("	index++.\n"));
	  current_glyph++;
	}
      current_state = entry_subtable->newState;
    }
  FT_TRACE2(("Leave contextual\n"));
  return FT_Err_Ok;
}

static FT_UShort
xcontextual_lookup_glyph( GX_XMetamorphosisContextualBody body,
			  FT_UShort lookup_table_index,
			  FT_UShort glyph )
{
  GX_LookupTable * lookupTables;
  GX_LookupResultRec result;
  FT_ASSERT( lookup_table_index < body->substitutionTable.nTables );
  
  lookupTables = body->substitutionTable.lookupTables;
  result       = gx_LookupTable_lookup( lookupTables[lookup_table_index], glyph );
  if ( result.value == NULL )
    return glyph;		/* ??? */
  else if ( result.firstGlyph == GX_LOOKUP_RESULT_NO_FIRST_GLYPH )
    return result.value->raw.u;
  else
    return result.value->extra.word[glyph - result.firstGlyph];
}

/*************************************************************************
 * Ligature
 *************************************************************************/ 

static FT_ULong * xligature_get_action( GX_XMetamorphosisLigatureBody body, 
					FT_UShort action_index );
static FT_UShort  xligature_get_component( GX_XMetamorphosisLigatureBody body,
					   FT_Long component_index_base,
					   FT_UShort gid );
static FT_UShort xligature_get_ligature ( GX_XMetamorphosisLigatureBody body,
					  FT_ULong action,
					  FT_Long  ligature_index );

FT_LOCAL_DEF( FT_Error )
gx_xligature_subst( GX_XMetamorphosisLigatureBody body,
		    GXL_Initial_State initial_state,
		    FTL_Glyphs_Array garray )
{
  FT_ULong glength  = garray->length;
  FTL_Glyph glyphs = garray->glyphs;
  FT_UShort current_state;

  FT_ULong current_glyph, component_glyph;
  FT_UShort class_code, final_class;
  GX_EntrySubtable entry_subtable;

  LigatureStackRec ligstack;
  FT_UShort action_index;
  FT_ULong *action;

  FT_UShort component;
  FT_Long component_index_base;
  FT_Long component_accumulator;
  FT_UShort tmp;
  
  FT_TRACE2(("Enter xligature\n"));
  
  current_glyph = 0;
  current_state = 0;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;

  ligstack_init( &ligstack );
  while ( current_glyph <= glength )
    {
      FT_TRACE3(("  glyph id: %u, glyph position: %lu/%lu\n", 
		 current_glyph == glength? 0xFFFF: glyphs[current_glyph].gid,
		 current_glyph, glength ));

      /* glyph -> class */
      class_code = ( current_glyph == glength )
	? final_class: gx_XStateTable_get_class ( &body->state_table, 
						  glyphs[current_glyph].gid );
      FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));
      
      /* class -> entry */
      entry_subtable = gx_XStateTable_get_entry_subtable ( &body->state_table,
							   current_state, 
							   class_code );
      
      /* action for entry */
      if ( entry_subtable->flags & GX_MORX_LIGATURE_FLAGS_SET_COMPONENT )
	{
	  FT_TRACE3(("	Push[current_glyph %lu] %lu to the stack\n",
		     current_glyph, glyphs[current_glyph] ));
	  ligstack_push(&ligstack, current_glyph);
	}

      if ( entry_subtable->flags & GX_MORX_LIGATURE_FLAGS_PERFORM_ACTION )
	{
	  action_index = entry_subtable->glyphOffsets.ligActionIndex;
	  action = xligature_get_action( body,  action_index ) - 1;
	  component_accumulator = 0;
	  ligstack_ligatured_init( &ligstack );

	  do {
	    action++;
	    component_glyph  = ligstack_pop( &ligstack );
	    FT_TRACE3(("	Pop[component_glyph %lu] %u from the stack\n",
		       component_glyph, glyphs[component_glyph].gid ));
	    FT_TRACE3(("	Action[last %d, store %d]\n",
		       (*action & ( GX_MORX_LIGATURE_ACTION_LAST ))?1:0,
		       (*action & ( GX_MORX_LIGATURE_ACTION_STORE ))?1:0 ));
	    component_index_base = (*action) & GX_MORX_LIGATURE_ACTION_OFFSET;
	    component = xligature_get_component( body, 
						 component_index_base,
						 glyphs[component_glyph].gid );
	    component_accumulator += component;
	    tmp = xligature_get_ligature( body, 
					  *action,
					  component_accumulator );
	    FT_TRACE3(("	Substitution[component %lu] %u to %u\n",
		       component_glyph, glyphs[component_glyph].gid, tmp ));
	    glyphs[component_glyph].gid = tmp; 
	    if (*action & ( GX_MORX_LIGATURE_ACTION_LAST |
			    GX_MORX_LIGATURE_ACTION_STORE ))
	      {
		ligstack_ligatured_push( &ligstack, component_glyph );
		component_accumulator = 0;
	      }
	  } while (! ((*action) & GX_MORX_LIGATURE_ACTION_LAST));
	  ligstack_unify ( &ligstack );
	}
      
      /* advance */
      if ( !(entry_subtable->flags & GX_MORX_LIGATURE_FLAGS_DONT_ADVANCE ) )
	{
	  FT_TRACE3(("	index++.\n"));
	  current_glyph++;
	}
      
      /* new state */
      current_state = entry_subtable->newState;
    }
  FT_TRACE2(("Leave xligature\n"));
  return FT_Err_Ok;
}

static FT_ULong *
xligature_get_action( GX_XMetamorphosisLigatureBody body, 
		       FT_UShort action_index )
{
  GX_XMetamorphosisLigatureActionTable action_table = &body->ligActionTable;
  FT_ASSERT( (action_index < action_table->nActions ) );
  return &(action_table->body[action_index]);
}

static FT_UShort
xligature_get_component( GX_XMetamorphosisLigatureBody body,
			 FT_Long component_index_base,
			 FT_UShort gid )
{
  FT_Long component_index;
  GX_XMetamorphosisComponentTable component_table = &body->componentTable;
  component_index = gx_sign_extend ( component_index_base,
				     GX_MORX_LIGATURE_ACTION_OFFSET )
    + gid;
  FT_ASSERT ( component_index < component_table->nComponent );
  return component_table->body[component_index];
}

static FT_UShort
xligature_get_ligature ( GX_XMetamorphosisLigatureBody body,
			 FT_ULong action,
			 FT_Long  ligature_index )
{
  GX_XMetamorphosisLigatureTable ligature_table = &body->ligatureTable;

  FT_ASSERT ( ligature_index < ligature_table->nLigature );

  if (action & ( GX_MORT_LIGATURE_ACTION_LAST
		 | GX_MORT_LIGATURE_ACTION_STORE ))
    return ligature_table->body[ligature_index];
  else
    return GX_DELETED_GLYPH_INDEX;
}

/*************************************************************************
 * Insertion
 *************************************************************************/ 
#define xinsertion_insert insertion_insert

FT_LOCAL_DEF( FT_Error )
gx_xinsertion_subst( GX_XMetamorphosisInsertionBody body,
		     GXL_Initial_State initial_state,
		     FTL_Glyphs_Array garray )
{
  FT_Error error;
  FT_Memory memory = garray->memory;
  
  GlyphsListRec glist;
  FTL_Glyph current_glyph;
  FT_UShort current_state;  

  FT_Byte class_code, final_class;
  GX_EntrySubtable entry_subtable;  

  GX_MetamorphosisInsertionList current_insertion;
  GX_MetamorphosisInsertionList marked_insertion;

  
  FT_TRACE2(("Enter xinsertion\n"));

  if (( error = glist_init(memory, &glist, garray) ))
    return error;
  
  current_state = 0;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;

  do {
    current_glyph = glist_get_current(&glist);
    
    /* glyph -> class */
    class_code  = ( current_glyph == NULL) 
      ? final_class: gx_XStateTable_get_class( &body->state_table,
					       current_glyph->gid );
    FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));
    
    /* class -> entry */
    entry_subtable = gx_XStateTable_get_entry_subtable ( &body->state_table,
							 current_state, 
							 class_code );

    /* action for entry */
    current_insertion = &entry_subtable->glyphOffsets.insertion->currentInsertList;
    marked_insertion  = &entry_subtable->glyphOffsets.insertion->markedInsertList;
    if ( current_insertion->offset != GX_MORX_NO_INSERTION )
      {
	if (( error = xinsertion_insert( current_insertion,
					 entry_subtable->flags,
					 GX_MORX_INSERTION_FLAGS_CURRENT_INSERT_COUNT,
					 GX_MORX_INSERTION_FLAGS_CURRENT_INSERT_BEFORE,
					 glist_insert_current,
					 &glist )))
	  goto Failure;
      }
    if ( marked_insertion->offset != GX_MORX_NO_INSERTION )
      {
	if ((error = xinsertion_insert( marked_insertion,
					entry_subtable->flags,
					GX_MORX_INSERTION_FLAGS_MARKED_INSERT_COUNT,
					GX_MORX_INSERTION_FLAGS_MARKED_INSERT_BEFORE,
					glist_insert_mark,
					&glist )))
	  goto Failure;
      }
    /* set mark */
    if ( entry_subtable->flags & GX_MORX_INSERTION_FLAGS_SET_MARK )
      glist_set_mark( &glist );
    
    /* advance */
    if ( !(entry_subtable->flags & GX_MORX_INSERTION_FLAGS_DONT_ADVANCE) )
      (void)glist_get_next( &glist );

    /* new state */
    current_state = entry_subtable->newState;
  } while ( current_glyph );
  
  FT_TRACE2(("Leave xinsertion\n"));
  if (( error = glist_store ( &glist, garray ) ))
    goto Failure;
  glist_finalize( &glist );
  return error;
 Failure:
  glist_finalize( &glist );
  return error;
}

/*************************************************************************
 * Rearrangement
 *************************************************************************/ 
#define xrearrangement_rearrange rearrangement_rearrange

FT_LOCAL( FT_Error )
gx_xrearrangement_subst ( GX_XMetamorphosisRearrangementBody body,
			  GXL_Initial_State initial_state,
			  FTL_Glyphs_Array garray )

{
  FT_ULong glength  = garray->length;
  FTL_Glyph glyphs = garray->glyphs;
  FT_UShort current_state;
  
  FT_ULong current_glyph, first_glyph, last_glyph;
  FT_UShort class_code, final_class;
  GX_EntrySubtable entry_subtable;

  GX_XMetamorphosisRearrangementVerb verb;
  
  FT_TRACE2(("Enter xrearrangement\n"));

  current_glyph = 0;
  first_glyph 	= 0;
  last_glyph 	= 0;
  current_state = 0;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;

  while ( current_glyph <= glength )
    {
      FT_TRACE3(("  glyph id: %u, glyph position: %lu/%lu\n", 
		 current_glyph == glength? 0xFFFF: glyphs[current_glyph].gid,
		 current_glyph, glength ));

      /* glyph -> class */
      class_code = ( current_glyph == glength )
	? final_class: gx_XStateTable_get_class ( &body->state_table, 
						  glyphs[current_glyph].gid ); 
      FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));
      
      /* class -> entry */
      entry_subtable = gx_XStateTable_get_entry_subtable ( &body->state_table,
							   current_state, 
							   class_code );
      /* action for entry */
      if ( entry_subtable->flags & GX_MORX_REARRANGEMENT_FLAGS_MARK_FIRST )
	{
	  FT_TRACE3(( "	set %lu as first\n", current_glyph ));
	  first_glyph = current_glyph;
	}
      if ( entry_subtable->flags & GX_MORX_REARRANGEMENT_FLAGS_MARK_LAST )
	{
	  FT_TRACE3(( "	set %lu as last\n", current_glyph ));
	  last_glyph = current_glyph;
	}
      
      /* rearrange */
      verb = entry_subtable->flags & GX_MORX_REARRANGEMENT_FLAGS_VERB;
      FT_TRACE3(("	verb %d\n", verb));
      xrearrangement_rearrange( verb, first_glyph, last_glyph, garray );
      
      /* advance */
      if ( !(entry_subtable->flags & GX_MORX_REARRANGEMENT_FLAGS_DONT_ADVANCE) ) 
	{
	  FT_TRACE3(("  index++.\n"));
	  current_glyph++;
	}

      /* new state */
      current_state = entry_subtable->newState;      
    }
  FT_TRACE2(("Leave xrearrangement\n"));
  return FT_Err_Ok;
}


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                        Kerning Stack                            ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#define KERNING_STACK_DEPTH 8
typedef LigatureStackRec KerningStackRec;
typedef LigatureStack    KerningStack;
static void kernstack_init( KerningStack stack );
#define kernstack_push ligstack_push
#define kernstack_pop  ligstack_pop
static void
kernstack_init( KerningStack stack )
{
  FT_Int i;
  stack->depth = KERNING_STACK_DEPTH;
  stack->top = -1;
  for ( i = 0; i < stack->depth; i++ )
    stack->indexes[i]   = 0;
}

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                        Contextual Kerning                       ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

FT_LOCAL_DEF( FT_Error )
gx_contextual_kerning_calc ( GX_KerningSubtableFormat1Body kern_fmt1,
			     FTL_Glyphs_Array garray,
			     FTL_Direction dir,
			     FT_Bool cross_stream,
			     GXL_Initial_State initial_state,
			     FT_Vector * kerning )
{
  FT_ULong  glength = garray->length;
  FTL_Glyph glyphs  = garray->glyphs;
  FT_UShort current_state;
  
  FT_ULong current_glyph, applied_glyph;
  FT_Byte class_code, final_class;
  GX_EntrySubtable entry_subtable;

  KerningStackRec kernstack;
  FT_UShort value_offset;
  FT_UShort value_index;
  FT_FWord raw_value, value;
  FT_Pos total_cross_stream = 0;
  FT_Pos * target;

  
  current_glyph = 0;
  current_state = kern_fmt1->state_table.header.stateArray + (FT_Byte)initial_state;
  final_class = ( initial_state == GXL_START_OF_TEXT_STATE )
    ? GX_CLASS_END_OF_TEXT: GX_CLASS_END_OF_LINE;

  kernstack_init( &kernstack );
  while ( current_glyph <= glength )
    {
      FT_TRACE3(("  glyph id: %u, glyph position: %lu/%lu\n", 
		 current_glyph == glength? 0xFFFF: glyphs[current_glyph].gid,
		 current_glyph, glength ));

      /* glyph -> class */
      class_code = ( current_glyph == glength )
	? final_class: gx_StateTable_get_class ( &kern_fmt1->state_table,
						 glyphs[current_glyph].gid );
      FT_TRACE3(("  class code: %u current state: %u\n", class_code, current_state ));
      
      /* class -> entry */
      entry_subtable = gx_StateTable_get_entry_subtable ( &kern_fmt1->state_table,
							  current_state, 
							  class_code );

      /* action for entry */
      if ( entry_subtable->flags & GX_KERN_ACTION_PUSH )
	{
	  FT_TRACE3(("  Push[current_glyph %lu] %lu to the stack\n",
		     current_glyph, glyphs[current_glyph] ));
	  kernstack_push(&kernstack, current_glyph);
	}

      /* action */
      value_offset = entry_subtable->flags & GX_KERN_ACTION_VALUE_OFFSET;
      if ( value_offset )
	{
	  value_index = ( value_offset - kern_fmt1->valueTable ) 
	    / sizeof (*(kern_fmt1->values));
	  
	  FT_ASSERT ( value_index < kern_fmt1->nValues );

	  value_index--;
	  do {
	    value_index++;
	    FT_ASSERT ( value_index < kern_fmt1->nValues );
	    
	    raw_value     = kern_fmt1->values[value_index];
	    if ( raw_value & GX_KERN_VALUE_RESET_CROSS_STREAM )
	      {
		FT_TRACE3(("  Reset cross stream kerning\n"));
		value 		   = -total_cross_stream;
		total_cross_stream = 0;
	      }
	    else if (value == GX_KERN_VALUE_END_LIST)
	      value = 0;
	    else
	      {
		value = raw_value;
		if ( cross_stream )
		  total_cross_stream += value;
	      }
	    
	    applied_glyph = kernstack_pop( &kernstack );
	    if ( cross_stream )
	      {
		if ( dir == FTL_HORIZONTAL )
		  target = &kerning[applied_glyph + 1].y;
		else
		  target = &kerning[applied_glyph + 1].x;
	      }
	    else
	      {
		if ( dir == FTL_HORIZONTAL )
		  target = &kerning[applied_glyph + 1].x;
		else
		  target = &kerning[applied_glyph + 1].y;
	      }

	    *target =  value;
	  } while ( !( raw_value & GX_KERN_VALUE_END_LIST ) );
	}
 
      /* advance */
      if ( !(entry_subtable->flags & GX_KERN_ACTION_DONT_ADVANCE ) )
	{
	  FT_TRACE3(("  index++.\n"));
	  current_glyph++;
	}

      /* new state */
      current_state = entry_subtable->newState;
    }
  FT_TRACE2(("Leave contextual kerning\n"));
  return FT_Err_Ok;
}

/* END */
