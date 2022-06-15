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
    extern lv_indev_t* indev_keypad;
    extern const uint16_t Freq_Buff[6][8];
    void page_main_create();
    void page_main_exit();
    void page_main_group_create();
    void fre_label_update(uint8_t a, uint8_t b);
    /**********************
    *      MACROS
    **********************/


#ifdef __cplusplus
} /*extern "C"*/
#endif




#endif
