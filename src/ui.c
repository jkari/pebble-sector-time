#include <pebble.h>
#include "config.h"
#include "weather.h"
#include "ui.h"
#include "health.h"
#include "gbitmap_color_palette_manipulator.h"

static TextLayer *s_layer_temperature;
static TextLayer *s_layer_day_of_month;
static TextLayer *s_layer_weekday;
static Layer *s_layer_hands;
static Layer *s_layer_bg;
static Layer *s_layer_activity;
static Layer *s_layer_battery;
static GBitmap *s_bitmap_weather = 0;
static GBitmap *s_bitmap_bluetooth = 0;
static BitmapLayer *s_layer_bluetooth;
static BitmapLayer *s_layer_weather;
static GFont s_font_big;
static GFont s_font_small;
static bool is_battery_animation_active = false;
static int battery_animation_percent = 0;

static int get_hour_angle() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  return TRIG_MAX_ANGLE * ((tick_time->tm_hour % 12) / 12.f + tick_time->tm_min / (12.f * 60));
}

static int get_hour_angle_next_marker() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  int minutes = (tick_time->tm_hour % 12) * 60 + tick_time->tm_min + 12;
  minutes = minutes - (minutes % 12);
  return TRIG_MAX_ANGLE * minutes / 720.0f;
}

static void _ui_set_temperature() {
  static char temperature_buffer[8];
  
  int temperature = weather_get_temperature();
  
  if (temperature >= 0) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "+%d", temperature);
  } else {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", temperature);
  }
  
  text_layer_set_text(s_layer_temperature, temperature_buffer);
}

static void _ui_reload_bitmap(GBitmap **image, uint32_t resource_id, GColor color) {
  if (*image == 0) {
    gbitmap_destroy(*image);
  }
  
  *image = gbitmap_create_with_resource(resource_id);
  
  replace_gbitmap_color(GColorWhite, color, *image, NULL);
}

static void _ui_set_weather_icon() {
  int32_t resource_id = weather_get_resource_id(weather_get_condition());
  
  _ui_reload_bitmap(&s_bitmap_weather, resource_id, config_get_color_front());
  bitmap_layer_set_bitmap(s_layer_weather, s_bitmap_weather);
  
  layer_set_hidden((Layer *)s_layer_weather, false);
}

static void _generate_bitmaps() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Generating bitmaps");
  _ui_set_weather_icon();
}

static void _charge_animation_callback(void *data) {
  battery_animation_percent = (battery_animation_percent + 2) % 100;
  
  if (is_battery_animation_active) {
    app_timer_register(30, _charge_animation_callback, NULL);
  }
  
  layer_mark_dirty(s_layer_battery);
}

static void _draw_sun_cycle(GContext *ctx, GPoint offset) {
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  
  float hour_sunrise = weather_get_sunrise_hour() + weather_get_sunrise_minute() / 60.f;
  float hour_sunset = weather_get_sunset_hour() + weather_get_sunset_minute() / 60.f;
  bool is_min_12_light_hours = (hour_sunrise > hour_sunset && hour_sunset + 24 - hour_sunrise >= 12)
    || hour_sunset - hour_sunrise >= 12;
  
  hour_sunrise = (hour_sunrise >= 13) ? hour_sunrise - 12 : hour_sunrise;
  hour_sunset = (hour_sunset >= 13) ? hour_sunset - 12 : hour_sunset;
  
  float angle_start = 360.f * hour_sunrise / 12.f;
  float angle_end = 360.f * hour_sunset / 12.f;
  
  if (is_min_12_light_hours) {
    float temp = angle_start;
    angle_start = angle_end;
    angle_end = temp;
  }
  
  if (angle_end < angle_start) {
    angle_end += 360.f;
  }
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Angle %d - %d", (int)angle_start, (int)angle_end);
  graphics_context_set_fill_color(ctx, config_get_color_sector_light());
  graphics_fill_radial(
    ctx,
    GRect(
      offset.x - SUN_ARC_RADIUS,
      offset.y - SUN_ARC_RADIUS,
      2 * SUN_ARC_RADIUS, 2*SUN_ARC_RADIUS
    ),
    GOvalScaleModeFitCircle,
    ARC_WIDTH,
    DEG_TO_TRIGANGLE(angle_start),
    DEG_TO_TRIGANGLE(angle_end)
  );
}

static void _draw_activity_cycle(GContext *ctx, GPoint offset) {
  float activities[60];
  health_get_activity(activities);
  
  int start_angle = TRIGANGLE_TO_DEG(get_hour_angle_next_marker());
  APP_LOG(APP_LOG_LEVEL_INFO, "Start angle %d", start_angle);
  
  int drawn_to = 0;
  float activity = activities[0];
  
  for (int i = 0; i < 60; i++) {
    float activity = activities[i];
    int width = ACTIVITY_ARC_RADIUS - (1.0f - activity) * ARC_WIDTH;
    
    if (width > 0) {  
      graphics_context_set_fill_color(ctx, GColorPastelYellow);
      graphics_fill_radial(
        ctx,
        GRect(
          offset.x - width,
          offset.y - width,
          2 * width,
          2 * width
        ),
        GOvalScaleModeFitCircle,
        (int)(activity * (ARC_WIDTH - 2)),
        DEG_TO_TRIGANGLE(start_angle + i * 6),
        DEG_TO_TRIGANGLE(start_angle + (i + 1) * 6)
      );
    }
  }
}

static void _draw_bg(GContext *ctx, GPoint offset, GRect area) {
  graphics_context_set_fill_color(ctx, config_get_color_bg());
  graphics_fill_rect(ctx, area, 0, GCornerNone);
  
  _draw_sun_cycle(ctx, offset);
  
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  
  for (int i = 0; i < 60; i++) {
    int length = i % 5 == 0 ? 6 : 3;
    int angle = TRIG_MAX_ANGLE * (i / 60.0f);
    GPoint from = {
      .x = (int16_t)(sin_lookup(angle) * (int32_t)MARKER_ARC_RADIUS / TRIG_MAX_RATIO) + offset.x,
      .y = (int16_t)(-cos_lookup(angle) * (int32_t)MARKER_ARC_RADIUS / TRIG_MAX_RATIO) + offset.y,
    };
    GPoint to = {
      .x = (int16_t)(sin_lookup(angle) * (int32_t)(MARKER_ARC_RADIUS - length) / TRIG_MAX_RATIO) + offset.x,
      .y = (int16_t)(-cos_lookup(angle) * (int32_t)(MARKER_ARC_RADIUS - length) / TRIG_MAX_RATIO) + offset.y,
    };
    graphics_context_set_stroke_width(ctx, i % 5 == 0 ? 3 : 1);
    graphics_draw_line(ctx, from, to);
  }
  
  layer_mark_dirty(s_layer_activity);
  layer_mark_dirty(s_layer_hands);
}

static void _draw_hands(GContext *ctx, GPoint offset) {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  int32_t minute_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
  int32_t hour_angle = get_hour_angle();
  
  GPoint minute_from = {
    .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)(MARKER_ARC_RADIUS - 10) / TRIG_MAX_RATIO) + offset.x,
    .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)(MARKER_ARC_RADIUS - 10) / TRIG_MAX_RATIO) + offset.y,
  };
  GPoint minute_to = {
    .x = (int16_t)(sin_lookup(minute_angle) * (int32_t)(MARKER_ARC_RADIUS - 22) / TRIG_MAX_RATIO) + offset.x,
    .y = (int16_t)(-cos_lookup(minute_angle) * (int32_t)(MARKER_ARC_RADIUS - 22) / TRIG_MAX_RATIO) + offset.y,
  };
  GPoint hour_from = {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(MARKER_ARC_RADIUS - 16) / TRIG_MAX_RATIO) + offset.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(MARKER_ARC_RADIUS - 16) / TRIG_MAX_RATIO) + offset.y,
  };
  GPoint hour_to = {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(MARKER_ARC_RADIUS - 22) / TRIG_MAX_RATIO) + offset.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(MARKER_ARC_RADIUS - 22) / TRIG_MAX_RATIO) + offset.y,
  };
  
  graphics_context_set_stroke_width(ctx, 5);
  graphics_context_set_stroke_color(ctx, GColorMagenta);
  graphics_draw_line(ctx, minute_from, minute_to);
  graphics_context_set_stroke_color(ctx, GColorCyan);
  graphics_draw_line(ctx, hour_from, hour_to);
}

static void _layer_bg_update_callback(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update bg");
  GRect bounds = layer_get_bounds(layer);
  _draw_bg(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), bounds);
}

static void _layer_hands_update_callback(Layer *layer, GContext *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update hands");
  GRect bounds = layer_get_bounds(layer);
  _draw_hands(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2));
}

static void _layer_activity_update_callback(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  _draw_activity_cycle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2));
}

static void _layer_battery_update_callback(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  BatteryChargeState charge_state = battery_state_service_peek();
  int current_charge = is_battery_animation_active ? battery_animation_percent : charge_state.charge_percent;

  graphics_context_set_stroke_color(ctx, config_get_color_front());
  graphics_context_set_stroke_width(ctx, BATTERY_STROKE + 2);
  graphics_draw_line(ctx, GPoint(BATTERY_STROKE, BATTERY_STROKE / 2 + 1), GPoint(BATTERY_STROKE + BATTERY_WIDTH, BATTERY_STROKE / 2 + 1));
  
  graphics_context_set_stroke_color(ctx, config_get_color_bg());
  graphics_context_set_stroke_width(ctx, BATTERY_STROKE);
  graphics_draw_line(ctx, GPoint(BATTERY_STROKE, BATTERY_STROKE / 2 + 1), GPoint(BATTERY_STROKE + BATTERY_WIDTH, BATTERY_STROKE / 2 + 1));
  
  GColor batteryColor = BATTERY_COLOR_MEDIUM;
  if (charge_state.charge_percent <= BATTERY_LOW_MAX) {
    batteryColor = BATTERY_COLOR_LOW;
  } else if (charge_state.charge_percent >= BATTERY_HIGH_MIN) {
    batteryColor = BATTERY_COLOR_HIGH;
  }
  graphics_context_set_stroke_color(ctx, batteryColor);
  graphics_draw_line(ctx, GPoint(BATTERY_STROKE, BATTERY_STROKE / 2 + 1), GPoint(BATTERY_STROKE + (current_charge / 100.0) * BATTERY_WIDTH, BATTERY_STROKE / 2 + 1));
}

void ui_update_date(void) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_day_of_month_buffer[6];
  strftime(s_day_of_month_buffer, sizeof(s_day_of_month_buffer), "%d", tick_time);
    
  static char s_weekday_buffer[4];
  strftime(s_weekday_buffer, sizeof(s_weekday_buffer), "%a", tick_time);
  
  text_layer_set_text(s_layer_day_of_month, s_day_of_month_buffer);
  text_layer_set_text(s_layer_weekday, s_weekday_buffer);
}

void ui_load(Window *window) {
                              
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);

  s_font_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_24));
  s_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_16));
  
  s_bitmap_bluetooth = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);
  
  s_layer_day_of_month = text_layer_create(GRect(center.x + TEXT_X_DISTANCE, center.y + DATE_OFFSET_Y, 40, 40));
  text_layer_set_background_color(s_layer_day_of_month, GColorClear);
  text_layer_set_font(s_layer_day_of_month, s_font_small);
  text_layer_set_text_alignment(s_layer_day_of_month, GTextAlignmentLeft);
  
  s_layer_weekday = text_layer_create(GRect(center.x + TEXT_X_DISTANCE, center.y + WEEK_OFFSET_Y, 40, 40));
  text_layer_set_background_color(s_layer_weekday, GColorClear);
  text_layer_set_font(s_layer_weekday, s_font_small);
  text_layer_set_text_alignment(s_layer_weekday, GTextAlignmentLeft);

  s_layer_temperature = text_layer_create(GRect(center.x - TEXT_X_DISTANCE - 40, center.y + DATE_OFFSET_Y, 40, 40));
  text_layer_set_background_color(s_layer_temperature, GColorClear);
  text_layer_set_text_alignment(s_layer_temperature, GTextAlignmentRight);
  text_layer_set_font(s_layer_temperature, s_font_small);
  
  s_layer_weather = bitmap_layer_create(GRect(center.x - TEXT_X_DISTANCE - 27, center.y + WEATHER_OFFSET_Y, 32, 32));
  bitmap_layer_set_compositing_mode(s_layer_weather, GCompOpSet);
  bitmap_layer_set_bitmap(s_layer_weather, s_bitmap_weather);
  layer_set_hidden((Layer *)s_layer_weather, true);
  
  s_layer_bluetooth = bitmap_layer_create(GRect(center.x + BLUETOOTH_OFFSET_X, center.y - 10, 20, 20));
  bitmap_layer_set_compositing_mode(s_layer_bluetooth, GCompOpSet);
  bitmap_layer_set_bitmap(s_layer_bluetooth, s_bitmap_bluetooth);
  
  s_layer_bg = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_layer_bg, _layer_bg_update_callback);

  s_layer_hands = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_layer_hands, _layer_hands_update_callback);
  
  s_layer_battery = layer_create(GRect(center.x + BATTERY_X, center.y + BATTERY_Y, BATTERY_WIDTH + BATTERY_STROKE * 2, 20));
  layer_set_update_proc(s_layer_battery, _layer_battery_update_callback);
  
  s_layer_activity = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_layer_activity, _layer_activity_update_callback);
  
  layer_add_child(window_layer, s_layer_bg);
  
  layer_add_child(s_layer_bg, text_layer_get_layer(s_layer_day_of_month));
  layer_add_child(s_layer_bg, text_layer_get_layer(s_layer_weekday));
  layer_add_child(s_layer_bg, text_layer_get_layer(s_layer_temperature));
  
  layer_add_child(s_layer_bg, bitmap_layer_get_layer(s_layer_weather));
  layer_add_child(s_layer_bg, bitmap_layer_get_layer(s_layer_bluetooth));
  
  layer_add_child(s_layer_bg, s_layer_battery);
  layer_add_child(s_layer_bg, s_layer_hands);
  layer_add_child(s_layer_bg, s_layer_activity);
}

void ui_unload(void) {
  text_layer_destroy(s_layer_day_of_month);
  text_layer_destroy(s_layer_weekday);
  text_layer_destroy(s_layer_temperature);
  
  bitmap_layer_destroy(s_layer_weather);
  bitmap_layer_destroy(s_layer_bluetooth);
  
  layer_destroy(s_layer_battery);
  layer_destroy(s_layer_hands);
  layer_destroy(s_layer_bg);
  
  fonts_unload_custom_font(s_font_big);
  fonts_unload_custom_font(s_font_small);
  
  gbitmap_destroy(s_bitmap_weather);
  gbitmap_destroy(s_bitmap_bluetooth);
}

void ui_update_time(void) {
  layer_mark_dirty(s_layer_hands);
}

void ui_update_activity() {
  layer_mark_dirty(s_layer_activity);
}

void ui_update_all(void) {
  
  text_layer_set_text_color(s_layer_temperature, config_get_color_front());
  text_layer_set_text_color(s_layer_weekday, config_get_color_front());
  text_layer_set_text_color(s_layer_day_of_month, config_get_color_front());
  
  ui_update_weather();
  ui_update_date();
  
  layer_mark_dirty(s_layer_bg);
}

void ui_bluetooth_set_available(bool is_available) {
  layer_set_hidden((Layer *)s_layer_bluetooth, is_available);
}

void ui_battery_charge_start(void) {
  is_battery_animation_active = true;
  _charge_animation_callback(NULL);
}

void ui_battery_charge_stop(void) {
  is_battery_animation_active = false;
  layer_mark_dirty(s_layer_battery);
}

void ui_update_weather() {
  _generate_bitmaps();

  _ui_set_temperature();
  _ui_set_weather_icon();
}