<script>
  export let type = 'queue'
  export let song = {
    title: "--",
    artist:"-",
    album:"-",
    duration:100,
    id:6,
    pos:1,
    year: '2000',
    uri: ''
  }
  import { secondsToString } from './../utils.js'
  import { status } from './../stores.js'

  let albumArt = '/logo.svg'
  let currentStatus = {}

  status.subscribe(status => {
    currentStatus = status
  })

  function play() {
    let event
    switch (type) {
      case 'queue':
        event = new CustomEvent('play', { detail: song.id })
        document.dispatchEvent(event)
        break
      case 'playlist':
        event = new CustomEvent('play', { detail: song.id })
        document.dispatchEvent(event)
        break
      case 'browse':
        event = new CustomEvent('add_track', { detail: song.uri })
        document.dispatchEvent(event)
        break
    }
  }
</script>

<tr 
  class="cursor-pointer hover:bg-gray-200 { (song.id == currentStatus.current) ? 'text-green-500' : '' }"
  title="Play { song.title }"
  on:click="{play}">
  <td class="p-1">{ song.id }</td>
  <td class="p-2 whitespace-nowrap w-1/3 overflow-hidden">
    <div class="flex flex-nowrap items-center">
      <div class="w-10 h-10 flex-shrink-0 mr-2 sm:mr-3">
        <img
          class="rounded-full { (song.id == currentStatus.current) ? 'bg-blue-200' : '' }"
          src={ song.uri || albumArt }
          onerror="this.src='/logo.svg'"
          width="40" height="40" alt="{song.title}">
      </div>
      <div class="flex-grow-0 mr-2 sm:mr-3 overflow-hidden">
        <h2 class="text-sm leading-5 font-medium { (song.id == currentStatus.current) ? 'text-green-500' : 'text-gray-900 '}">{ song.title }</h2>
        <h3
          class="text-sm leading-5 font-medium { (song.id == currentStatus.current) ? 'text-green-300' : 'text-gray-500' }">
          { (song.artist.length > 30) ? song.artist.substr(0, 29) + '...' : song.artist } | { song.year || '-' }
        </h3>
      </div>
    </div>
  </td>
  <td class="p-2 whitespace-nowrap">
    <div class="text-left font-medium">{ song.album }</div>
  </td>
  <td class="p-2 whitespace-nowrap">
    <div class="text-lg text-center">{ secondsToString(song.duration) }</div>
  </td>
</tr>

