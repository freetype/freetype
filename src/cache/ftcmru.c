#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_INTERNAL_MRU_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H

#include "ftcerror.h"


  FT_EXPORT_DEF( void )
  FTC_MruNode_Prepend( FTC_MruNode  *plist,
                       FTC_MruNode   node )
  {
    FTC_MruNode  first = *plist;

    if ( first )
    {
      FTC_MruNode  last = first->prev;

      last->next  = node;
      first->prev = node;
      node->prev  = last;
      node->next  = first;
    }
    else
    {
      node->next = node;
      node->prev = node;
    }
    *plist = node;
  }


  FT_EXPORT_DEF( void )
  FTC_MruNode_Up( FTC_MruNode  *plist,
                  FTC_MruNode   node )
  {
    FTC_MruNode  first = *plist;

    FT_ASSERT( first != NULL );

    if ( node != first )
    {
      FTC_MruNode  prev = node->prev;
      FTC_MruNode  next = node->next;
      FTC_MruNode  last;

      prev->next = next;
      next->prev = prev;

      last = first->prev;

      first->prev = node;
      last->next  = node;

      node->prev  = last;
      node->next  = first;

      *plist = node;
    }
  }


  FT_EXPORT( void )
  FTC_MruNode_Remove( FTC_MruNode  *plist,
                      FTC_MruNode   node )
  {
    FTC_MruNode  first = *plist;
    FTC_MruNode  prev, next;

    FT_ASSERT( first != NULL );

    next = node->next;
    prev = node->prev;

    if ( node == next )
    {
      FT_ASSERT( node == prev );
      FT_ASSERT( node == first );

      *plist = NULL;
    }
    else
    {
      prev->next = next;
      next->prev = prev;

      if ( node == first )
        *plist = next;
    }
    node->prev = NULL;
    node->next = NULL;
  }



  FT_EXPORT_DEF( void )
  FTC_MruList_Init( FTC_MruList       list,
                    FTC_MruListClass  clazz,
                    FT_UInt           max_nodes,
                    FT_Pointer        data,
                    FT_Memory         memory )
  {
    list->num_nodes = 0;
    list->max_nodes = max_nodes;
    list->nodes     = NULL;
    list->clazz     = *clazz;
    list->data      = data;
    list->memory    = memory;
  }




  FT_EXPORT( void )
  FTC_MruList_Reset( FTC_MruList  list )
  {
    FT_Memory    memory = list->memory;
    FTC_MruNode  first  = list->nodes;

    if ( first )
    {
      FTC_MruNode  node = first;
      FTC_MruNode  next;

      do
      {
        next = node->next;

        if ( list->clazz.node_done )
          list->clazz.node_done( node, list->data );

        FT_FREE( node );

        node = next;
      }
      while ( node != first );
    }
    list->nodes     = NULL;
    list->num_nodes = 0;
  }


  FT_EXPORT( void )
  FTC_MruList_Done( FTC_MruList  list )
  {
    FTC_MruList_Reset( list );
  }


  FT_EXPORT_DEF( void )
  FTC_MruList_Up( FTC_MruList  list,
                  FTC_MruNode  node )
  {
    FTC_MruNode_Up( &list->nodes, node );
  }


  FT_EXPORT_DEF( FTC_MruNode )
  FTC_MruList_Lookup( FTC_MruList  list,
                      FT_Pointer   key )
  {
    FTC_MruNode_CompareFunc  compare = list->clazz.node_compare;
    FTC_MruNode              node, first;

    first = list->nodes;
    node  = NULL;

    if ( first )
    {
      node = first;
      do
      {
        if ( compare( node, key ) )
          goto Exit;

        node = node->next;

      } while ( node != first );

      node = NULL;
    }
  Exit:
    return node;
  }



  FT_EXPORT_DEF( FT_Error )
  FTC_MruList_New( FTC_MruList    list,
                   FT_Pointer     key,
                   FTC_MruNode   *anode )
  {
    FT_Memory    memory = list->memory;
    FT_Error     error;
    FTC_MruNode  node;

    if ( list->max_nodes > 0 && list->num_nodes >= list->max_nodes )
    {
      FT_ASSERT( list->nodes != NULL );

      node = list->nodes->prev;  /* last node */

      if ( list->clazz.node_reset )
      {
        FTC_MruNode_Up( &list->nodes, node );

        error = list->clazz.node_reset( node, key, list->data );
        if ( !error )
          goto Exit;
      }

      FTC_MruNode_Remove( &list->nodes, node );
      list->num_nodes--;

      if ( list->clazz.node_done )
        list->clazz.node_done( node, list->data );
    }
    else
    {
      if ( FT_ALLOC( node, list->clazz.node_size ) )
        goto Exit;
    }
    error = list->clazz.node_init( node, key, list->data );
    if ( !error )
    {
      FTC_MruNode_Prepend( &list->nodes, node );
      list->num_nodes++;
      goto Exit;
    }

    if ( list->clazz.node_done )
      list->clazz.node_done( node, list->data );

    FT_FREE( node );

  Exit:
    *anode = node;
    return error;
  }



  FT_EXPORT_DEF( FT_Error )
  FTC_MruList_Get( FTC_MruList   list,
                   FT_Pointer    key,
                   FTC_MruNode  *anode )
  {
    FT_Error     error = 0;
    FTC_MruNode  node;

    node = FTC_MruList_Lookup( list, key );
    if ( node == NULL )
    {
      error = FTC_MruList_New( list, key, &node );
      if ( error )
        node = NULL;
    }
    *anode = node;
    return error;
  }


  FT_EXPORT_DEF( void )
  FTC_MruList_Remove( FTC_MruList   list,
                      FTC_MruNode   node )
  {
    FT_Memory    memory = list->memory;

    FT_ASSERT( list->nodes != NULL && list->num_nodes > 0 );

    FTC_MruNode_Remove( &list->nodes, node );
    list->num_nodes--;

    if ( list->clazz.node_done )
      list->clazz.node_done( node, list->data );

    FT_FREE( node );
  }


  FT_EXPORT_DEF( void )
  FTC_MruList_RemoveSelection( FTC_MruList              list,
                               FTC_MruNode_CompareFunc  select,
                               FT_Pointer               key )
  {
    FTC_MruNode   first = list->nodes;

    while ( first && select( first, key ) )
    {
      FTC_MruList_Remove( list, first );
      first = list->nodes;
    }

    if ( first )
    {
      FTC_MruNode  node = first->next;
      FTC_MruNode  next;

      while ( node != first )
      {
        next = node->next;

        if ( select( node, key ) )
          FTC_MruList_Remove( list, node );

        node = next;
      }
    }
  }

/* END */

