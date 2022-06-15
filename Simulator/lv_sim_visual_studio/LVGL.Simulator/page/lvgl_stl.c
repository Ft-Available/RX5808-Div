#include "lvgl_stl.h"


void lv_amin_start(void* obj, \
    int32_t start_value, \
    int32_t end_value, \
    uint32_t repeat_count, \
    uint32_t duration, \
    uint32_t delay, \
    lv_anim_exec_xcb_t exec_cb, \
    lv_anim_path_cb_t path_cb)
{
    lv_anim_t  anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_set_repeat_count(&anim, repeat_count);
    lv_anim_set_exec_cb(&anim, exec_cb);
    lv_anim_set_time(&anim, duration);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_path_cb(&anim, path_cb);
    lv_anim_start(&anim);
}

void lv_obj_create_delayed(lv_obj_t* obj, uint32_t delay_ms)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, NULL);
    lv_anim_set_time(&a, 1);
    lv_anim_set_delay(&a, delay_ms);
    lv_anim_set_ready_cb(&a, lv_obj_create_anim_ready_cb);
    lv_anim_start(&a);
}



void lv_fun_param_delayed(void (*fun)(uint8_t), uint32_t delay_ms,uint8_t user_data)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, fun);
    lv_anim_set_exec_cb(&a, NULL);
    lv_anim_set_time(&a, 1);
    lv_anim_set_delay(&a, delay_ms);
    lv_anim_set_ready_cb(&a, lv_fun_param_ready_cb);
    lv_anim_set_user_data(&a, (void*)user_data);
    lv_anim_start(&a);
    
}

void lv_fun_delayed(void (*fun)(), uint32_t delay_ms)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, fun);
    lv_anim_set_exec_cb(&a, NULL);
    lv_anim_set_time(&a, 1);
    lv_anim_set_delay(&a, delay_ms);
    lv_anim_set_ready_cb(&a, lv_fun_ready_cb);
    lv_anim_start(&a);

}


void lv_style_init_simple(lv_style_t *style)
{
    lv_style_init(style);
    lv_style_set_bg_color(style, lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_bg_opa(style, LV_OPA_COVER);

    lv_style_set_text_color(style, lv_color_white());
    lv_style_set_text_opa(style, LV_OPA_COVER);

    lv_style_set_border_width(style, 2);
    lv_style_set_border_color(style, lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_border_opa(style, LV_OPA_COVER);

    //lv_style_set_width(style,width);
    //lv_style_set_height(style, height);

    //lv_style_set_x(style, x);
    //lv_style_set_y(style, y);

   /*  lv_style_set_width(style, lv_coord_t value);
     lv_style_set_min_width(style, lv_coord_t value);
     lv_style_set_max_width(style, lv_coord_t value);
     lv_style_set_height(style, lv_coord_t value);
     lv_style_set_min_height(style, lv_coord_t value);
     lv_style_set_max_height(style, lv_coord_t value);
     lv_style_set_x(style, lv_coord_t value);
     lv_style_set_y(style, lv_coord_t value);
     lv_style_set_align(style, lv_align_t value);
     lv_style_set_transform_width(style, lv_coord_t value);
     lv_style_set_transform_height(style, lv_coord_t value);
     lv_style_set_translate_x(style, lv_coord_t value);
     lv_style_set_translate_y(style, lv_coord_t value);
     lv_style_set_transform_zoom(style, lv_coord_t value);
     lv_style_set_transform_angle(style, lv_coord_t value);
     lv_style_set_pad_top(style, lv_coord_t value);
     lv_style_set_pad_bottom(style, lv_coord_t value);
     lv_style_set_pad_left(style, lv_coord_t value);
     lv_style_set_pad_right(style, lv_coord_t value);
     lv_style_set_pad_row(style, lv_coord_t value);
     lv_style_set_pad_column(style, lv_coord_t value);
     lv_style_set_bg_color(style, lv_color_t value);
     lv_style_set_bg_color_filtered(style, lv_color_t value);
     lv_style_set_bg_opa(style, lv_opa_t value);
     lv_style_set_bg_grad_color(style, lv_color_t value);
     lv_style_set_bg_grad_color_filtered(style, lv_color_t value);
     lv_style_set_bg_grad_dir(style, lv_grad_dir_t value);
     lv_style_set_bg_main_stop(style, lv_coord_t value);
     lv_style_set_bg_grad_stop(style, lv_coord_t value);
     lv_style_set_bg_grad(style, const lv_grad_dsc_t * value);
     lv_style_set_bg_dither_mode(style, lv_dither_mode_t value);
     lv_style_set_bg_img_src(style, const * value);
     lv_style_set_bg_img_opa(style, lv_opa_t value);
     lv_style_set_bg_img_recolor(style, lv_color_t value);
     lv_style_set_bg_img_recolor_filtered(style, lv_color_t value);
     lv_style_set_bg_img_recolor_opa(style, lv_opa_t value);
     lv_style_set_bg_img_tiled(style, bool value);
     lv_style_set_border_color(style, lv_color_t value);
     lv_style_set_border_color_filtered(style, lv_color_t value);
     lv_style_set_border_opa(style, lv_opa_t value);
     lv_style_set_border_width(style, lv_coord_t value);
     lv_style_set_border_side(style, lv_border_side_t value);
     lv_style_set_border_post(style, bool value);
     lv_style_set_outline_width(style, lv_coord_t value);
     lv_style_set_outline_color(style, lv_color_t value);
     lv_style_set_outline_color_filtered(style, lv_color_t value);
     lv_style_set_outline_opa(style, lv_opa_t value);
     lv_style_set_outline_pad(style, lv_coord_t value);
     lv_style_set_shadow_width(style, lv_coord_t value);
     lv_style_set_shadow_ofs_x(style, lv_coord_t value);
     lv_style_set_shadow_ofs_y(style, lv_coord_t value);
     lv_style_set_shadow_spread(style, lv_coord_t value);
     lv_style_set_shadow_color(style, lv_color_t value);
     lv_style_set_shadow_color_filtered(style, lv_color_t value);
     lv_style_set_shadow_opa(style, lv_opa_t value);
     lv_style_set_img_opa(style, lv_opa_t value);
     lv_style_set_img_recolor(style, lv_color_t value);
     lv_style_set_img_recolor_filtered(style, lv_color_t value);
     lv_style_set_img_recolor_opa(style, lv_opa_t value);
     lv_style_set_line_width(style, lv_coord_t value);
     lv_style_set_line_dash_width(style, lv_coord_t value);
     lv_style_set_line_dash_gap(style, lv_coord_t value);
     lv_style_set_line_rounded(style, bool value);
     lv_style_set_line_color(style, lv_color_t value);
     lv_style_set_line_color_filtered(style, lv_color_t value);
     lv_style_set_line_opa(style, lv_opa_t value);
     lv_style_set_arc_width(style, lv_coord_t value);
     lv_style_set_arc_rounded(style, bool value);
     lv_style_set_arc_color(style, lv_color_t value);
     lv_style_set_arc_color_filtered(style, lv_color_t value);
     lv_style_set_arc_opa(style, lv_opa_t value);
     lv_style_set_arc_img_src(style, const * value);
     lv_style_set_text_color(style, lv_color_t value);
     lv_style_set_text_color_filtered(style, lv_color_t value);
     lv_style_set_text_opa(style, lv_opa_t value);
     lv_style_set_text_font(style, const lv_font_t * value);
     lv_style_set_text_letter_space(style, lv_coord_t value);
     lv_style_set_text_line_space(style, lv_coord_t value);
     lv_style_set_text_decor(style, lv_text_decor_t value);
     lv_style_set_text_align(style, lv_text_align_t value);
     lv_style_set_radius(style, lv_coord_t value);
     lv_style_set_clip_corner(style, bool value);
     lv_style_set_opa(style, lv_opa_t value);
     lv_style_set_color_filter_dsc(style, const lv_color_filter_dsc_t * value);
     lv_style_set_color_filter_opa(style, lv_opa_t value);
     lv_style_set_anim_time(style, uint32_t value);
     lv_style_set_anim_speed(style, uint32_t value);
     lv_style_set_transition(style, const lv_style_transition_dsc_t * value);
     lv_style_set_blend_mode(style, lv_blend_mode_t value);
     lv_style_set_layout(style, uint16_t value);
     lv_style_set_base_dir(style, lv_base_dir_t value);*/

}


/*********************************************************************************************************************************************/

void lv_fun_param_ready_cb(lv_anim_t* a)
{
    (*(FUN)a->var)((uint8_t)a->user_data);
}


void lv_fun_ready_cb(lv_anim_t* a)
{
    ((void (*)())a->var)();
}

void lv_obj_create_anim_ready_cb(lv_anim_t* a)
{
    lv_obj_create(a->var);
}


void lv_obj_opa_cb(lv_obj_t* obj, uint8_t value)
{
    lv_obj_set_style_text_opa(obj, value, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, value, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(obj, value, LV_STATE_DEFAULT);
}

/*********************************************************************************************************************************************/
