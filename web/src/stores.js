import { writable } from 'svelte/store'

export const queue = writable([])

export const browse = writable({})

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

export const status = writable({})

export const messages = writable([
  {
    time: Date.now(),
    duration: 2000,
    type:"info",
    text:"Welcome to htmpd the web interface of the MPD server"
  }
])

