#include <ft2build.h>
#include FT_EXCEPT_H


  FT_BASE_DEF( void )
  ft_cleanup_stack_init( FT_CleanupStack  stack,
                         FT_Memory        memory )
  {
    stack->chunk = &stack->chunk_0;
    stack->top   = stack->chunk->items;
    stack->limit = stack->top + FT_CLEANUP_CHUNK_SIZE;
    stack->chunk_0.link = NULL;
    
    stack->memory = memory;
  }                         



  FT_BASE_DEF( void )
  ft_cleanup_stack_done( FT_CleanupStack  stack )
  {
    FT_Memory        memory = stack->memory;
    FT_CleanupChunk  chunk, next;
    
    for (;;)
    {
      chunk = stack->chunk;
      if ( chunk == &stack->chunk_0 )
        break;

      stack->chunk = chunk->link;
      
      FT_FREE( chunk );
    }
    
    stack->memory = NULL;
  }



  FT_BASE_DEF( void )
  ft_cleanup_stack_push( FT_CleanupStack  stack,
                         FT_Pointer       item,
                         FT_CleanupFunc   item_func,
                         FT_Pointer       item_data )
  {
    FT_CleanupItem  top;


    FT_ASSERT( stack && stack->chunk && stack->top );
    FT_ASSERT( item  && item_func );
    
    top = stack->top;
    
    top->item      = item;
    top->item_func = item_func;
    top->item_data = item_data;
    
    top ++;
    
    if ( top == stack->limit )
    {
      FT_CleanupChunk  chunk;
      FT_Error         error;
      
      if ( FT_ALLOC( chunk, stack->memory ) )
        ft_cleanup_stack_throw( stack, error );

      chunk->link  = stack->chunk;
      stack->chunk = chunk;
      stack->limit = chunk->items + FT_CLEANUP_CHUNK_SIZE;
      top          = chunk->items;
    }

    stack->top = top;
  }                         



  FT_BASE_DEF( void )
  ft_cleanup_stack_pop( FT_CleanupStack   stack,
                        FT_Int            destroy )
  {
    FT_CleanupItem  top;
    
    
    FT_ASSERT( stack && stack->chunk && stack->top );
    top = stack->top;
    
    if ( top == stack->chunk->items )
    {
      FT_CleanupChunk  chunk;
      
      chunk = stack->chunk;
      
      if ( chunk == &stack->chunk_0 )
      {
        FT_ERROR(( "cleanup.pop: empty cleanup stack !!\n" ));
        ft_cleanup_throw( stack, FT_Err_EmptyCleanupStack );
      }

      chunk = chunk->link;
      FT_QFree( stack->chunk, stack->memory );
      
      stack->chunk = chunk;
      stack->limit = chunk->items + FT_CLEANUP_CHUNK_SIZE;
      top          = stack->limit;
    }
    
    top --;
    
    if ( destroy )
      top->item_func( top->item, top->item_data );
    
    top->item      = NULL;
    top->item_func = NULL;
    top->item_data = NULL;
    
    stack->top = top;
  }



  FT_BASE_DEF( FT_CleanupItem )
  ft_cleanup_stack_peek( FT_CleanupStack  stack )
  {
    FT_CleanupItem   top;
    FT_CleanupChunk  chunk;


    FT_ASSERT( stack && stack->chunk && stack->top );
    
    top   = stack->top;
    chunk = stack->chunk;
    
    if ( top > chunk->items )
      top--;
    else
    {
      chunk = chunk->link;
      top   = NULL;
      if ( chunk != NULL )
        top = chunk->items + FT_CLEANUP_CHUNK_SIZE - 1;
    }
    return top;
  }


  FT_BASE_DEF( void )
  ft_cleanup_stack_throw( FT_CleanupStack  stack, FT_Error  error )
  {
  }


  FT_BASE_DEF( void )
  ft_xhandler_enter( FT_XHandler  xhandler,
                     FT_Memory    memory )
  {
    
  }



  FT_BASE_DEF( void )
  ft_xhandler_exit( FT_XHandler  xhandler )
  {
  }

