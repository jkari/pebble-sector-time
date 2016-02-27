#pragma once

#define BATTERY_WIDTH 16
#define BATTERY_STROKE 3
#define BATTERY_HIGH_MIN 60
#define BATTERY_LOW_MAX 20
#define BATTERY_COLOR_HIGH GColorGreen
#define BATTERY_COLOR_MEDIUM GColorChromeYellow 
#define BATTERY_COLOR_LOW GColorRed
#define SUN_ARC_RADIUS PBL_IF_ROUND_ELSE(88, 67)
#define MARKER_ARC_RADIUS PBL_IF_ROUND_ELSE(70, 88)
#define ACTIVITY_ARC_RADIUS PBL_IF_ROUND_ELSE(88, 60)
#define ARC_WIDTH PBL_IF_ROUND_ELSE(15, 15)
#define MINUTE_HAND_LENGTH PBL_IF_ROUND_ELSE(60, 48)
#define HOUR_HAND_LENGTH PBL_IF_ROUND_ELSE(40, 32)
#define DATE_OFFSET_Y PBL_IF_ROUND_ELSE(-16, -10)
#define WEEK_OFFSET_Y PBL_IF_ROUND_ELSE(1, 4)
#define WEATHER_OFFSET_Y PBL_IF_ROUND_ELSE(-1, 1)
#define BLUETOOTH_OFFSET_X PBL_IF_ROUND_ELSE(40, 32)
#define TEXT_X_DISTANCE 8
#define BATTERY_X TEXT_X_DISTANCE
#define BATTERY_Y PBL_IF_ROUND_ELSE(20, 19)

#define ANGLE_POINT(cx,cy,angle,radius) {.x=(int16_t)(sin_lookup((angle)/360.f*TRIG_MAX_ANGLE)*(radius)/TRIG_MAX_RATIO)+cx,.y=(int16_t)(-cos_lookup((angle)/360.f*TRIG_MAX_ANGLE)*(radius)/TRIG_MAX_RATIO)+cy}

void ui_bluetooth_set_available(bool is_available);
void ui_load(Window *window);
void ui_unload(void);
void ui_update_all(void);
void ui_update_time(void);
void ui_update_date(void);
void ui_update_activity();
void ui_update_weather(void);
void ui_battery_charge_start(void);
void ui_battery_charge_stop(void);
void ui_set_temperature(int temperature);
void ui_set_weather_icon(uint32_t resource_id);