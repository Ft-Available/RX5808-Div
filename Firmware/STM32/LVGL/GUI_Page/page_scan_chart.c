#include "page_scan.h"
#include "page_scan_chart.h"
#include "page_menu.h"
#include "page_main.h"
#include "rx5808.h"
#include "lvgl_stl.h"

#define page_scan_chart_anim_enter  lv_anim_path_bounce
#define page_scan_chart_anim_leave  lv_anim_path_bounce

#define scan_turn_time  100

static lv_obj_t* page_scan_chart_contain = NULL;
static lv_obj_t* chart_fre_label;
static lv_group_t* scan_group;
static uint8_t time_repeat_count = 0;
static lv_obj_t* rssi_quality_chart;
static lv_chart_series_t* rssi0_curve;
static lv_chart_series_t* rssi1_curve;
static lv_timer_t* scan_chart_timer;

static void page_scan_chart_timer_event(lv_timer_t* tmr);
static void page_scan_event_callback(lv_event_t* event);
static void page_scan_chart_exit(void);

static void page_scan_chart_timer_event(lv_timer_t* tmr)
{
    int repeat_count = 47 - tmr->repeat_count;
    time_repeat_count = repeat_count;
    if (tmr == scan_chart_timer)
    {
			if(RX5808_Get_Signal_Source()==1)
		{						
        lv_chart_set_value_by_id(rssi_quality_chart, rssi1_curve, repeat_count, Rx5808_Get_Precentage1());
		}
		else if(RX5808_Get_Signal_Source()==2)
		{
        lv_chart_set_value_by_id(rssi_quality_chart, rssi0_curve, repeat_count, Rx5808_Get_Precentage0());
		}
		else
		{
		    lv_chart_set_value_by_id(rssi_quality_chart, rssi0_curve, repeat_count, Rx5808_Get_Precentage0());
        lv_chart_set_value_by_id(rssi_quality_chart, rssi1_curve, repeat_count, Rx5808_Get_Precentage1());
		}
        
        if (time_repeat_count < 47)
            RX5808_Set_Freq(5300 + (time_repeat_count + 1) * 12.5);
        else if (time_repeat_count == 47)
            RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
    }

}

static void page_scan_event_callback(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_target(event);
    if (code == LV_EVENT_KEY)
    {
        beep_on_off(1);
        lv_fun_param_delayed(beep_on_off, 100, 0);
        lv_key_t key_status = lv_indev_get_key(lv_indev_get_act());
        if (key_status == LV_KEY_LEFT) {
            page_scan_chart_exit();
        }
    }
}

static void page_scan_chart_exit()
{
    lv_amin_start(rssi_quality_chart, lv_obj_get_y(rssi_quality_chart), -60, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_chart_anim_leave);
    lv_amin_start(chart_fre_label, lv_obj_get_y(chart_fre_label), 80, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_chart_anim_leave);
    if (time_repeat_count < 47)
    {
        lv_timer_del(scan_chart_timer);
        RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
    }

    lv_obj_del_delayed(page_scan_chart_contain, 500);
    lv_fun_delayed(page_scan_create, 500);
    lv_group_del(scan_group);
}

void page_scan_chart_create()
{
    time_repeat_count = 0;
    page_scan_chart_contain = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(page_scan_chart_contain);
    lv_obj_set_style_bg_color(page_scan_chart_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(page_scan_chart_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(page_scan_chart_contain, 160, 80);
    lv_obj_set_pos(page_scan_chart_contain, 0, 0);


    rssi_quality_chart = lv_chart_create(page_scan_chart_contain);
    lv_obj_remove_style(rssi_quality_chart, NULL, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rssi_quality_chart, (lv_opa_t)LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_size(rssi_quality_chart, 140, 60);
    lv_obj_set_pos(rssi_quality_chart, 10, 5);
    lv_chart_set_type(rssi_quality_chart, LV_CHART_TYPE_LINE);   /*Show lines and points too*/
    lv_obj_set_style_text_color(rssi_quality_chart, lv_color_white(), LV_STATE_DEFAULT);
    lv_chart_set_range(rssi_quality_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_range(rssi_quality_chart, LV_CHART_AXIS_PRIMARY_X, 0, 8);

    lv_obj_set_style_border_color(rssi_quality_chart, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(rssi_quality_chart, lv_color_make(100, 100, 100), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(rssi_quality_chart, 100, LV_STATE_DEFAULT);
    //lv_obj_set_style_text_color(rssi_quality_chart, lv_color_make(0, 0, 255), LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(rssi_quality_chart, 0, LV_STATE_DEFAULT);
    //lv_obj_set_style_text_color_filtered(rssi_quality_chart, lv_color_make(0, 0, 255), LV_STATE_DEFAULT);
    //lv_obj_set_style_text_font(rssi_quality_chart, &lv_font_montserrat_8,LV_STATE_DEFAULT);


    lv_obj_set_style_size(rssi_quality_chart, 0, LV_PART_INDICATOR);
    lv_chart_set_div_line_count(rssi_quality_chart, 5, 8);
    //lv_chart_set_range(rssi_quality_chart, LV_CHART_AXIS_SECONDARY_Y, -50, 100);
    lv_chart_set_point_count(rssi_quality_chart, 48);
    lv_chart_set_axis_tick(rssi_quality_chart, LV_CHART_AXIS_PRIMARY_X, 5, 3, 6, 1, true, 40);
    lv_chart_set_axis_tick(rssi_quality_chart, LV_CHART_AXIS_PRIMARY_Y, 5, 3, 5, 2, true, 50);

    
		if(RX5808_Get_Signal_Source()==1)
		{						
			rssi1_curve = lv_chart_add_series(rssi_quality_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
		}
		else if(RX5808_Get_Signal_Source()==2)
		{
			rssi0_curve = lv_chart_add_series(rssi_quality_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
		}
		else
		{
			rssi0_curve = lv_chart_add_series(rssi_quality_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
			rssi1_curve = lv_chart_add_series(rssi_quality_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
		}
   


    chart_fre_label = lv_label_create(page_scan_chart_contain);

    lv_obj_set_style_text_font(chart_fre_label, &lv_font_montserrat_8, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(chart_fre_label, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(chart_fre_label, 255, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(chart_fre_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_label_set_text_fmt(chart_fre_label, "5300     5450     5600     5750     5900");
    lv_obj_set_pos(chart_fre_label, 10, 68);


    scan_group = lv_group_create();
    lv_indev_set_group(indev_keypad, scan_group);


    lv_obj_add_event_cb(rssi_quality_chart, page_scan_event_callback, LV_EVENT_KEY, NULL);

    lv_group_add_obj(scan_group, rssi_quality_chart);
    lv_group_set_editing(scan_group, false);

    RX5808_Set_Freq(5300);
    scan_chart_timer = lv_timer_create(page_scan_chart_timer_event, scan_turn_time, NULL);
    lv_timer_set_repeat_count(scan_chart_timer, 48);

    lv_amin_start(rssi_quality_chart, -60, 5, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_chart_anim_enter);
    lv_amin_start(chart_fre_label, 80, 68, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_chart_anim_enter);

}

