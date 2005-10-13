#ifndef __FT_CACHE_INTERNALS_H__
#define __FT_CACHE_INTERNALS_H__

#include <ft2build.h>
#include FT_CACHE_H

#define FTC_MAX_FACES_DEFAULT  2
#define FTC_MAX_SIZES_DEFAULT  4
#define FTC_MAX_BYTES_DEFAULT  200000L  /* ~200kByte by default */

/* maximum number of caches registered in a single manager
 */
#define FTC_MAX_CACHES         16


/* internal fields of the @FTC_ManagerRec structure
 */
#define  FTC_MANAGER_PRIVATE                         \
    FTC_Node            nodes_list;                  \
    FT_ULong            max_weight;                  \
                                                     \
    FTC_Cache           caches[FTC_MAX_CACHES];      \
    FT_UInt             num_caches;                  \
                                                     \
    FTC_MruListRec      faces;                       \
    FTC_MruListRec      sizes;                       \
                                                     \
    FT_Pointer          request_data;                \
    FTC_Face_Requester  request_face;                \
                                                     \
    FT_UInt             retry_depth;                 \
    FT_UInt             retry_num_nodes;             \
    FT_UInt             retry_num_faces;             \
    FT_UInt             retry_overflow;

#include FT_CACHE_INTERNAL_MANAGER_H

  FT_LOCAL( void )
  ftc_node_destroy( FTC_Node     node,
                    FTC_Manager  manager );

/* */

#endif /* __FT_CACHE_INTERNALS_H__ */
