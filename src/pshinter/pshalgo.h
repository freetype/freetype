#ifndef __PS_HINTER_ALGO_H__
#define __PS_HINTER_ALGO_H__

FT_BEGIN_HEADER

/* define to choose hinting algorithm */
#define  PSH_ALGORITHM_2

#ifdef PSH_ALGORITHM_1
#  include  "pshalgo1.h"
#  define  PS_HINTS_APPLY_FUNC   ps1_hints_apply
#else
#  include  "pshalgo2.h"
#  define  PS_HINTS_APPLY_FUNC   ps2_hints_apply
#endif

FT_END_HEADER

#endif /* __PS_HINTER_ALGO_H__ */
