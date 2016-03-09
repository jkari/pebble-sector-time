#include <pebble.h>
#include "ui.h"
#include "weather.h"
#include "config.h"

static void _update_settings(int use_celcius, int color_bg, int color_text, int color_light,
                             int color_dark, int color_minute, int color_hour, int color_activity,
                             int activity_sensitivity)
{
  persist_write_int(PERSIST_KEY_USE_CELCIUS, use_celcius);
  persist_write_int(PERSIST_KEY_COLOR_BG, color_bg);
  persist_write_int(PERSIST_KEY_COLOR_TEXT, color_text);
  persist_write_int(PERSIST_KEY_COLOR_LIGHT, color_light);
  persist_write_int(PERSIST_KEY_COLOR_DARK, color_dark);
  persist_write_int(PERSIST_KEY_COLOR_MINUTE, color_minute);
  persist_write_int(PERSIST_KEY_COLOR_HOUR, color_hour);
  persist_write_int(PERSIST_KEY_COLOR_ACTIVITY, color_activity);
  persist_write_int(PERSIST_KEY_ACTIVITY_SENSITIVITY, activity_sensitivity);
  
  ui_update_colors();
  ui_show();
  weather_update();
}

void config_received_callback(DictionaryIterator* iterator) {
  Tuple *use_celcius = dict_find(iterator, KEY_USE_CELCIUS);
  Tuple *color_bg = dict_find(iterator, KEY_COLOR_BG);
  Tuple *color_text = dict_find(iterator, KEY_COLOR_TEXT);
  Tuple *color_light = dict_find(iterator, KEY_COLOR_LIGHT);
  Tuple *color_dark = dict_find(iterator, KEY_COLOR_DARK);
  Tuple *color_minute = dict_find(iterator, KEY_COLOR_MINUTE);
  Tuple *color_hour = dict_find(iterator, KEY_COLOR_HOUR);
  Tuple *color_activity = dict_find(iterator, KEY_COLOR_ACTIVITY);
  Tuple *activity_sensitivity = dict_find(iterator, KEY_ACTIVITY_SENSITIVITY);
  
  if (use_celcius && color_bg) {
    _update_settings(
      use_celcius->value->int32,
      color_bg->value->int32,
      color_text->value->int32,
      color_light->value->int32,
      color_dark->value->int32,
      color_minute->value->int32,
      color_hour->value->int32,
      color_activity->value->int32,
      activity_sensitivity->value->int32
    );
  }
}

GColor config_get_color_bg() {
  return persist_exists(PERSIST_KEY_COLOR_BG) ? GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_BG)) : GColorWhite;
}

GColor config_get_color_text() {
  return persist_exists(PERSIST_KEY_COLOR_TEXT) ? GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_TEXT)) : GColorBlack;
}

GColor config_get_color_light() {
  return persist_exists(PERSIST_KEY_COLOR_LIGHT) ? GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_LIGHT)) : GColorLightGray;
}

GColor config_get_color_dark() {
  return persist_exists(PERSIST_KEY_COLOR_DARK) ? GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_DARK)) : GColorDarkGray;
}

GColor config_get_color_hour() {
  return persist_exists(PERSIST_KEY_COLOR_HOUR) ? GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_HOUR)) : GColorBlueMoon;
}

GColor config_get_color_minute() {
  return persist_exists(PERSIST_KEY_COLOR_MINUTE) ? GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_MINUTE)) : GColorMagenta;
}

GColor config_get_color_activity() {
  return persist_exists(PERSIST_KEY_COLOR_ACTIVITY) ? GColorFromHEX(persist_read_int(PERSIST_KEY_COLOR_ACTIVITY)) : GColorBlack;
}

bool config_get_use_celcius() {
  return persist_exists(PERSIST_KEY_USE_CELCIUS) ? persist_read_int(PERSIST_KEY_USE_CELCIUS) > 0 : true;
}

int config_get_activity_sensitivity() {
  return persist_exists(PERSIST_KEY_ACTIVITY_SENSITIVITY) ? persist_read_int(PERSIST_KEY_ACTIVITY_SENSITIVITY) : 800;
}
