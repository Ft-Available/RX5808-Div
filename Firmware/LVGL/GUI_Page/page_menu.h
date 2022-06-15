#ifndef __page_menu_H
#define __page_menu_H



#ifdef __cplusplus
extern "C" {
#endif

    /*********************
    *      INCLUDES
    *********************/
#include "../lvgl/lvgl.h"
#include "page_main.h"

    /*********************
    *      DEFINES
    *********************/

    /**********************
    *      TYPEDEFS
    **********************/
    typedef enum
    {
			
        item_scan= 0,
			  item_setup ,
        item_about,

        item_count
    }menu_item_list;

    typedef struct
    {
        lv_obj_t* item_contain;
        lv_obj_t* item_imag;
        lv_obj_t* item_title;
        lv_obj_t* item_label0;
        lv_obj_t* item_label1;
        
    }menu_item;

    /**********************
    * GLOBAL PROTOTYPES
    **********************/
    extern lv_indev_t* indev_keypad;

    void page_menu_create(uint8_t);
    /**********************
    *      MACROS
    **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif




#endif
