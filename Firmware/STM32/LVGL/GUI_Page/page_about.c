#include "page_about.h"
#include "page_menu.h"
#include "rx5808.h"
#include "lvgl_stl.h"


LV_FONT_DECLARE(lv_font_chinese_16);
LV_FONT_DECLARE(lv_font_chinese_12);

#define page_about_anim_enter  lv_anim_path_bounce
#define page_about_anim_leave  lv_anim_path_bounce

#define label_about_Abscissa0 15
#define label_about_Abscissa1 85
#define label_about_space     15

static lv_obj_t* page_about_contain = NULL;
static lv_obj_t* vbat_label = NULL;
static lv_obj_t* version_label = NULL;
static lv_obj_t* base_label = NULL;
static lv_obj_t* protocol_label = NULL;
static lv_group_t* about_group = NULL;

static lv_timer_t* vbat_label_timer = NULL;

static lv_style_t label_about_style;
extern uint16_t adc_converted_value[3];

static void page_about_style_init(void);
static void page_about_style_deinit(void);
static void vbat_label_update(lv_timer_t* tmr);
static void page_about_exit(void);

static void event_callback(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_KEY)
    {
        beep_on_off(1);
        lv_fun_param_delayed(beep_on_off, 100, 0);
        lv_key_t key_status = lv_indev_get_key(lv_indev_get_act());
        if (key_status == LV_KEY_ENTER) {
           
        }
        else if (key_status == LV_KEY_LEFT) {
            page_about_exit();

        }
        else if (key_status == LV_KEY_RIGHT) {

        }
        else if (key_status == LV_KEY_NEXT) {

        }

    }
}

static void page_about_style_init()
{
    lv_style_init_simple(&label_about_style);
}

static void page_about_style_deinit()
{
    lv_style_reset(&label_about_style);
}

static void vbat_label_update(lv_timer_t* tmr)
{
    if (RX5808_Get_Language() == 0)
    {
        lv_label_set_text_fmt(vbat_label, "VCC_BAT:%.3fV", Get_Battery_Voltage());
    }
    else
    {
        lv_label_set_text_fmt(vbat_label, "供电电压:%.3fV", Get_Battery_Voltage());
    }
}


static void page_about_exit()
{
    lv_amin_start(vbat_label, lv_obj_get_style_text_opa(vbat_label, LV_PART_MAIN), 0, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_about_anim_leave);
    lv_amin_start(version_label, lv_obj_get_style_text_opa(version_label, LV_PART_MAIN), 0, 1, 200, 200, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_about_anim_leave);
    lv_amin_start(base_label, lv_obj_get_style_text_opa(base_label, LV_PART_MAIN), 0, 1, 200, 100, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_about_anim_leave);
    lv_amin_start(protocol_label, lv_obj_get_style_text_opa(protocol_label, LV_PART_MAIN), 0, 1, 200, 0, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_about_anim_leave);

    lv_obj_del_delayed(page_about_contain, 500);
    lv_group_del(about_group);
    lv_timer_del(vbat_label_timer);
    lv_fun_delayed(page_about_style_deinit, 500);
    lv_fun_param_delayed(page_menu_create, 500, item_about);
}

void page_about_create()
{
    page_about_contain = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(page_about_contain);
    lv_obj_set_style_bg_color(page_about_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(page_about_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(page_about_contain, 160, 80);
    lv_obj_set_pos(page_about_contain, 0, 0);

    lv_obj_t* menu_about_label = lv_label_create(page_about_contain);
    lv_label_set_long_mode(menu_about_label, LV_LABEL_LONG_WRAP);

    lv_obj_align(menu_about_label, LV_ALIGN_TOP_MID, 0, 0);
    //lv_obj_set_style_text_font(menu_about_label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(menu_about_label, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
    lv_label_set_recolor(menu_about_label, true);
    //lv_label_set_text(menu_about_label, "#1010ff RX5808# #00ff00 -# #ff0000 Div#");

    page_about_style_init();

    vbat_label = lv_label_create(page_about_contain);
    lv_obj_add_style(vbat_label, &label_about_style, LV_STATE_DEFAULT);
    lv_obj_align(vbat_label, LV_ALIGN_TOP_MID, 0, label_about_space * 1);
    //lv_obj_set_pos(vbat_label, label_about_Abscissa0, label_about_space * 1);
    lv_label_set_long_mode(vbat_label, LV_LABEL_LONG_WRAP);
    //lv_obj_set_style_text_font(vbat_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(vbat_label, lv_color_make(128, 255, 128), LV_STATE_DEFAULT);

    version_label = lv_label_create(page_about_contain);
    lv_obj_add_style(version_label, &label_about_style, LV_STATE_DEFAULT);
    lv_obj_align(version_label, LV_ALIGN_TOP_MID, 0, label_about_space * 2);
    //lv_obj_set_pos(version_label, label_about_Abscissa0, label_about_space * 2);
    lv_label_set_long_mode(version_label, LV_LABEL_LONG_WRAP);
    //lv_obj_set_style_text_font(version_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(version_label, lv_color_make(255, 255, 0), LV_STATE_DEFAULT);

    base_label = lv_label_create(page_about_contain);
    lv_obj_add_style(base_label, &label_about_style, LV_STATE_DEFAULT);
    lv_obj_align(base_label, LV_ALIGN_TOP_MID, 0, label_about_space * 3);
    //lv_obj_set_pos(base_label, label_about_Abscissa0, label_about_space * 3);
    lv_label_set_long_mode(base_label, LV_LABEL_LONG_WRAP);
    //lv_obj_set_style_text_font(base_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(base_label, lv_color_make(128, 128, 255), LV_STATE_DEFAULT);

    protocol_label = lv_label_create(page_about_contain);
    lv_obj_add_style(protocol_label, &label_about_style, LV_STATE_DEFAULT);
    lv_obj_align(protocol_label, LV_ALIGN_TOP_MID, 0, label_about_space * 4);
    //lv_obj_set_pos(protocol_label, label_about_Abscissa0, label_about_space * 4);
    lv_label_set_long_mode(protocol_label, LV_LABEL_LONG_WRAP);
    //lv_obj_set_style_text_font(protocol_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(protocol_label, lv_color_make(255, 74, 128), LV_STATE_DEFAULT);

   


    if (RX5808_Get_Language() == 0)
    {
        lv_obj_set_style_text_font(menu_about_label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
        lv_label_set_text(menu_about_label, "#1010ff RX5808# #00ff00 -# #ff0000 Div#");
        lv_obj_set_style_text_font(vbat_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(version_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(base_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(protocol_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(vbat_label, "VCC_BAT:%.3fV", Get_Battery_Voltage());
        lv_label_set_text_fmt(version_label, "VERSION:1.1.0");
        lv_label_set_text_fmt(base_label, "LVGL:v%d.%d.%d", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
        lv_label_set_text_fmt(protocol_label, "LICENSE:GPL3.0");
    }
    else
    {
        lv_obj_set_style_text_font(menu_about_label, &lv_font_chinese_16, LV_STATE_DEFAULT);
        lv_label_set_text(menu_about_label, "#1010ff 模拟# #00ff00 -# #ff0000 接收机#");
        lv_obj_set_style_text_font(vbat_label, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(version_label, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(base_label, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(protocol_label, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(vbat_label, "供电电压:%.3fV", Get_Battery_Voltage());
        lv_label_set_text_fmt(version_label, "固件版本:1.1.0");
        lv_label_set_text_fmt(base_label, "界面版本:v%d.%d.%d", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
        lv_label_set_text_fmt(protocol_label, "开源协议:GPL3.0");
    }

    about_group = lv_group_create();
    lv_indev_set_group(indev_keypad, about_group);

    lv_obj_add_event_cb(menu_about_label, event_callback, LV_EVENT_KEY, NULL);

    lv_group_add_obj(about_group, menu_about_label);
    lv_group_set_editing(about_group, false);

    vbat_label_timer = lv_timer_create(vbat_label_update, 300, NULL);

    lv_amin_start(vbat_label, 0, 255, 1, 200, 0, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_about_anim_enter);
    lv_amin_start(version_label, 0, 255, 1, 200, 100, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_about_anim_enter);
    lv_amin_start(base_label, 0, 255, 1, 200, 200, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_about_anim_enter);
    lv_amin_start(protocol_label, 0, 255, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_about_anim_enter);
}



