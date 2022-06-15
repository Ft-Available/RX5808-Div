#include "page_scan.h"
#include "page_menu.h"
#include "page_main.h"
#include "page_scan_chart.h"
#include "page_scan_table.h"
#include "page_scan_calib.h"
#include "lvgl_stl.h"
#include "rx5808.h"
#include "beep.h"


#define page_scan_anim_enter  lv_anim_path_bounce
#define page_scan_anim_leave  lv_anim_path_bounce

#define LABEL_FOCUSE_COLOR    lv_color_make(255, 100, 0)
#define LABEL_DEFAULT_COLOR   lv_color_make(255, 255, 255)

static lv_obj_t* menu_scan_contain = NULL;
static lv_obj_t* chart_label;
static lv_obj_t* table_label;
static lv_obj_t* calib_label;
static lv_group_t* scan_group;


static void page_scan_callback(lv_event_t* event);
static void page_scan_exit(void);

static void page_scan_callback(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_target(event);
    if (code == LV_EVENT_KEY)
    {
        beep_on_off(1);
        lv_fun_param_delayed(beep_on_off, 100, 0);
        lv_key_t key_status = lv_indev_get_key(lv_indev_get_act());
        if (key_status == LV_KEY_ENTER) {
            if (obj == chart_label)
            {
                page_scan_exit();
                lv_fun_delayed(page_scan_chart_create, 500);
            }
            else if (obj == table_label)
            {
                page_scan_exit();
                lv_fun_delayed(page_scan_table_create, 500);
            }
            else if (obj == calib_label)
            {
                page_scan_exit();
                lv_fun_delayed(page_scan_calib_create, 500);
            }
        }
        else if (key_status == LV_KEY_LEFT) {
            page_scan_exit();
            lv_fun_param_delayed(page_menu_create, 500, item_scan);
        }
        else if (key_status == LV_KEY_UP) {
            lv_group_focus_prev(scan_group);
        }
        else if (key_status == LV_KEY_DOWN) {
            lv_group_focus_next(scan_group);
        }
    }
}

static void page_scan_exit()
{
    lv_amin_start(chart_label, lv_obj_get_x(chart_label), 160, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_anim_leave);
    lv_amin_start(table_label, lv_obj_get_style_opa(table_label, LV_PART_MAIN), 0, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_scan_anim_leave);
    lv_amin_start(calib_label, lv_obj_get_x(calib_label), -160, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_anim_leave);

    lv_group_del(scan_group);
    lv_obj_del_delayed(menu_scan_contain, 500);
}

void page_scan_create()
{

    menu_scan_contain = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(menu_scan_contain);
    lv_obj_set_style_bg_color(menu_scan_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(menu_scan_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(menu_scan_contain, 160, 80);
    lv_obj_set_pos(menu_scan_contain, 0, 0);

    chart_label = lv_label_create(menu_scan_contain);
    lv_obj_set_style_bg_opa(chart_label, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(chart_label, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(chart_label, LABEL_DEFAULT_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(chart_label, LABEL_FOCUSE_COLOR, LV_STATE_FOCUSED);
    lv_obj_set_style_text_font(chart_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(chart_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_label_set_text_fmt(chart_label, "Show with Chart");
    lv_obj_align(chart_label, LV_ALIGN_TOP_MID, 0, 10);
    lv_label_set_long_mode(chart_label, LV_LABEL_LONG_WRAP);

    table_label = lv_label_create(menu_scan_contain);
    lv_obj_set_style_bg_opa(table_label, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(table_label, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(table_label, LABEL_DEFAULT_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(table_label, LABEL_FOCUSE_COLOR, LV_STATE_FOCUSED);
    lv_obj_set_style_text_font(table_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(table_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_label_set_text_fmt(table_label, "Show with Table");
    lv_obj_align(table_label, LV_ALIGN_TOP_MID, 0, 32);
    lv_label_set_long_mode(table_label, LV_LABEL_LONG_WRAP);


    calib_label = lv_label_create(menu_scan_contain);
    lv_obj_set_style_bg_opa(calib_label, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(calib_label, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(calib_label, LABEL_DEFAULT_COLOR, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(calib_label, LABEL_FOCUSE_COLOR, LV_STATE_FOCUSED);
    lv_obj_set_style_text_font(calib_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(calib_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_label_set_text_fmt(calib_label, "RSSI Calibration");
    lv_obj_align(calib_label, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_label_set_long_mode(calib_label, LV_LABEL_LONG_WRAP);


    scan_group = lv_group_create();
    lv_indev_set_group(indev_keypad, scan_group);


    lv_obj_add_event_cb(chart_label, page_scan_callback, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(table_label, page_scan_callback, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(calib_label, page_scan_callback, LV_EVENT_KEY, NULL);

    lv_group_add_obj(scan_group, chart_label);
    lv_group_add_obj(scan_group, table_label);
    lv_group_add_obj(scan_group, calib_label);
    lv_group_set_editing(scan_group, true);


    lv_amin_start(chart_label, -160, 0, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_anim_enter);
    lv_amin_start(table_label, 0, 255, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_opa_cb, page_scan_anim_enter);
    lv_amin_start(calib_label, 160, 0, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_anim_enter);

}
