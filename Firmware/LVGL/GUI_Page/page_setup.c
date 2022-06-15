#include "page_setup.h"
#include "page_menu.h"
#include "lcd.h"
#include "beep.h"
#include "page_start.h"
#include "rx5808_config.h"
#include "lvgl_stl.h"

#define page_setup_anim_enter  lv_anim_path_bounce
#define page_setup_anim_leave  lv_anim_path_bounce

#define LABEL_FOCUSE_COLOR    lv_color_make(255, 100, 0)
#define LABEL_DEFAULT_COLOR   lv_color_make(255, 255, 255)
#define BAR_COLOR             lv_color_make(255, 168, 0)
#define SWITCH_COLOR          lv_color_make(255, 0, 128)

static lv_obj_t* menu_setup_contain = NULL;
static lv_obj_t* back_light_label;
static lv_obj_t* boot_animation_label;
static lv_obj_t* beep_label;
static lv_obj_t* exit_label;
static lv_obj_t* back_light_bar;
static lv_obj_t* boot_animation_switch;
static lv_obj_t* beep_switch;
static lv_group_t* setup_group;

static lv_style_t style_label;

static uint8_t setup_back_light;
static bool  boot_animation_state;
static bool  beep_state;

static void page_setup_exit(void);
static void page_setup_style_deinit(void);


static void setup_event_callback(lv_event_t* event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t* obj = lv_event_get_target(event);
    if (code == LV_EVENT_KEY)
    {
        beep_on_off(1);
        lv_fun_param_delayed(beep_on_off, 100, 0);
        lv_key_t key_status = lv_indev_get_key(lv_indev_get_act());
        if (key_status == LV_KEY_ENTER) {
            if (obj == exit_label) {
                page_set_animation_en(lv_obj_has_state(boot_animation_switch, LV_STATE_CHECKED));
                beep_set_enable_disable(lv_obj_has_state(beep_switch, LV_STATE_CHECKED));
                LCD_SET_BLK(setup_back_light);
                rx5808_div_setup_upload();
                page_setup_exit();
            }
            else if (obj == boot_animation_label)
            {
                if (lv_obj_has_state(boot_animation_switch, LV_STATE_CHECKED) == true)
                    lv_obj_clear_state(boot_animation_switch, LV_STATE_CHECKED);
                else
                    lv_obj_add_state(boot_animation_switch, LV_STATE_CHECKED);
            }
            else if (obj == beep_label)
            {
                if (lv_obj_has_state(beep_switch, LV_STATE_CHECKED) == true)
                    lv_obj_clear_state(beep_switch, LV_STATE_CHECKED);
                else
                    lv_obj_add_state(beep_switch, LV_STATE_CHECKED);
            }
        }
        else if (key_status == LV_KEY_LEFT) {
            if (obj == back_light_label)
            {
                setup_back_light -= 5;
                if (setup_back_light < 10)
                    setup_back_light = 10;
                LCD_SET_BLK(setup_back_light);
                lv_bar_set_value(back_light_bar, setup_back_light, LV_ANIM_OFF);
            }
            else if (obj == boot_animation_label)
            {
                lv_obj_clear_state(boot_animation_switch, LV_STATE_CHECKED);
            }
            else if (obj == beep_label)
            {
                lv_obj_clear_state(beep_switch, LV_STATE_CHECKED);
            }
            else if (obj == exit_label)
            {

            }
        }
        else if (key_status == LV_KEY_RIGHT) {
            if (obj == back_light_label)
            {
                setup_back_light += 5;
                if (setup_back_light > 100)
                    setup_back_light = 100;
                LCD_SET_BLK(setup_back_light);
                lv_bar_set_value(back_light_bar, setup_back_light, LV_ANIM_OFF);
            }
            else if (obj == boot_animation_label)
            {
                lv_obj_add_state(boot_animation_switch, LV_STATE_CHECKED);
            }
            else if (obj == beep_label)
            {
                lv_obj_add_state(beep_switch, LV_STATE_CHECKED);
            }
            else if (obj == exit_label)
            {

            }
        }
        else if (key_status == LV_KEY_UP) {
            lv_group_focus_prev(setup_group);
        }
        else if (key_status == LV_KEY_DOWN) {
            lv_group_focus_next(setup_group);
        }
    }
}

static void page_setup_style_init()
{
    lv_style_init(&style_label);
    lv_style_set_bg_color(&style_label, LABEL_DEFAULT_COLOR);
    lv_style_set_bg_opa(&style_label, LV_OPA_COVER);
    lv_style_set_text_color(&style_label, lv_color_black());
    lv_style_set_border_width(&style_label, 2);
    lv_style_set_text_font(&style_label, &lv_font_montserrat_12);
    lv_style_set_text_opa(&style_label, LV_OPA_COVER);
    lv_style_set_radius(&style_label, 4);
    lv_style_set_x(&style_label, 100);
}


static void page_setup_style_deinit()
{
    lv_style_reset(&style_label);
}



void page_setup_create()
{
    page_setup_style_init();

    menu_setup_contain = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(menu_setup_contain);
    lv_obj_set_style_bg_color(menu_setup_contain, lv_color_make(0, 0, 0), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(menu_setup_contain, (lv_opa_t)LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_size(menu_setup_contain, 160, 80);
    lv_obj_set_pos(menu_setup_contain, 0, 0);


    back_light_label = lv_label_create(menu_setup_contain);
    lv_obj_add_style(back_light_label, &style_label, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(back_light_label, LABEL_FOCUSE_COLOR, LV_STATE_FOCUSED);
    lv_label_set_text_fmt(back_light_label, "BackLight");
    lv_obj_set_pos(back_light_label, 0, 2);
    lv_obj_set_size(back_light_label, 75, 20);
    lv_label_set_long_mode(back_light_label, LV_LABEL_LONG_WRAP);

    back_light_bar = lv_bar_create(menu_setup_contain);
    lv_obj_remove_style(back_light_bar, NULL, LV_PART_KNOB);
    lv_obj_set_size(back_light_bar, 50, 14);
    lv_obj_set_style_bg_color(back_light_bar, BAR_COLOR, LV_PART_INDICATOR);
    lv_obj_set_pos(back_light_bar, 110, 5);
    setup_back_light = LCD_GET_BLK();
    lv_bar_set_value(back_light_bar, setup_back_light, LV_ANIM_ON);
    lv_obj_set_style_anim_time(back_light_bar, 200, LV_STATE_DEFAULT);

    boot_animation_label = lv_label_create(menu_setup_contain);
    lv_obj_add_style(boot_animation_label, &style_label, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(boot_animation_label, LABEL_FOCUSE_COLOR, LV_STATE_FOCUSED);
    lv_label_set_text_fmt(boot_animation_label, "Boot Logo");
    lv_obj_set_size(boot_animation_label, 75, 20);
    lv_obj_set_pos(boot_animation_label, 0, 21);
    lv_label_set_long_mode(boot_animation_label, LV_LABEL_LONG_WRAP);

    boot_animation_switch = lv_switch_create(menu_setup_contain);
    lv_obj_set_style_border_opa(boot_animation_switch, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(boot_animation_switch, 50, 14);
    lv_obj_set_pos(boot_animation_switch, 110, 24);
    lv_obj_set_style_bg_color(boot_animation_switch, SWITCH_COLOR, LV_PART_INDICATOR | LV_STATE_CHECKED);
    boot_animation_state = page_get_animation_en();
    if (boot_animation_state == true)
        lv_obj_add_state(boot_animation_switch, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(boot_animation_switch, LV_STATE_CHECKED);

    beep_label = lv_label_create(menu_setup_contain);
    lv_obj_add_style(beep_label, &style_label, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(beep_label, LABEL_FOCUSE_COLOR, LV_STATE_FOCUSED);
    lv_label_set_text_fmt(beep_label, "Beep");
    lv_obj_set_pos(beep_label, 0, 40);
    lv_obj_set_size(beep_label, 75, 20);
    lv_label_set_long_mode(beep_label, LV_LABEL_LONG_WRAP);



    beep_switch = lv_switch_create(menu_setup_contain);
    lv_obj_set_style_border_opa(beep_switch, 0, LV_STATE_DEFAULT);
    lv_obj_set_size(beep_switch, 50, 14);
    lv_obj_set_pos(beep_switch, 110, 43);
    lv_obj_set_style_bg_color(beep_switch, SWITCH_COLOR, LV_PART_INDICATOR | LV_STATE_CHECKED);
    beep_state = beep_get_status();
    if (beep_state == true)
        lv_obj_add_state(beep_switch, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(beep_switch, LV_STATE_CHECKED);


    exit_label = lv_label_create(menu_setup_contain);
    lv_obj_add_style(exit_label, &style_label, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(exit_label, LABEL_FOCUSE_COLOR, LV_STATE_FOCUSED);
    lv_label_set_text_fmt(exit_label, "SAVE&EXIT");
    lv_obj_set_pos(exit_label, 0, 59);
    lv_obj_set_size(exit_label, 75, 20);
    lv_label_set_long_mode(exit_label, LV_LABEL_LONG_WRAP);

    setup_group = lv_group_create();
    lv_indev_set_group(indev_keypad, setup_group);
    lv_obj_add_event_cb(boot_animation_label, setup_event_callback, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(back_light_label, setup_event_callback, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(beep_label, setup_event_callback, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(exit_label, setup_event_callback, LV_EVENT_KEY, NULL);

    lv_group_add_obj(setup_group, back_light_label);
    lv_group_add_obj(setup_group, boot_animation_label);
    lv_group_add_obj(setup_group, beep_label);
    lv_group_add_obj(setup_group, exit_label);
    lv_group_set_editing(setup_group, true);




    lv_amin_start(back_light_label, -100, 0, 1, 200, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_enter);
    lv_amin_start(boot_animation_label, -100, 0, 1, 200, 100, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_enter);
    lv_amin_start(beep_label, -100, 0, 1, 200, 200, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_enter);
    lv_amin_start(exit_label, -100, 0, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_enter);

    lv_amin_start(back_light_bar, 160, 110, 1, 200, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_enter);
    lv_amin_start(boot_animation_switch, 160, 110, 1, 200, 100, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_enter);
    lv_amin_start(beep_switch, 160, 110, 1, 200, 200, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_enter);

}


static void page_setup_exit()
{
    lv_amin_start(back_light_label, lv_obj_get_x(back_light_label), -100, 1, 200, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_leave);
    lv_amin_start(boot_animation_label, lv_obj_get_x(boot_animation_label), -100, 1, 200, 100, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_leave);
    lv_amin_start(beep_label, lv_obj_get_x(beep_label), -100, 1, 200, 200, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_leave);
    lv_amin_start(exit_label, lv_obj_get_x(exit_label), -100, 1, 200, 300, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_leave);

    lv_amin_start(back_light_bar, lv_obj_get_x(back_light_bar), 160, 1, 200, 0, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_leave);
    lv_amin_start(boot_animation_switch, lv_obj_get_x(boot_animation_switch), 160, 1, 200, 100, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_leave);
    lv_amin_start(beep_switch, lv_obj_get_x(beep_switch), 160, 1, 200, 200, (lv_anim_exec_xcb_t)lv_obj_set_x, page_setup_anim_leave);

    lv_fun_delayed(page_setup_style_deinit, 500);
    lv_group_del(setup_group);
    lv_obj_del_delayed(menu_setup_contain, 500);
    lv_fun_param_delayed(page_menu_create, 500, item_setup);
}

