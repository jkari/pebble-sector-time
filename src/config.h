#pragma once

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_SUNRISE 2
#define KEY_SUNSET 3
#define KEY_USE_CELCIUS 4
#define KEY_COLOR_BG 5
#define KEY_COLOR_TEXT 6
#define KEY_MESSAGE_TYPE 7
#define KEY_COLOR_LIGHT 8
#define KEY_COLOR_DARK 9
#define KEY_COLOR_MINUTE 10
#define KEY_COLOR_HOUR 11
#define KEY_COLOR_ACTIVITY 12
#define KEY_ACTIVITY_SENSITIVITY 13

#define PERSIST_KEY_TEMPERATURE 1
#define PERSIST_KEY_CONDITION 2
#define PERSIST_KEY_SUNRISE 3
#define PERSIST_KEY_SUNSET 4
#define PERSIST_KEY_USE_CELCIUS 5
#define PERSIST_KEY_COLOR_BG 6
#define PERSIST_KEY_COLOR_TEXT 7
#define PERSIST_KEY_ACTIVITY 8
#define PERSIST_KEY_LAST_ACTIVITY_UPDATE 9
#define PERSIST_KEY_LAST_ACTIVITY_VALUE 10
#define PERSIST_KEY_COLOR_LIGHT 11
#define PERSIST_KEY_COLOR_DARK 12
#define PERSIST_KEY_COLOR_MINUTE 13
#define PERSIST_KEY_COLOR_HOUR 14
#define PERSIST_KEY_COLOR_ACTIVITY 15
#define PERSIST_KEY_ACTIVITY_SENSITIVITY 16

void config_received_callback(DictionaryIterator* iterator);
GColor config_get_color_bg();
GColor config_get_color_front();
GColor config_get_color_light();
GColor config_get_color_dark();
GColor config_get_color_hour();
GColor config_get_color_minute();
GColor config_get_color_activity();
GColor config_get_color_text();
bool config_get_use_celcius();
int config_get_activity_sensitivity();