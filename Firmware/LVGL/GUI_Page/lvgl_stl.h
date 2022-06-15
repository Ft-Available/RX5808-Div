#ifndef __lvgl_stl_H
#define __lvgl_stl_H


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
    void lv_amin_start(void* obj, \
        int32_t start_value, \
        int32_t end_value, \
        uint32_t repeat_count, \
        uint32_t duration, \
        uint32_t delay, \
        lv_anim_exec_xcb_t exec_cb, \
        lv_anim_path_cb_t path_cb);
    void lv_obj_create_delayed(lv_obj_t* obj, uint32_t SysTick_Delay_ms);

    void lv_fun_param_delayed(void (*fun)(uint8_t), uint32_t SysTick_Delay_ms, uint8_t user_data);
    void lv_fun_delayed(void (*fun)(), uint32_t SysTick_Delay_ms);
    void lv_style_init_simple(lv_style_t* style);

    void lv_obj_opa_cb(lv_obj_t* obj, uint8_t value);
    void lv_fun_ready_cb(lv_anim_t* a);
    void lv_fun_param_ready_cb(lv_anim_t* a);
    void lv_obj_create_anim_ready_cb(lv_anim_t* a);
    /**********************
    *      MACROS
    **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif




#endif
