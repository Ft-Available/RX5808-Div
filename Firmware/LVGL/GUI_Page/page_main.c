#include "page_main.h"
#include "page_menu.h"
#include "rx5808.h"
#include "rx5808_config.h"
#include "lvgl_stl.h"
#include <stdlib.h>

#define page_main_anim_enter  lv_anim_path_bounce
#define page_main_anim_leave  lv_anim_path_bounce


LV_IMG_DECLARE(lock_img);
LV_FONT_DECLARE(lv_font_fre);

static lv_obj_t* main_contain;
static lv_obj_t* lock_btn;
static lv_obj_t* lv_channel_label;
static lv_obj_t* frequency_label_contain;
static lv_obj_t* frequency_label[fre_pre_cur_count][fre_label_count];

static lv_obj_t* rssi_bar0;
static lv_obj_t* rssi_label0;

static lv_obj_t* rssi_bar1;
static lv_obj_t* rssi_label1;
static lv_obj_t* lv_rsss0_label;
static lv_obj_t* lv_rsss1_label;

static lv_timer_t* page_main_update_timer;

static bool lock_flag = false;    //lock

static void event_callback(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    if (code == LV_EVENT_KEY)
    {
        if (lock_flag == true)
        {
            lv_key_t key_status = lv_indev_get_key(lv_indev_get_act());
            if (key_status>=LV_KEY_UP&&key_status<= LV_KEY_LEFT){
                beep_on_off(1);
                lv_fun_param_delayed(beep_on_off, 100, 0);
            }           
            if (key_status == LV_KEY_LEFT) {
                channel_count--;
                if (channel_count < 0)
                    channel_count = 7;
                RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
                Rx5808_Set_Channel(channel_count + Chx_count * 8);
                rx5808_div_setup_upload();
                fre_label_update(Chx_count, channel_count);
                lv_label_set_text_fmt(lv_channel_label, "%c%d", 'A' + Chx_count, channel_count + 1);
            }
            else if (key_status == LV_KEY_RIGHT) {
                channel_count++;
                if (channel_count > 7)
                    channel_count = 0;
                RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
                Rx5808_Set_Channel(channel_count + Chx_count * 8);
                rx5808_div_setup_upload();
                fre_label_update(Chx_count, channel_count);
                lv_label_set_text_fmt(lv_channel_label, "%c%d", 'A' + Chx_count, channel_count + 1);

            }
            else if (key_status == LV_KEY_UP) {
                Chx_count--;
                if (Chx_count < 0)
                    Chx_count = 5;
                RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
                Rx5808_Set_Channel(channel_count + Chx_count * 8);
                rx5808_div_setup_upload();
                fre_label_update(Chx_count, channel_count);
                lv_label_set_text_fmt(lv_channel_label, "%c%d", 'A' + Chx_count, channel_count + 1);
            }
            else if (key_status == LV_KEY_DOWN) {
                Chx_count++;
                if (Chx_count > 5)
                    Chx_count = 0;
                RX5808_Set_Freq(Rx5808_Freq[Chx_count][channel_count]);
                Rx5808_Set_Channel(channel_count + Chx_count * 8);
                rx5808_div_setup_upload();
                fre_label_update(Chx_count, channel_count);
                lv_label_set_text_fmt(lv_channel_label, "%c%d", 'A' + Chx_count, channel_count + 1);
            }
        }
    }
    else if (code == LV_EVENT_SHORT_CLICKED)
    {
        beep_on_off(1);
        lv_fun_param_delayed(beep_on_off, 100, 0);
        page_main_exit();
        lv_fun_param_delayed(page_menu_create, 500, 0);
    }
    else if (code == LV_EVENT_LONG_PRESSED)
    {
        beep_on_off(1);
        lv_fun_param_delayed(beep_on_off, 100, 0);
        if (lock_flag == false)
        {
            lv_obj_set_style_bg_color(lock_btn, lv_color_make(160, 160, 160), LV_STATE_DEFAULT);
            lock_flag = true;

        }
        else
        {
            lv_obj_set_style_bg_color(lock_btn, lv_color_make(255, 0, 0), LV_STATE_DEFAULT);
            lock_flag = false;
        }
    }
}

static void page_main_update(lv_timer_t* tmr)
{
    int rssi0 = (int)Rx5808_Get_Precentage0();
    int rssi1 = (int)Rx5808_Get_Precentage1();

    lv_bar_set_value(rssi_bar0, rssi1, LV_ANIM_ON);
    lv_label_set_text_fmt(rssi_label0, "%d", rssi1);
    lv_bar_set_value(rssi_bar1, rssi0, LV_ANIM_ON);
    lv_label_set_text_fmt(rssi_label1, "%d", rssi0);
}

static void fre_pre_label_update()
{

    for (int i = 0; i < fre_label_count; i++)
    {
        lv_label_set_text_fmt(frequency_label[fre_pre][i], "%c", lv_label_get_text(frequency_label[fre_cur][i])[0]);
    }
}

static void fre_label_update(uint8_t a, uint8_t b)
{
    for (int i = 0; i < fre_label_count; i++)
    {
        lv_label_set_text_fmt(frequency_label[fre_cur][i], "%d", (Rx5808_Freq[a][b] / lv_pow(10, fre_label_count - i - 1)) % 10);
        if (lv_label_get_text(frequency_label[fre_pre][i])[0] != lv_label_get_text(frequency_label[fre_cur][i])[0])
        {
            lv_amin_start(frequency_label[fre_cur][i], lv_pow(-1, (i % 2)) * 50, 0, 1, 200, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, lv_anim_path_ease_in);
            lv_amin_start(frequency_label[fre_pre][i], 0, lv_pow(-1, (i % 2)) * -50, 1, 200, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, lv_anim_path_ease_in);
        }
    }
    lv_fun_delayed(fre_pre_label_update, 200);
}

void page_main_create()
{
    main_contain = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(main_contain);
    lv_obj_set_style_bg_color(main_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(main_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(main_contain, 160, 80);
    lv_obj_set_pos(main_contain, 0, 0);


    frequency_label_contain = lv_obj_create(main_contain);
    lv_obj_remove_style_all(frequency_label_contain);
    lv_obj_set_style_bg_color(frequency_label_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(frequency_label_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(frequency_label_contain, 130, 50);
    lv_obj_set_pos(frequency_label_contain, 30, 0);


    for (int i = 0; i < fre_label_count; i++)
    {
        frequency_label[fre_pre][i] = lv_label_create(frequency_label_contain);
        lv_label_set_long_mode(frequency_label[fre_pre][i], LV_LABEL_LONG_WRAP);
        lv_obj_set_pos(frequency_label[fre_pre][i], 32 * i, -50);
        lv_obj_set_style_text_font(frequency_label[fre_pre][i], &lv_font_fre, LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(frequency_label[fre_pre][i], lv_color_make(255, 157, 0), LV_STATE_DEFAULT);
        lv_label_set_text_fmt(frequency_label[fre_pre][i], "%d", (Rx5808_Freq[Chx_count][channel_count] / lv_pow(10, fre_label_count - i - 1)) % 10);
        lv_label_set_long_mode(frequency_label[fre_pre][i], LV_LABEL_LONG_WRAP);
    }

    for (int i = 0; i < fre_label_count; i++)
    {
        frequency_label[fre_cur][i] = lv_label_create(frequency_label_contain);
        lv_label_set_long_mode(frequency_label[fre_cur][i], LV_LABEL_LONG_WRAP);
        lv_obj_set_pos(frequency_label[fre_cur][i], 32 * i, 0);
        lv_obj_set_style_text_font(frequency_label[fre_cur][i], &lv_font_fre, LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(frequency_label[fre_cur][i], lv_color_make(255, 157, 0), LV_STATE_DEFAULT);
        lv_label_set_text_fmt(frequency_label[fre_cur][i], "%d", (Rx5808_Freq[Chx_count][channel_count] / lv_pow(10, fre_label_count - i - 1)) % 10);
        lv_label_set_long_mode(frequency_label[fre_cur][i], LV_LABEL_LONG_WRAP);
    }

    lv_channel_label = lv_label_create(main_contain);
    lv_label_set_long_mode(lv_channel_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(lv_channel_label, LV_ALIGN_TOP_LEFT, 5, 2);
    lv_obj_set_style_text_font(lv_channel_label, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_channel_label, lv_color_make(0, 157, 255), LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_channel_label, true);
    lv_label_set_text_fmt(lv_channel_label, "%c%d", 'A' + Chx_count, channel_count + 1);

    lv_rsss0_label = lv_label_create(main_contain);
    lv_label_set_long_mode(lv_rsss0_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(lv_rsss0_label, LV_ALIGN_TOP_LEFT, 0, 50);
    lv_obj_set_style_text_font(lv_rsss0_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_rsss0_label, lv_color_make(0, 0, 255), LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_rsss0_label, true);
    lv_label_set_text(lv_rsss0_label, "RSSI1:");

    lv_rsss1_label = lv_label_create(main_contain);
    lv_label_set_long_mode(lv_rsss1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(lv_rsss1_label, LV_ALIGN_TOP_LEFT, 0, 65);
    lv_obj_set_style_text_font(lv_rsss1_label, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_rsss1_label, lv_color_make(200, 0, 200), LV_STATE_DEFAULT);
    lv_label_set_recolor(lv_rsss1_label, true);
    lv_label_set_text(lv_rsss1_label, "RSSI2:");

    rssi_bar0 = lv_bar_create(main_contain);

    lv_obj_remove_style(rssi_bar0, NULL, LV_PART_KNOB);
    lv_obj_set_size(rssi_bar0, 100, 12);
    lv_obj_set_style_bg_color(rssi_bar0, lv_color_make(0, 0, 200), LV_PART_INDICATOR);
    lv_obj_set_pos(rssi_bar0, 40, 50);
    lv_bar_set_value(rssi_bar0, Rx5808_Get_Precentage1(), LV_ANIM_ON);
    lv_obj_set_style_anim_time(rssi_bar0, 200, LV_STATE_DEFAULT);

    rssi_label0 = lv_label_create(main_contain);
    lv_obj_align_to(rssi_label0, rssi_bar0, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_opa(rssi_label0, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(rssi_label0, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(rssi_label0, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(rssi_label0, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(rssi_label0, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_label_set_recolor(rssi_label0, true);
    lv_label_set_text_fmt(rssi_label0, "%d", Rx5808_Get_Precentage1());


    rssi_bar1 = lv_bar_create(main_contain);
    lv_obj_remove_style(rssi_bar1, NULL, LV_PART_KNOB);
    lv_obj_set_size(rssi_bar1, 100, 12);
    lv_obj_set_style_bg_color(rssi_bar1, lv_color_make(200, 0, 200), LV_PART_INDICATOR);
    lv_obj_set_pos(rssi_bar1, 40, 65);
    lv_bar_set_value(rssi_bar1, Rx5808_Get_Precentage0(), LV_ANIM_ON);
    lv_obj_set_style_anim_time(rssi_bar1, 200, LV_STATE_DEFAULT);


    rssi_label1 = lv_label_create(main_contain);
    lv_obj_align_to(rssi_label1, rssi_bar1, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_opa(rssi_label1, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(rssi_label1, 4, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(rssi_label1, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(rssi_label1, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(rssi_label1, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_label_set_recolor(rssi_label1, true);
    lv_label_set_text_fmt(rssi_label1, "%d", Rx5808_Get_Precentage0());

    lock_btn = lv_imgbtn_create(main_contain);
    lv_obj_remove_style(lock_btn, NULL, LV_PART_ANY);
    lv_obj_set_size(lock_btn, 20, 24);
    lv_obj_set_pos(lock_btn, 5, 23);
    lv_obj_set_style_bg_opa(lock_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_width(lock_btn, 20, LV_STATE_PRESSED);
    lv_obj_set_style_height(lock_btn, 24, LV_STATE_PRESSED);
    if (lock_flag == false)
        lv_obj_set_style_bg_color(lock_btn, lv_color_hex(0xff0000), LV_STATE_DEFAULT);
    else
        lv_obj_set_style_bg_color(lock_btn, lv_color_hex(0x999999), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(lock_btn, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(lock_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(lock_btn, 4, 0);
    lv_imgbtn_set_src(lock_btn, LV_IMGBTN_STATE_RELEASED, &lock_img, NULL, NULL);
    lv_imgbtn_set_src(lock_btn, LV_IMGBTN_STATE_PRESSED, &lock_img, NULL, NULL);

    page_main_update_timer = lv_timer_create(page_main_update, 500, NULL);

    lv_amin_start(lv_rsss0_label, 80, 50, 1, 800, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(rssi_label0, 80, 50, 1, 800, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(rssi_bar0, 80, 50, 1, 800, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(lv_rsss1_label, 95, 65, 1, 800, 200, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(rssi_label1, 95, 65, 1, 800, 200, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(rssi_bar1, 95, 65, 1, 800, 200, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);

    lv_amin_start(lv_channel_label, -48, 2, 1, 800, 200, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(lock_btn, -27, 23, 1, 800, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(frequency_label[fre_cur][0], -50, 0, 1, 800, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(frequency_label[fre_cur][1], -50, 0, 1, 800, 200, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(frequency_label[fre_cur][2], -50, 0, 1, 800, 400, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);
    lv_amin_start(frequency_label[fre_cur][3], -50, 0, 1, 800, 600, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_enter);

    lv_fun_delayed(page_main_group_create, 500);

}

static void page_main_group_create()
{
    lv_obj_add_event_cb(lock_btn, event_callback, LV_EVENT_ALL, NULL);
    lv_group_t* group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);
    lv_group_add_obj(group, lock_btn);
    lv_group_set_editing(group, false);

}

static void page_main_exit()
{
    lv_obj_remove_event_cb(lock_btn, event_callback);
    lv_timer_del(page_main_update_timer);
    lv_amin_start(lv_rsss0_label, lv_obj_get_y(lv_rsss0_label), 80, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(rssi_label0, lv_obj_get_y(rssi_label0), 80, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(rssi_bar0, lv_obj_get_y(rssi_bar0), 80, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(lv_rsss1_label, lv_obj_get_y(lv_rsss1_label), 95, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(rssi_label1, lv_obj_get_y(rssi_label1), 95, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(rssi_bar1, lv_obj_get_y(rssi_bar1), 95, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);

    lv_amin_start(lv_channel_label, lv_obj_get_y(lv_channel_label), -48, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(lock_btn, lv_obj_get_y(lock_btn), -27, 1, 800, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(frequency_label[fre_cur][0], lv_obj_get_y(frequency_label[fre_cur][0]), -50, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(frequency_label[fre_cur][1], lv_obj_get_y(frequency_label[fre_cur][1]), -50, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(frequency_label[fre_cur][2], lv_obj_get_y(frequency_label[fre_cur][2]), -50, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);
    lv_amin_start(frequency_label[fre_cur][3], lv_obj_get_y(frequency_label[fre_cur][3]), -50, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_y, page_main_anim_leave);

    lv_obj_del_delayed(main_contain, 500);
}
