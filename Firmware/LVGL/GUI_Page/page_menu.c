#include "page_menu.h"
#include "page_about.h"
#include "page_setup.h"
#include "page_scan.h"
#include "page_main.h"
#include "lvgl_stl.h"
#include "rx5808.h"
#include "lcd.h"
#include "beep.h"


#define page_menu_anim_enter  lv_anim_path_bounce
#define page_menu_anim_leave  lv_anim_path_bounce

#define menu_item_count item_count


LV_IMG_DECLARE(menu_setup_icon);
LV_IMG_DECLARE(menu_rx_icon);
LV_IMG_DECLARE(menu_about_icon);

static lv_obj_t* menu_contain;
static lv_style_t style_icon;
static lv_style_t style_text;
static lv_group_t* menu_group;

static lv_timer_t* menu_timer;

static menu_item rx5808_div_menu[menu_item_count];

static void menu_timer_event(lv_timer_t* tmr);
static void page_menu_exit(void);
static void page_menu_style_init(void);
static void page_menu_style_deinit(void);
static void menu_item_label_update(void);
static void page_menu_item_create(lv_obj_t* parent, menu_item* item, uint8_t i);

static const char icon_txt_array[menu_item_count][10] = { "Scan","Setup","About" };
static const lv_img_dsc_t* icon_imagine[menu_item_count] = { &menu_rx_icon,&menu_setup_icon,&menu_about_icon };



void Menu_event_callback(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_target(event);
    lv_key_t key_status = lv_indev_get_key(lv_indev_get_act());
    if (code == LV_EVENT_KEY)
    {
        beep_on_off(1);
        lv_fun_param_delayed(beep_on_off, 100, 0);
       
            if (key_status == LV_KEY_ENTER) {
                if (obj == rx5808_div_menu[item_setup].item_contain){
                    page_menu_exit();
                    page_setup_create();
                }
                else if (obj == rx5808_div_menu[item_scan].item_contain){
                    page_menu_exit();
                    page_scan_create();
                }
                else if (obj == rx5808_div_menu[item_about].item_contain){
                    page_menu_exit();
                    page_about_create();
                }
            }
            else if (key_status == LV_KEY_LEFT) {
                page_menu_exit();
                page_main_create();
            }
            else if (key_status == LV_KEY_RIGHT) {

            }
            else if (key_status == LV_KEY_UP) {
                lv_group_focus_prev(menu_group);
            }
            else if (key_status == LV_KEY_DOWN) {
                lv_group_focus_next(menu_group);

            }
        }
}


static void menu_timer_event(lv_timer_t* tmr)
{
    menu_item_label_update();
}


static void group_obj_scroll(lv_group_t* g)
{
    lv_obj_t* icon = lv_group_get_focused(g);
    lv_coord_t y = lv_obj_get_y(icon);
    lv_obj_scroll_to_y(lv_obj_get_parent(icon), y, LV_ANIM_ON);
}


void page_menu_create(uint8_t item_id)
{
    page_menu_style_init();

    menu_contain = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(menu_contain);
    lv_obj_set_style_bg_color(menu_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(menu_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(menu_contain, 160, 80);
    lv_obj_set_pos(menu_contain, 0, 0);
    lv_obj_clear_flag(menu_contain, LV_OBJ_FLAG_SCROLLABLE);

    menu_group = lv_group_create();
    lv_indev_set_group(indev_keypad, menu_group);
    lv_group_set_editing(menu_group, true);

    lv_group_set_wrap(menu_group, true);
    lv_group_set_focus_cb(menu_group, group_obj_scroll);


    lv_obj_t* cont_col = lv_obj_create(menu_contain);
    lv_obj_set_style_bg_opa(cont_col, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_ver(cont_col, 0, 0);
    lv_obj_set_style_radius(cont_col, 0, LV_STATE_DEFAULT);
    lv_obj_clear_flag(cont_col, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_size(cont_col, 160, 80);
    lv_obj_set_style_bg_color(cont_col, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cont_col, lv_color_make(0x00, 0x00, 0x00), LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(cont_col, LV_FLEX_FLOW_COLUMN);

    lv_obj_set_flex_align(
        cont_col,
        LV_FLEX_ALIGN_START,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_SPACE_BETWEEN
    );

    uint32_t i;
    for (i = 0; i < menu_item_count; i++) {
        page_menu_item_create(cont_col, &rx5808_div_menu[(i + item_id) % menu_item_count], (i + item_id) % menu_item_count);
    }

    lv_amin_start(cont_col, 160, 0, 1, 500, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_menu_anim_enter);
    menu_timer = lv_timer_create(menu_timer_event, 1000, NULL);
}

static void page_menu_exit()
{
    lv_timer_del(menu_timer);
    page_menu_style_deinit();
    lv_obj_del(menu_contain);
    lv_group_del(menu_group);
}


static void page_menu_style_init()
{
    lv_style_init(&style_icon);
    lv_style_set_bg_color(&style_icon, lv_color_make(0x70, 0x00, 0x00));
    lv_style_set_bg_opa(&style_icon, LV_OPA_COVER);
    lv_style_set_text_color(&style_icon, lv_color_white());
    lv_style_set_border_side(&style_icon, LV_BORDER_SIDE_RIGHT);
    lv_style_set_border_width(&style_icon, 2);
    lv_style_set_border_color(&style_icon, lv_color_make(0x00, 0x00, 0xff));

    lv_style_init(&style_text);
    lv_style_set_bg_color(&style_text, lv_color_make(0x70, 0x00, 0x00));
    lv_style_set_bg_opa(&style_text, LV_OPA_COVER);
    lv_style_set_text_color(&style_text, lv_color_white());
    lv_style_set_border_side(&style_text, LV_BORDER_SIDE_RIGHT);
    lv_style_set_border_width(&style_text, 2);
    lv_style_set_border_color(&style_text, lv_color_make(0x00, 0x00, 0xff));
}

static void page_menu_style_deinit()
{
    lv_style_reset(&style_icon);
    lv_style_reset(&style_text);
}

static void menu_item_label_update()
{
    for (int i = 0; i < menu_item_count; i++) {
        switch (i)
        {
           
        case item_scan:  lv_label_set_text_fmt(rx5808_div_menu[i].item_label0, "RSSI1: %d%%", (int)Rx5808_Get_Precentage1());
            lv_label_set_text_fmt(rx5808_div_menu[i].item_label1, "RSSI2: %d%%", (int)Rx5808_Get_Precentage0()); break;
        //case item_setup: lv_label_set_text_fmt(rx5808_div_menu[i].item_label0, "Light:%d%%", LCD_GET_BLK()); break;
        case item_about: lv_label_set_text_fmt(rx5808_div_menu[i].item_label0, "Battery:%.2fV", Get_Battery_Voltage()); break;
        default:break;
        }
    }
}


static void page_menu_item_create(lv_obj_t* parent, menu_item* item, uint8_t i)
{
    item->item_contain = lv_obj_create(parent);

    lv_obj_remove_style_all(item->item_contain);
    lv_obj_set_size(item->item_contain, 150, 80);
    lv_obj_set_style_bg_opa(item->item_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);      
    lv_obj_set_style_bg_color(item->item_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);


    item->item_imag = lv_img_create(item->item_contain);
    lv_obj_set_size(item->item_imag, 50, 50);
    lv_img_set_src(item->item_imag, icon_imagine[i]);
    lv_obj_align(item->item_imag, LV_ALIGN_LEFT_MID, 0, 0);

    item->item_title = lv_label_create(item->item_contain);
    lv_label_set_text_fmt(item->item_title, "%s", &icon_txt_array[i]);
    lv_obj_set_style_text_font(item->item_title, &lv_font_montserrat_16, LV_STATE_DEFAULT);
    lv_obj_align_to(item->item_title, item->item_imag, LV_ALIGN_OUT_RIGHT_TOP, 5, 0);
    lv_obj_set_style_text_color(item->item_title, lv_color_make(255, 255, 255), LV_STATE_DEFAULT);

    item->item_label0 = lv_label_create(item->item_contain);
    lv_obj_align_to(item->item_label0, item->item_imag, LV_ALIGN_OUT_RIGHT_TOP, 5, 20);
    lv_obj_set_style_text_font(item->item_label0, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(item->item_label0, lv_color_make(198, 198, 198), LV_STATE_DEFAULT);

    item->item_label1 = lv_label_create(item->item_contain);
    lv_obj_align_to(item->item_label1, item->item_imag, LV_ALIGN_OUT_RIGHT_TOP, 5, 35);
    lv_obj_set_style_text_font(item->item_label1, &lv_font_montserrat_12, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(item->item_label1, lv_color_make(198, 198, 198), LV_STATE_DEFAULT);


    switch (i)
    {   
    case item_scan:  lv_label_set_text_fmt(item->item_label0, "RSSI1: %d%%", (int)Rx5808_Get_Precentage1());
        lv_label_set_text_fmt(item->item_label1, "RSSI2: %d%%", (int)Rx5808_Get_Precentage0()); break;
    case item_setup: lv_label_set_text_fmt(item->item_label0, "BLCK:%d%%", LCD_GET_BLK());
        lv_label_set_text_fmt(item->item_label1, beep_get_status() ? "BEEP:Open" : "BEEP:Close"); break;
    case item_about: lv_label_set_text_fmt(item->item_label0, "Battery:%.2fV", Get_Battery_Voltage());
        lv_label_set_text_fmt(item->item_label1, "LVGL:v%d.%d.%d", LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH); break;
    default:break;
    }

    lv_group_add_obj(menu_group, item->item_contain);
    lv_obj_add_event_cb(item->item_contain, Menu_event_callback, LV_EVENT_KEY, NULL);
}
