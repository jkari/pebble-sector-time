var MESSAGE_TYPE_CONFIG = 3;

function parseColorInt(value) {
  return parseInt(value.substr(value.length - 6), 16);
}

Pebble.addEventListener('showConfiguration', function(e) {
  Pebble.openURL('http://95.85.12.164/mytime/');
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));
  console.log('Config window returned: ', JSON.stringify(config_data));

  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(
    {
      'KEY_MESSAGE_TYPE': MESSAGE_TYPE_CONFIG,
      'KEY_USE_CELCIUS': parseInt(config_data.use_celcius),
      'KEY_COLOR_BG': parseColorInt(config_data.color_bg),
      'KEY_COLOR_TEXT': parseColorInt(config_data.color_text),
      'KEY_COLOR_LIGHT': parseColorInt(config_data.color_light),
      'KEY_COLOR_DARK': parseColorInt(config_data.color_dark),
      'KEY_COLOR_MINUTE': parseColorInt(config_data.color_minute),
      'KEY_COLOR_HOUR': parseColorInt(config_data.color_hour),
      'KEY_COLOR_ACTIVITY': parseColorInt(config_data.color_activity),
      'KEY_ACTIVITY_SENSITIVITY': parseInt(config_data.activity_sensitivity)
    },
    function(){
      console.log('Sent config data to Pebble');  
    },
    function() {
      console.log('Failed to send config data!');
    }
  );
});