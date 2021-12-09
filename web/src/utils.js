const commands = [
  'queue',
  'browse',
  'mpdhost',
  'add_track',
  'add_play_track',
  'add_playlist',
  'play_track',
  'save_queue',
  'rm_track',
  'rm_range',
  'rm_all',
  'move_track',
  'search',
  'get_channels',
  'send_message',
  'volume',
  'pause',
  'play',
  'stop',
  'seek',
  'next',
  'prev',
  'mpdhost',
  'mpdpass',
  'update_db',
  'outputs',
  'toggle_output',
  'toggle_random',
  'toggle_consume',
  'toggle_single',
  'toggle_crossfade',
  'toggle_repeat',
  'seek_current',
  'get_commands',
  'authorize'
]

export const getCommand = (command) => {
  return commands.indexOf(command)
}

// convert a seconds value to a human readable string
export const secondsToString = (t) => {
  var h = Math.floor(t / 3600);
  var m = Math.floor(t % 3600 / 60);
  var s = Math.floor(t % 3600 % 60);

  if (m < 10) {
    m = '0' + m
  }
  if (s < 10) {
    s = '0' + s
  }
  return ((h > 0) ? h + ':' : '' ) + m + ':' + s
}

