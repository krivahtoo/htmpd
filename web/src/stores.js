import { writable } from 'svelte/store'

export const queue = writable({
  limit: 20,
  offset: 0,
  total: 0,
  totalTime: 0,
  queue: []
})

export const browse = writable({
  limit: 20,
  offset: 0,
  total: 0,
  dirs: [],
  dirsCount: 0,
  files: [],
  filesCount: 0,
  playlists: [],
  playlistsCount: 0,
})

export const current = writable({
  title: "--",
  artist:"-",
  album:"-",
  duration:100,
  id:6,
  pos:1
})

export const playing = writable(true)

export const totalTime = writable(0)

export const outputs = writable([])

export const status = writable({
  bitrate: 256,
  consume: 0,
  crossfade: 5,
  current: 1,
  elapsedTime: 0,
  playlistLength: 0,
  playlistSong: 0,
  random: 0,
  repeat: 1,
  single: 0,
  song: {},
  state: 2,
  totalTime: 100,
  volume: 96
})

export const stats = writable({
  albumsCount: 0,
  artistsCount: 0,
  dbPlayTime: 0,
  dbUpdateTime: 0,
  playTime: 0,
  songsCount: 0,
  uptime: 0
})

export const messages = writable([
  {
    time: Date.now(),
    duration: 2000,
    type:"info",
    text:"Welcome to htmpd the web interface of the MPD server"
  }
])

