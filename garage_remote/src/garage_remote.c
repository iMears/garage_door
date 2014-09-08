#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *status_layer;

static AppSync sync;
static uint8_t sync_buffer[64];
static bool was_closed = false;

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  if(key == 0) /* status message */
  {
    const char *status_message = new_tuple->value->cstring;
    if(!strcmp(status_message, ""))
      status_message = "Press Select to activate.";

    APP_LOG(APP_LOG_LEVEL_DEBUG, "New status message: %s", status_message);
    text_layer_set_text(text_layer, status_message);
  }
  if(key == 1)//garagedoor status
  {
    const char *garage_status = new_tuple->value->cstring;
    bool now_closed = !strcmp(garage_status, "Closed");
    if(was_closed != now_closed) {
      if(now_closed)
        vibes_double_pulse();
      was_closed = now_closed;
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "New garage status: %s", garage_status);
    text_layer_set_text(status_layer, garage_status);
  }
}

static void send_cmd(void) {
  Tuplet value = TupletInteger(2, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "WTF? iter == null");
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Connecting...");
  send_cmd();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 48 }, .size = { bounds.size.w, 60 } });
  text_layer_set_text(text_layer, "");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  status_layer = text_layer_create((GRect) {.origin = { 0, bounds.size.h - 16 }, .size = { bounds.size.w, 16}  });
  text_layer_set_text_alignment(status_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(status_layer));

  Tuplet initial_values[] = {
    TupletCString(0, ""),
    TupletCString(1, ""),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
