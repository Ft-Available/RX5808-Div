#ifndef __page_about_H
#define __page_about_H

 
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

    /**********************
    * GLOBAL PROTOTYPES
    **********************/
    extern lv_indev_t* indev_keypad;


    void page_about_create(void);
    void page_about_exit(void);
		/**********************
    *      MACROS
    **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif




#endif
