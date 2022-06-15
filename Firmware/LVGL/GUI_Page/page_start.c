#include "page_start.h"
#include "page_main.h"
#include "lvgl_stl.h"

LV_FONT_DECLARE(lv_font_start);

lv_obj_t* page_start_contain;
volatile uint8_t start_animation = 1;

static void page_start_exit(void);

void page_start_create()
{
    if (start_animation == true)
    {
        page_start_contain = lv_obj_create(lv_scr_act());                         
        lv_obj_remove_style_all(page_start_contain);                            
        lv_obj_set_style_bg_color(page_start_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);      
        lv_obj_set_style_bg_opa(page_start_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);        
        lv_obj_set_size(page_start_contain, 160, 80);                              
        lv_obj_set_pos(page_start_contain, 0, 0);                                  


        lv_obj_t* start_info_label = lv_label_create(page_start_contain);
        lv_label_set_long_mode(start_info_label, LV_LABEL_LONG_WRAP);

        lv_obj_align(start_info_label, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_text_font(start_info_label, &lv_font_start, LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(start_info_label, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
        lv_label_set_recolor(start_info_label, true);
        lv_label_set_text(start_info_label, "RX5808");

        lv_obj_t* start_info_bar = lv_bar_create(page_start_contain);
        lv_obj_remove_style(start_info_bar, NULL, LV_PART_KNOB);
        lv_obj_set_size(start_info_bar, 100, 5);
        lv_obj_set_style_bg_color(start_info_bar, lv_color_make(255, 140, 0), LV_PART_INDICATOR);
        lv_obj_align(start_info_bar, LV_ALIGN_TOP_MID, 0, 53);
        lv_bar_set_value(start_info_bar, 0, LV_ANIM_OFF);


        lv_amin_start(start_info_label, -50, 20, 1, 800, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, lv_anim_path_linear);
        lv_amin_start(start_info_bar, 0, 100, 1, 500, 800, (lv_anim_exec_xcb_t)lv_obj_opa_cb, lv_anim_path_linear);
        lv_amin_start(start_info_bar, 0, 100, 1, 1500, 1000, (lv_anim_exec_xcb_t)lv_bar_set_value, lv_anim_path_linear);

        lv_fun_delayed(page_start_exit, 3000);
    }
		else
			 page_main_create();

}

static void page_start_exit()
{
    lv_obj_del(page_start_contain);
    page_main_create();
}


void page_set_animation_en(uint8_t en)
{
    if (en)
        start_animation = true;
    else
        start_animation = false;
}

uint8_t page_get_animation_en()
{
   return start_animation;
}


