#include "page_scan.h"
#include "page_scan_table.h"
#include "page_menu.h"
#include "page_main.h"
#include "rx5808.h"
#include "rx5808_config.h"
#include "lvgl_stl.h"
#include "beep.h"

LV_FONT_DECLARE(lv_font_chinese_16);

#define page_scan_table_anim_enter  lv_anim_path_bounce
#define page_scan_table_anim_leave  lv_anim_path_bounce

#define scan_turn_time  100


static lv_obj_t* page_scan_table_contain = NULL;

static lv_style_t style_label;
static lv_group_t* scan_group;
static uint8_t time_repeat_count = 0;

static lv_obj_t* scan_info_label;
static lv_obj_t* fre_info_label;
static lv_timer_t* scan_table_timer;
static lv_obj_t* scan_info_cont;
static uint8_t  max_rssi;
static uint8_t  max_channel;

static void page_scan_table_timer_event(lv_timer_t* tmr);
static void scroll_event(lv_event_t* event);
static void page_scan_table_style_init(void);
static void page_scan_table_style_deinit(void);
static void group_obj_scroll(lv_group_t* g);
static void page_scan_table_exit(void);


static const char fre_channel_nale[6][2] = { "A","B","E","F","R","L" };
static const int fre_channel_num[] = { 1,2,3,4,5,6,7,8 };
static const lv_color_t channel_label_color[] = { {.full = 0XFC07},{.full = 0XFC00}, {.full = 0X841F},{.full = 0XF81F}, {.full = 0xBE7f},{.full = 0x7FFF} };


void page_scan_table_create(void);

static void page_scan_table_timer_event(lv_timer_t* tmr)
{
    static lv_obj_t* label_contain;
    int repeat_count = 47 - tmr->repeat_count;
    time_repeat_count = repeat_count;
    if (tmr == scan_table_timer)
    {
        if (repeat_count % 8 == 0)
        {
            label_contain = lv_obj_create(scan_info_cont);
            lv_obj_remove_style_all(label_contain);
            lv_obj_set_size(label_contain, 160, 22);
            lv_obj_set_style_bg_color(label_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(label_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
            lv_group_add_obj(scan_group, label_contain);
            lv_obj_add_event_cb(label_contain, scroll_event, LV_EVENT_KEY, NULL);

            lv_obj_t* obj = lv_label_create(label_contain);
            lv_obj_add_style(obj, &style_label, LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, channel_label_color[repeat_count / 8], LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, channel_label_color[repeat_count / 8], LV_STATE_DEFAULT);
            lv_label_set_text(obj, fre_channel_nale[repeat_count / 8]);
            lv_obj_set_pos(obj, 2, 0);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
        }

        lv_obj_t* obj = lv_label_create(label_contain);
        lv_obj_add_style(obj, &style_label, LV_STATE_DEFAULT);
        uint8_t rssi_pre = 0;
        if(RX5808_Get_Signal_Source()==1)
        {						
                rssi_pre=Rx5808_Get_Precentage1();
        }
        else if(RX5808_Get_Signal_Source()==2)
        {
                rssi_pre=Rx5808_Get_Precentage0();
        }
        else
        {
            rssi_pre=(Rx5808_Get_Precentage0() + Rx5808_Get_Precentage1()) / 2;
        }
        if (rssi_pre > max_rssi)
        {
            max_rssi = rssi_pre;
            max_channel = time_repeat_count;
        }
        if (rssi_pre >= 80)
        {
            lv_obj_set_style_text_color(obj, lv_color_make(0, 255, 0), LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_make(0, 255, 0), LV_STATE_DEFAULT);
        }
        else if (rssi_pre < 80 && rssi_pre >= 60)
        {
            lv_obj_set_style_text_color(obj, lv_color_make(255, 255, 0), LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_make(255, 255, 0), LV_STATE_DEFAULT);
        }
        else if (rssi_pre < 60 && rssi_pre >= 40)
        {
            lv_obj_set_style_text_color(obj, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
        }

        lv_label_set_text_fmt(obj, "%d", fre_channel_num[repeat_count % 8]);
        lv_obj_set_pos(obj, 17 * (repeat_count % 8) + 20, 0);
        lv_obj_set_style_border_width(obj, 2, LV_STATE_DEFAULT);
        lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
        lv_label_set_text_fmt(fre_info_label, "%c%d:%d", 'A' + time_repeat_count / 8, (time_repeat_count % 8) + 1, Rx5808_Freq[repeat_count / 8][repeat_count % 8]);
        if (repeat_count == 47)
        {
            if (RX5808_Get_Language() == 0)
            {
                lv_label_set_text_fmt(scan_info_label, "%s", "Finish!");
            }
            else
            {
                lv_label_set_text_fmt(scan_info_label, "%s", "扫描结束!");
            }
            lv_obj_set_style_text_opa(scan_info_label, LV_OPA_COVER, LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(scan_info_label, lv_color_make(0, 255, 0), LV_STATE_DEFAULT);
            lv_label_set_text_fmt(fre_info_label, "%c%d:%d", 'A' + max_channel / 8, (max_channel % 8) + 1, Rx5808_Freq[max_channel / 8][max_channel % 8]);
            RX5808_Set_Freq(Rx5808_Freq[max_channel / 8][max_channel % 8]);
            Rx5808_Set_Channel(max_channel);
            rx5808_div_setup_upload();
        }
        if (time_repeat_count < 47)
            RX5808_Set_Freq(Rx5808_Freq[(time_repeat_count + 1) / 8][(time_repeat_count + 1) % 8]);
    }

}


static void scroll_event(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_KEY)
    {
        beep_on_off(1);
        lv_fun_param_delayed(beep_on_off, 100, 0);
        lv_key_t key_status = lv_indev_get_key(lv_indev_get_act());
        if (key_status == LV_KEY_LEFT) {
            page_scan_table_exit();
        }
        else if (key_status == LV_KEY_UP) {
            lv_group_focus_prev(scan_group);

        }
        else if (key_status == LV_KEY_DOWN) {
            lv_group_focus_next(scan_group);
        }
    }
}

static void page_scan_table_style_init()
{
    lv_style_init(&style_label);
    lv_style_set_bg_color(&style_label, lv_color_make(0x00, 0x00, 0x00));
    lv_style_set_bg_opa(&style_label, LV_OPA_COVER);
    lv_style_set_text_color(&style_label, lv_color_make(0x40, 0x40, 0x40));
    lv_style_set_border_width(&style_label, 2);
    lv_style_set_text_font(&style_label, &lv_font_montserrat_16);
    lv_style_set_text_opa(&style_label, LV_OPA_COVER);
    lv_style_set_radius(&style_label, 4);
    lv_style_set_border_color(&style_label, lock_flag?lv_color_black():lv_color_make(0x40, 0x40, 0x40));
    lv_style_set_border_opa(&style_label, LV_OPA_COVER);
}

static void page_scan_table_style_deinit()
{
    lv_style_reset(&style_label);
}




static void group_obj_scroll(lv_group_t* g)
{
    lv_obj_t* icon = lv_group_get_focused(g);
    lv_coord_t y = lv_obj_get_y(icon);
    lv_obj_scroll_to_y(lv_obj_get_parent(icon), y, LV_ANIM_ON);
}

static void page_scan_table_exit()
{
    lv_amin_start(scan_info_label, lv_obj_get_y(scan_info_label), -20, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_table_anim_leave);
    lv_amin_start(fre_info_label, lv_obj_get_y(fre_info_label), -20, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_table_anim_leave);
    lv_amin_start(scan_info_cont, lv_obj_get_y(scan_info_cont), 80, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_table_anim_leave);
    if (time_repeat_count < 47)
    {
        RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
        lv_timer_del(scan_table_timer);
    }
    lv_obj_del_delayed(page_scan_table_contain, 500);
    lv_fun_delayed(page_scan_create, 500);
    lv_fun_delayed(page_scan_table_style_deinit, 500);
    lv_group_del(scan_group);
}

void page_scan_table_create()
{
    time_repeat_count = 0;
    max_rssi = 0;
    page_scan_table_contain = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(page_scan_table_contain);
    lv_obj_set_style_bg_color(page_scan_table_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(page_scan_table_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(page_scan_table_contain, 160, 80);
    lv_obj_set_pos(page_scan_table_contain, 0, 0);

    page_scan_table_style_init();
    scan_group = lv_group_create();
    lv_indev_set_group(indev_keypad, scan_group);

    lv_group_set_editing(scan_group, true);

    lv_group_set_wrap(scan_group, true);
    lv_group_set_focus_cb(scan_group, group_obj_scroll);

    scan_info_label = lv_label_create(page_scan_table_contain);
    lv_obj_set_pos(scan_info_label, 0, 2);
    lv_obj_set_style_bg_color(scan_info_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(scan_info_label, lv_color_make(0x00, 0x00, 0x00), LV_STATE_DEFAULT);
    //lv_label_set_text_fmt(scan_info_label, "%s", "Scaning....");
    //lv_obj_set_style_text_font(scan_info_label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scan_info_label, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(scan_info_label, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_label_set_long_mode(scan_info_label, LV_LABEL_LONG_WRAP);


    if (RX5808_Get_Language() == 0)
    {
        lv_label_set_text_fmt(scan_info_label, "%s", "Scaning....");
        lv_obj_set_style_text_font(scan_info_label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    }
    else
    {
        lv_label_set_text_fmt(scan_info_label, "%s", "扫描中....");
        lv_obj_set_style_text_font(scan_info_label, &lv_font_chinese_16, LV_STATE_DEFAULT);
    }


    fre_info_label = lv_label_create(page_scan_table_contain);
    //lv_obj_set_pos(scan_info_label, 0, 0);
    lv_obj_align(fre_info_label, LV_ALIGN_TOP_RIGHT, -2, 2);
    lv_obj_set_style_bg_color(fre_info_label, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(fre_info_label, lv_color_make(0x00, 0x00, 0x00), LV_STATE_DEFAULT);
    lv_label_set_text_fmt(fre_info_label, "%c%d:%d", 'A' + time_repeat_count / 8, time_repeat_count % 8, Rx5808_Freq[0][0]);
    lv_obj_set_style_text_font(fre_info_label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(fre_info_label, lv_color_make(255, 128, 255), LV_STATE_DEFAULT);
    lv_label_set_long_mode(fre_info_label, LV_LABEL_LONG_WRAP);

    scan_info_cont = lv_obj_create(page_scan_table_contain);
    lv_obj_set_size(scan_info_cont, 160, 60);
    lv_obj_set_pos(scan_info_cont, 0, 20);
    lv_obj_set_flex_flow(scan_info_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(scan_info_cont, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(scan_info_cont, lv_color_make(0x00, 0x00, 0x00), LV_STATE_DEFAULT);
    lv_obj_set_style_pad_ver(scan_info_cont, 0, 0);
    lv_obj_set_style_pad_gap(scan_info_cont, 4, 0);
    lv_obj_clear_flag(scan_info_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_align(
        scan_info_cont,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_SPACE_BETWEEN
    );

    lv_anim_t  anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, scan_info_label);
    lv_anim_set_values(&anim, 255, 0);
    lv_anim_set_repeat_count(&anim, 3);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_opa_cb);
    lv_anim_set_time(&anim, (scan_turn_time - 5) * 8);
    lv_anim_set_playback_time(&anim, (scan_turn_time - 5) * 8);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear);
    lv_anim_start(&anim);

    RX5808_Set_Freq(Rx5808_Freq[0][0]);
    scan_table_timer = lv_timer_create(page_scan_table_timer_event, scan_turn_time, NULL);
    lv_timer_set_repeat_count(scan_table_timer, 48);

    lv_amin_start(scan_info_label, -20, 0, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_table_anim_enter);
    lv_amin_start(fre_info_label, -20, 0, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_table_anim_enter);
    lv_amin_start(scan_info_cont, 80, 20, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_scan_table_anim_enter);
}