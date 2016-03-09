#pragma once

#define BATTERY_WIDTH 16
#define BATTERY_STROKE 3
#define BATTERY_HIGH_MIN 60
#define BATTERY_LOW_MAX 20
#define BATTERY_COLOR_HIGH GColorGreen
#define BATTERY_COLOR_MEDIUM GColorChromeYellow 
#define BATTERY_COLOR_LOW GColorRed
#define ARC_RADIUS_SUN PBL_IF_ROUND_ELSE(91, 70)
#define ARC_RADIUS_MARKER PBL_IF_ROUND_ELSE(72, 56)
#define ARC_RADIUS_ACTIVITY PBL_IF_ROUND_ELSE(85, 64)
#define ARC_RADIUS_HAND PBL_IF_ROUND_ELSE(45, 36)
#define HAND_LENGTH_MINUTE PBL_IF_ROUND_ELSE(14, 9)
#define HAND_LENGTH_HOUR PBL_IF_ROUND_ELSE(7, 5)
#define DATE_OFFSET_Y PBL_IF_ROUND_ELSE(-19, -19)
#define WEEK_OFFSET_Y PBL_IF_ROUND_ELSE(-2, -2)
#define WEATHER_OFFSET_Y PBL_IF_ROUND_ELSE(-3, -3)
#define BLUETOOTH_OFFSET_X PBL_IF_ROUND_ELSE(40, 32)
#define TEXT_X_DISTANCE PBL_IF_ROUND_ELSE(8, 4)
#define BATTERY_X TEXT_X_DISTANCE
#define BATTERY_Y PBL_IF_ROUND_ELSE(17, 17)
#define ARC_WIDTH_SUN 12
#define ARC_WIDTH_ACTIVITY 6

#define ANGLE_POINT(cx,cy,angle,radius) {.x=(int16_t)(sin_lookup((angle)/360.f*TRIG_MAX_ANGLE)*(radius)/TRIG_MAX_RATIO)+cx,.y=(int16_t)(-cos_lookup((angle)/360.f*TRIG_MAX_ANGLE)*(radius)/TRIG_MAX_RATIO)+cy}

void ui_bluetooth_set_available(bool is_available);
void ui_load(Window *window);
void ui_unload(void);
void ui_hide();
void ui_show();
void ui_update_colors();
void ui_update_time(void);
void ui_update_date(void);
void ui_update_time_and_activity();
void ui_update_weather(void);
void ui_battery_charge_start(void);
void ui_battery_charge_stop(void);
void ui_set_temperature(int temperature);
void ui_set_weather_icon(uint32_t resource_id);