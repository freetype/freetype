#include <ft2build.h>
#include FT_CACHE_H
#include FT_CACHE_INTERNAL_MRU_H
#include FT_INTERNAL_OBJECTS_H
#include FT_INTERNAL_DEBUG_H

#include "ftcerror.h"

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


  static void
  ftc_mrulist_free_nodes( FTC_MruList  list,
                          FTC_MruNode *plist )
  {
    FT_Memory  memory = list->memory;

    while ( *plist )
    {
      FTC_MruNode  node = *plist;

      *plist = node->next;

      if ( list->clazz.node_done )
        list->clazz.node_done( node, list->data );

      FT_FREE( node );
    }
  }


  FT_EXPORT( void )
  FTC_MruList_Reset( FTC_MruList  list )
  {
    ftc_mrulist_free_nodes( list, &list->nodes );
    list->num_nodes = 0;
  }


  FT_EXPORT( void )
  FTC_MruList_Done( FTC_MruList  list )
  {
    FTC_MruList_Reset( list );
  }


  FT_EXPORT( FT_Error )
  FTC_MruList_Lookup( FTC_MruList   list,
                      FT_Pointer    key,
                      FTC_MruNode  *anode )
  {
    FT_Memory                memory = list->memory;
    FTC_MruNode_CompareFunc  compare = list->clazz.node_compare;
    FTC_MruNode              *plast, *pnode, *pfirst;
    FTC_MruNode              node;
    FT_Error                 error = 0;

    pfirst = &list->nodes;
    plast  = pnode = pfirst;

    for (;;)
    {
      node = *pnode;
      if ( node == NULL )
        goto NewNode;
      if ( compare( node, key ) )
        break;
      plast = pnode;
      pnode = &node->next;
    }

    if ( node != *pfirst )
    {
      *pnode     = node->next;
      node->next = *pfirst;
      *pfirst    = node;
    }
    goto Exit;

  NewNode:
    if ( list->max_nodes > 0 && list->num_nodes >= list->max_nodes )
    {
      node = *plast;

      if ( node )
      {
        *plast = NULL;
        list->num_nodes--;

        if ( list->clazz.node_reset )
        {
          error = list->clazz.node_reset( node, key, list->data );
          if ( !error ) goto AddNode;
        }

        list->clazz.node_done( node, list->data );
      }
    }
    else if ( FT_ALLOC( node, list->clazz.node_size ) )
      goto Exit;

    error = list->clazz.node_init( node, key, list->data );
    if ( error )
    {
      if ( list->clazz.node_done )
        list->clazz.node_done( node, list->data );

      FT_FREE( node );
      goto Exit;
    }

  AddNode:
    node->next  = list->nodes;
    list->nodes = node;
    list->num_nodes++;

  Exit:
    *anode = node;
    return error;
  }


  FT_EXPORT_DEF( void )
  FTC_MruList_Remove( FTC_MruList   list,
                      FTC_MruNode   node )
  {
    FTC_MruNode  *pnode = &list->nodes;

    for ( ;; )
    {
      if ( *pnode == NULL )  /* should not happen !! */
      {
        FT_ERROR(( "%s: trying to remove unknown node !!\n",
                   "FTC_MruList_Remove" ));
        return;
      }

      if ( *pnode == node )
        break;

      pnode = &node->next;
    }

    *pnode     = node->next;
    node->next = NULL;
    list->num_nodes--;

    {
      FT_Memory  memory = list->memory;

      if ( list->clazz.node_done )
        list->clazz.node_done( node, list->data );

      FT_FREE( node );
    }
  }


  FT_EXPORT_DEF( void )
  FTC_MruList_RemoveSelection( FTC_MruList              list,
                               FTC_MruNode_CompareFunc  select,
                               FT_Pointer               key )
  {
    FTC_MruNode  *pnode = &list->nodes;
    FTC_MruNode   node, free = NULL;;

    if ( select )
    {
      for (;;)
      {
        FTC_MruNode  node = *pnode;

        if ( node == NULL )
          break;

        if ( select( node, key ) )
        {
          *pnode     = node->next;
          node->next = free;
          free       = node;
        }
        else
          pnode = &node->next;
      }
    }

    ftc_mrulist_free_nodes( list, &free );
  }

/* END */

