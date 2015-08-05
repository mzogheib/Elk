/*
  WISH LIST

  Battery = 20% --> date text colour = yellow
  Battert = 10% --> date text colour = red

*/

#include <pebble.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *date_layer_bg;

BitmapLayer *image_layer;
GBitmap *image;
GSize image_size;

GFont custom_font;

// Returns the time
static char *write_time(struct tm tick_time) {
  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", &tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", &tick_time);
  }
   
  // Strip leading zero
  if(buffer[0]=='0') strcpy(buffer, buffer+1);
  
  return (char *)buffer;
}

// Returns the date suffix
static char *date_suffix(char date[]) {
  static char suffix[] = "th";
  
  if(strcmp(date, "1")==0 || strcmp(date, "21")==0 || strcmp(date, "31")==0) strcpy(suffix, "st");
  else if(strcmp(date, "2")==0 || strcmp(date, "22")==0) strcpy(suffix, "nd");
  else if(strcmp(date, "3")==0 || strcmp(date, "23")==0) strcpy(suffix, "rd");  
  // Else leave as is, "th"
  
  return (char *)suffix;  
}

// Returns the date
static char *write_date(struct tm tick_time) {
  // Create a long-lived buffer
  static char buffer[] = "WEDNESDAY 30TH";
  static char day[] = "WEDNESDAY ";
  static char date[] = "30";
  
  strftime(day, sizeof(day), "%A ", &tick_time);
  strftime(date, sizeof(date), "%d", &tick_time);
  // Strip leading zero
  if(date[0]=='0') strcpy(date, date+1);
  
  strcpy(buffer, day);
  strcat(buffer, date);
  strcat(buffer, date_suffix(date));
  
  return (char *)buffer;  
}

// Change the date text colour based on battery level
static void handle_battery(BatteryChargeState charge_state) {

  APP_LOG(APP_LOG_LEVEL_INFO, "Battery = %d", charge_state.charge_percent); 
  
  // If charging or plugged in: Green
  // Else if unplugged: White, yellow or red depending on battery level
  if(charge_state.is_charging || charge_state.is_plugged) {
    text_layer_set_text_color(date_layer, GColorGreen);
  } else {
    //charge_state = battery_state_service_peek();
    if(charge_state.charge_percent>20) {
      text_layer_set_text_color(date_layer, GColorWhite);
    } else if(charge_state.charge_percent>10) {
      text_layer_set_text_color(date_layer, GColorYellow);
    } else if(charge_state.charge_percent<=10) {
      text_layer_set_text_color(date_layer, GColorRed);
    } 
  }
  //static char s_battery_buffer[32];
  //snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d/100", charge_state.charge_percent);
  //text_layer_set_text(date_layer, s_battery_buffer);
}

// Run this function at every tick of the clock, i.e. second or minute
static void handle_tick(struct tm *tick_time, TimeUnits units){  
  // Write the current time and date
  text_layer_set_text(time_layer, write_time(*tick_time));
  text_layer_set_text(date_layer, write_date(*tick_time));
  //handle_battery(battery_state_service_peek());
}

// Initialize err'thang
static void handle_init(void) {
  my_window = window_create();
  Layer *window_layer = window_get_root_layer(my_window);
  
  // Subscribe to the battery
  battery_state_service_subscribe(handle_battery);
  
  // Init the image & layer. 
  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  image_size = gbitmap_get_bounds(image).size;  
  image_layer = bitmap_layer_create(GRect(0, 48, image_size.w, image_size.h));
  bitmap_layer_set_compositing_mode (image_layer, GCompOpSet);
  bitmap_layer_set_bitmap(image_layer, image);
  
  // Init the time & date layers
  int time_layer_x = 0;
  int time_layer_y = -2;
  int time_layer_w = 144;
  int time_layer_h = 45;
  int date_layer_x = 0;
  int date_layer_y = 140;
  int date_layer_w = 144;
  int date_layer_h = 35;
  // Position the time layer and it's background layer. Using two layers to tune the position of the text.
  time_layer = text_layer_create(GRect(time_layer_x, time_layer_y, time_layer_w, time_layer_h));
  date_layer_bg = text_layer_create(GRect(date_layer_x, date_layer_y, date_layer_w, date_layer_h));  
  date_layer = text_layer_create(GRect(date_layer_x, date_layer_y+1, date_layer_w, date_layer_h));
  
  // Init colours & alignment
  /* Define black and white mode here */
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_background_color(date_layer_bg, GColorBlack);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  window_set_background_color(my_window, GColorVividCerulean);
 
  // Init font
  custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_40));
  text_layer_set_font(time_layer, custom_font);  
  text_layer_set_text_color(time_layer, GColorBlack);
  custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_19));
  text_layer_set_font(date_layer, custom_font);  
  text_layer_set_text_color(date_layer, GColorWhite);
  
  // Finally, write the time on start up
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);  // Find a way to not use temp
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  text_layer_set_text(time_layer, write_time(*tick_time));
  text_layer_set_text(date_layer, write_date(*tick_time));
  

 
 
  // Add all layers to the window layer
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, text_layer_get_layer(date_layer_bg));
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  
  window_stack_push(my_window, true);

}

static void handle_deinit(void) {
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(date_layer_bg);
  bitmap_layer_destroy(image_layer);
  gbitmap_destroy(image);
  fonts_unload_custom_font(custom_font);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}