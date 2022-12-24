#include "page_scan.h"
#include "page_scan_calib.h"
#include "page_menu.h"
#include "page_main.h"
#include "rx5808.h"
#include "rx5808_config.h"
#include "lvgl_stl.h"
#include "beep.h"


LV_FONT_DECLARE(lv_font_chinese_12);

#define page_scan_calib_anim_enter  lv_anim_path_bounce
#define page_scan_calib_anim_leave  lv_anim_path_bounce

#define scan_turn_time  100

static uint8_t time_repeat_count = 0;
static uint16_t rssi_adc_min0 = 4095;
static uint16_t rssi_adc_max0 = 0;
static uint16_t rssi_adc_min1 = 4095;
static uint16_t rssi_adc_max1 = 0;


static lv_obj_t* page_scan_calib_contain;
static lv_group_t* scan_group;
static lv_obj_t* open_vtx_label;
static lv_obj_t* open_vtx_cont;
static lv_obj_t* open_vtx_info;
static lv_obj_t* calib_start_label;
static lv_obj_t* calib_start_cont;
static lv_obj_t* calib_result_label;
static lv_obj_t* calib_result_cont;
static lv_obj_t* calib_result_info;
static lv_obj_t* rssi_min_label;
static lv_obj_t* rssi_max_label;
static lv_obj_t* calib_progress_bar;
static lv_timer_t* scan_calib_timer;


static void page_scan_calib_timer_event(lv_timer_t* tmr);
static void page_scan_calib_event(lv_event_t* event);
static void page_scan_calib_update(void);
static void page_scan_calib_exit(void);

void page_scan_calib_create(void);

static void page_scan_calib_timer_event(lv_timer_t* tmr)
{
    int repeat_count = 47 - tmr->repeat_count;

    time_repeat_count = repeat_count;

    if (tmr == scan_calib_timer)
    {
        if (repeat_count == 47)
        {
            lv_amin_start(calib_result_label, lv_obj_get_y(calib_result_label), 32, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, lv_anim_path_bounce);
            lv_amin_start(calib_result_cont, lv_obj_get_y(calib_result_cont), 32, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, lv_anim_path_bounce);
            lv_amin_start(calib_result_cont, lv_obj_get_height(calib_result_cont), 48, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_height, lv_anim_path_bounce);
            lv_amin_start(calib_start_cont, lv_obj_get_height(calib_start_cont), 16, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_height, lv_anim_path_bounce);
            lv_obj_set_style_opa(rssi_min_label, (lv_opa_t)LV_OPA_0, LV_STATE_DEFAULT);
            lv_obj_set_style_opa(rssi_max_label, (lv_opa_t)LV_OPA_0, LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(calib_result_cont, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
            bool calib_flag=false;
            if (RX5808_Get_Signal_Source() == 0)
            {
                //rssi_adc_min = rssi_adc_min0+ rssi_adc_min1;
                //rssi_adc_max = rssi_adc_max0+ rssi_adc_max1;
                calib_flag=RX5808_Calib_RSSI(rssi_adc_min0, rssi_adc_max0,rssi_adc_min1,rssi_adc_max1);
            }
            else if (RX5808_Get_Signal_Source() == 1)
            {
                //rssi_adc_min = rssi_adc_min1;
                //rssi_adc_max = rssi_adc_max1;
                calib_flag=RX5808_Calib_RSSI(0, 4095,rssi_adc_min1,rssi_adc_max1);
            }
            else
            {
                //rssi_adc_min =rssi_adc_min0;
                //rssi_adc_max =rssi_adc_max0;
                calib_flag=RX5808_Calib_RSSI(rssi_adc_min0, rssi_adc_max0,0,4095);

            }
            if (calib_flag)
            {
                if (RX5808_Get_Language() == 0)
                {
                    lv_label_set_text_fmt(calib_result_info, "Calib success!\nResult has\nbeen saved!");
                    lv_label_set_text_fmt(calib_result_label, "Success");
                }
                else
                {
                    lv_label_set_text_fmt(calib_result_info, "校准成功!\n结果已保存!");
                    lv_label_set_text_fmt(calib_result_label, " 校准成功 ");
                }
                lv_obj_set_style_text_color(calib_result_info, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(calib_result_label, lv_color_make(0, 255, 0), LV_STATE_DEFAULT);
                RX5808_Set_RSSI_Ad_Min0(rssi_adc_min0);
                RX5808_Set_RSSI_Ad_Max0(rssi_adc_max0);
                RX5808_Set_RSSI_Ad_Min1(rssi_adc_min1);
                RX5808_Set_RSSI_Ad_Max1(rssi_adc_max1);
                rx5808_div_setup_upload(rx5808_div_config_rssi_adc_value_min0);
                rx5808_div_setup_upload(rx5808_div_config_rssi_adc_value_max0);
                rx5808_div_setup_upload(rx5808_div_config_rssi_adc_value_min1);
                rx5808_div_setup_upload(rx5808_div_config_rssi_adc_value_max1);
            }
            else
            {
                if (RX5808_Get_Language() == 0)
                {
                    lv_label_set_text_fmt(calib_result_info, "Calib failed!\nTry again!");
                    lv_label_set_text_fmt(calib_result_label, "Failed");
                }
                else
                {
                    lv_label_set_text_fmt(calib_result_info, "校准失败，请\n退出并重试!");
                    lv_label_set_text_fmt(calib_result_label, " 校准失败 ");
                }
                lv_obj_set_style_text_color(calib_result_info, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(calib_result_label, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
            }
        }
        page_scan_calib_update();
        if (time_repeat_count < 47)
            RX5808_Set_Freq(Rx5808_Freq[(time_repeat_count + 1) / 8][(time_repeat_count + 1) % 8]);
        else if (time_repeat_count == 47)
            RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
    }

}

static void page_scan_calib_event(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_target(event);
    if (code == LV_EVENT_KEY)
    {
        //beep_on_off(1);
        //lv_fun_param_delayed(beep_on_off, 100, 0);
        beep_turn_on();
        lv_key_t key_status = lv_indev_get_key(lv_indev_get_act());
        if (key_status == LV_KEY_ENTER) {
            if (obj == open_vtx_label)
            {
                lv_label_set_text_fmt(open_vtx_info, "......");
                lv_obj_set_style_text_color(open_vtx_info, lv_color_make(100, 100, 100), LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(open_vtx_label, lv_color_make(0, 255, 0), LV_STATE_DEFAULT);
                lv_obj_set_style_bg_color(calib_start_label, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
                lv_obj_set_style_opa(rssi_min_label, (lv_opa_t)LV_OPA_100, LV_STATE_DEFAULT);
                lv_obj_set_style_opa(rssi_max_label, (lv_opa_t)LV_OPA_100, LV_STATE_DEFAULT);
                lv_amin_start(calib_start_label, lv_obj_get_y(calib_start_label), 16, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, lv_anim_path_bounce);
                lv_amin_start(calib_start_cont, lv_obj_get_y(calib_start_cont), 16, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, lv_anim_path_bounce);
                lv_amin_start(open_vtx_cont, lv_obj_get_height(open_vtx_cont), 16, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_height, lv_anim_path_bounce);
                lv_amin_start(calib_start_cont, lv_obj_get_height(calib_start_cont), 48, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_height, lv_anim_path_bounce);
                lv_obj_set_style_border_color(calib_start_cont, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
                lv_group_focus_next(scan_group);
            }
            else if (obj == calib_start_label)
            {
                lv_obj_set_style_bg_color(calib_start_label, lv_color_make(0, 255, 0), LV_STATE_DEFAULT);
                lv_group_focus_next(scan_group);
                RX5808_Set_Freq(Rx5808_Freq[0][0]);
                scan_calib_timer = lv_timer_create(page_scan_calib_timer_event, scan_turn_time, NULL);
                lv_timer_set_repeat_count(scan_calib_timer, 48);
                lv_bar_set_value(calib_progress_bar, 100, LV_ANIM_ON);

            }
            else if (obj == calib_result_label)
            {

            }
        }
        else if (key_status == LV_KEY_LEFT) {
            page_scan_calib_exit();
        }
    }
}


static void page_scan_calib_update()
{
    if (adc_converted_value[0] > rssi_adc_max0)
        rssi_adc_max0 = adc_converted_value[0];
    if (adc_converted_value[0] < rssi_adc_min0)
        rssi_adc_min0 = adc_converted_value[0];
    if (adc_converted_value[1] > rssi_adc_max1)
        rssi_adc_max1 = adc_converted_value[1];
    if (adc_converted_value[1] < rssi_adc_min1)
        rssi_adc_min1 = adc_converted_value[1];
    lv_label_set_text_fmt(rssi_min_label, "%d\n%d", rssi_adc_min0, rssi_adc_min1);
    lv_label_set_text_fmt(rssi_max_label, "%d\n%d", rssi_adc_max0, rssi_adc_max1);
}

static void page_scan_calib_exit()
{
    lv_amin_start(open_vtx_label, lv_obj_get_x(open_vtx_label), -65, 1, 300, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_leave);
    lv_amin_start(calib_start_label, lv_obj_get_x(calib_start_label), -65, 1, 300, 100, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_leave);
    lv_amin_start(calib_result_label, lv_obj_get_x(calib_result_label), -65, 1, 300, 200, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_leave);
    lv_amin_start(open_vtx_cont, lv_obj_get_x(open_vtx_cont), 160, 1, 300, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_leave);
    lv_amin_start(calib_start_cont, lv_obj_get_x(calib_start_cont), 160, 1, 300, 100, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_leave);
    lv_amin_start(calib_result_cont, lv_obj_get_x(calib_result_cont), 160, 1, 300, 200, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_leave);

    if (lv_group_get_focused(scan_group) == calib_result_label && time_repeat_count < 47)
    {
        RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
        lv_timer_del(scan_calib_timer);
    }
    lv_group_del(scan_group);
    lv_obj_del_delayed(page_scan_calib_contain, 500);
    lv_fun_delayed(page_scan_create, 500);
}


void page_scan_calib_create()
{
    rssi_adc_min0 = 4095;
    rssi_adc_max0 = 0;
    rssi_adc_min1 = 4095;
    rssi_adc_max1 = 0;
    time_repeat_count = 0;
    page_scan_calib_contain = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(page_scan_calib_contain);
    lv_obj_set_style_bg_color(page_scan_calib_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(page_scan_calib_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(page_scan_calib_contain, 160, 80);
    lv_obj_set_pos(page_scan_calib_contain, 0, 0);

    open_vtx_label = lv_label_create(page_scan_calib_contain);
    lv_obj_set_style_bg_opa(open_vtx_label, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(open_vtx_label, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(open_vtx_label, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(open_vtx_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(open_vtx_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    //lv_label_set_text_fmt(open_vtx_label, "Open VTX");
    lv_obj_set_style_text_align(open_vtx_label, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    lv_obj_set_pos(open_vtx_label, 0, 0);
    lv_obj_set_size(open_vtx_label, 65, 16);
    lv_label_set_long_mode(open_vtx_label, LV_LABEL_LONG_WRAP);


    calib_start_label = lv_label_create(page_scan_calib_contain);
    lv_obj_set_style_bg_opa(calib_start_label, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(calib_start_label, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(calib_start_label, lv_color_make(100, 100, 100), LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(calib_start_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(calib_start_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(calib_start_label, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    //lv_label_set_text_fmt(calib_start_label, "Start Calib");
    lv_obj_set_pos(calib_start_label, 0, 48);
    lv_obj_set_size(calib_start_label, 65, 16);
    lv_label_set_long_mode(calib_start_label, LV_LABEL_LONG_WRAP);

    calib_result_label = lv_label_create(page_scan_calib_contain);
    lv_obj_set_style_bg_opa(calib_result_label, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(calib_result_label, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(calib_result_label, lv_color_make(100, 100, 100), LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(calib_result_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(calib_result_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(calib_result_label, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);
    //lv_label_set_text_fmt(calib_result_label, "......");
    lv_obj_set_pos(calib_result_label, 0, 64);
    lv_obj_set_size(calib_result_label, 65, 16);
    lv_label_set_long_mode(calib_result_label, LV_LABEL_LONG_WRAP);

    open_vtx_cont = lv_obj_create(page_scan_calib_contain);
    lv_obj_remove_style_all(open_vtx_cont);
    lv_obj_set_style_bg_color(open_vtx_cont, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(open_vtx_cont, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(open_vtx_cont, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(open_vtx_cont, 1, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(open_vtx_cont, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(open_vtx_cont, 5, LV_STATE_DEFAULT);
    lv_obj_set_size(open_vtx_cont, 90, 48);
    lv_obj_set_pos(open_vtx_cont, 70, 0);


    open_vtx_info = lv_label_create(open_vtx_cont);
    lv_obj_set_style_bg_opa(open_vtx_info, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(open_vtx_info, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(open_vtx_info, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(open_vtx_info, &lv_font_montserrat_8, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(open_vtx_info, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
    //lv_label_set_text_fmt(open_vtx_info, "Install the antenna\nOpen your VTX\nSet max power");
    lv_label_set_long_mode(open_vtx_info, LV_LABEL_LONG_WRAP);
    //lv_obj_set_size(open_vtx_info, 75, 14);
    lv_obj_center(open_vtx_info);

    calib_start_cont = lv_obj_create(page_scan_calib_contain);
    lv_obj_remove_style_all(calib_start_cont);
    lv_obj_set_style_bg_color(calib_start_cont, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(calib_start_cont, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(calib_start_cont, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(calib_start_cont, 1, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(calib_start_cont, lv_color_make(100, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(calib_start_cont, 5, LV_STATE_DEFAULT);
    lv_obj_set_size(calib_start_cont, 90, 16);
    lv_obj_set_pos(calib_start_cont, 70, 48);

    calib_result_cont = lv_obj_create(page_scan_calib_contain);
    lv_obj_remove_style_all(calib_result_cont);
    lv_obj_set_style_bg_color(calib_result_cont, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(calib_result_cont, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(calib_result_cont, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(calib_result_cont, 1, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(calib_result_cont, lv_color_make(100, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_radius(calib_result_cont, 5, LV_STATE_DEFAULT);
    lv_obj_set_size(calib_result_cont, 90, 16);
    lv_obj_set_pos(calib_result_cont, 70, 64);

    calib_result_info = lv_label_create(calib_result_cont);
    lv_obj_set_style_bg_opa(calib_result_info, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(calib_result_info, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(calib_result_info, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(calib_result_info, &lv_font_montserrat_8, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(calib_result_info, lv_color_make(100, 100, 100), LV_STATE_DEFAULT);
    lv_label_set_long_mode(calib_result_info, LV_LABEL_LONG_WRAP);
    //lv_label_set_text_fmt(calib_result_info, "......");
    //lv_obj_set_size(calib_result_info, 75, 14);
    lv_obj_center(calib_result_info);

    rssi_min_label = lv_label_create(calib_start_cont);
    lv_obj_set_style_opa(rssi_min_label, (lv_opa_t)LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(rssi_min_label, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(rssi_min_label, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(rssi_min_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(rssi_min_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(rssi_min_label, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
    lv_label_set_text_fmt(rssi_min_label, "%d\n%d", 1000, 1000);
    lv_obj_set_pos(rssi_min_label, 10, 2);
    lv_label_set_long_mode(rssi_min_label, LV_LABEL_LONG_WRAP);

    rssi_max_label = lv_label_create(calib_start_cont);
    lv_obj_set_style_opa(rssi_max_label, (lv_opa_t)LV_OPA_0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(rssi_max_label, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(rssi_max_label, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(rssi_max_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(rssi_max_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(rssi_max_label, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
    lv_label_set_text_fmt(rssi_max_label, "%d\n%d", 1000, 1000);
    lv_obj_set_pos(rssi_max_label, 50, 2);
    lv_label_set_long_mode(rssi_max_label, LV_LABEL_LONG_WRAP);

    calib_progress_bar = lv_bar_create(calib_start_cont);
    lv_obj_remove_style(calib_progress_bar, NULL, LV_PART_KNOB);
    lv_obj_set_size(calib_progress_bar, 70, 6);
    lv_obj_set_style_bg_color(calib_progress_bar, lv_color_make(255, 180, 0), LV_PART_INDICATOR);
    lv_obj_align(calib_progress_bar, LV_ALIGN_TOP_MID, 0, 32);
    lv_bar_set_value(calib_progress_bar, 0, LV_ANIM_ON);
    lv_obj_set_style_anim_time(calib_progress_bar, scan_turn_time * 48, LV_STATE_DEFAULT);


    if (RX5808_Get_Language() == 0)
    {
        lv_obj_set_style_text_font(open_vtx_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(open_vtx_label, "Open VTX");
        lv_obj_set_style_text_font(open_vtx_info, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(open_vtx_info, "Open VTX\nInstall Aerial");
        lv_obj_set_style_text_font(calib_start_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(calib_start_label, "Start Calib");
        lv_obj_set_style_text_font(calib_result_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(calib_result_label, "......");
        lv_obj_set_style_text_font(calib_result_info, &lv_font_montserrat_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(calib_result_info, "......");
    }
    else
    {
        lv_obj_set_style_text_font(open_vtx_label, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(open_vtx_label, " 打开图传 ");
        lv_obj_set_style_text_font(open_vtx_info, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(open_vtx_info, "安装接收机天\n线并打开图传 ");
        lv_obj_set_style_text_font(calib_start_label, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(calib_start_label, " 开始校准 ");
        lv_obj_set_style_text_font(calib_result_label, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(calib_result_label, "......");
        lv_obj_set_style_text_font(calib_result_info, &lv_font_chinese_12, LV_STATE_DEFAULT);
        lv_label_set_text_fmt(calib_result_info, "......");
    }

    scan_group = lv_group_create();
    lv_indev_set_group(indev_keypad, scan_group);

    lv_obj_add_event_cb(open_vtx_label, page_scan_calib_event, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(calib_start_label, page_scan_calib_event, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(calib_result_label, page_scan_calib_event, LV_EVENT_KEY, NULL);

    lv_group_add_obj(scan_group, open_vtx_label);
    lv_group_add_obj(scan_group, calib_start_label);
    lv_group_add_obj(scan_group, calib_result_label);
    lv_group_set_editing(scan_group, true);

    lv_amin_start(open_vtx_label, -65, 0, 1, 300, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_enter);
    lv_amin_start(calib_start_label, -65, 0, 1, 300, 100, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_enter);
    lv_amin_start(calib_result_label, -65, 0, 1, 300, 200, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_enter);
    lv_amin_start(open_vtx_cont, 160, 70, 1, 300, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_enter);
    lv_amin_start(calib_start_cont, 160, 70, 1, 300, 100, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_enter);
    lv_amin_start(calib_result_cont, 160, 70, 1, 300, 200, (lv_anim_exec_xcb_t)lv_obj_set_x, page_scan_calib_anim_enter);

}

