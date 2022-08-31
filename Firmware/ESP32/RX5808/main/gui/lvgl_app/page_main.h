#ifndef __page_main_H
#define __page_main_H



#ifdef __cplusplus
extern "C" {
#endif

    /*********************
    *      INCLUDES
    *********************/
#include "../lvgl/lvgl.h"

    /*********************
    *      DEFINES
    *********************/

    /**********************
    *      TYPEDEFS
    **********************/
   typedef enum
    {
        fre_single = 0,
        fre_ten,
        fre_hunderd,
        fre_thousand,
        fre_label_count,
    }fre_label_enum;
		
		    typedef enum
    {
        fre_pre = 0,
        fre_cur,
        fre_pre_cur_count,
    }fre_pre_cur;
    /**********************
    * GLOBAL PROTOTYPES
    **********************/
void page_main_create(void);

    /**********************
    *      MACROS
    **********************/
extern bool lock_flag;

#ifdef __cplusplus
} /*extern "C"*/
#endif




#endif
