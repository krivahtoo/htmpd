<script>
  import Router from 'svelte-spa-router'
  import { onMount } from 'svelte'

  import Toast from './components/Toast.svelte'
  import NavBar from './components/NavBar.svelte'
  import SideBar from './components/SideBar.svelte'
  import Settings from './components/Settings.svelte'
  import NowPlaying from './components/NowPlaying.svelte'

  import routes from './routes'
  import { getCommand } from './utils.js'
  import {
    totalTime,
    queue,
    outputs,
    current,
    playing,
    status,
    browse,
    messages,
    stats
  } from './stores.js'

  let ws
  let currentSong = {}
  let connected = false
  let showSettings = false

  onMount(() => {
    ws = new WebSocket(`ws://${location.host}/ws`);

    // Connection opened
    ws.addEventListener('open', function () {
      connected = true
      // commands to initialize
      ;['queue', 'outputs', 'browse']
        .map(v => runCommand(v))

      messages.update(msg => msg.concat({
        time: Date.now(),
        duration: 1000,
        type: 'info',
        text: 'Connected to server'
      }))
    })
    ws.addEventListener('message', function (event) {
      let data = JSON.parse(event.data)
      switch (data.type) {
        case 'queue':
          queue.set(data)
          totalTime.set(data.totalTime)
          break
        case 'browse':
          browse.set(data)
          break
        case 'outputs':
          outputs.set(data.outputs)
          break
        case 'status':
          currentSong = data.song
          current.set(data.song)
          // 0 = unknown 1 = stoped 2 = playing, 3 = paused
          playing.set(data.state === 2)
          status.set(data)
          break
        case 'stats':
          stats.set(data)
          break
        case 'error':
          messages.update(m => m.concat({
            time: Date.now(),
            duration: 3000,
            type: 'error',
            text: `Error: ${data.message}`
          }))
          break
        case 'connected':
          messages.update(m => m.concat({
            time: Date.now(),
            duration: 3000,
            type: 'info',
            text: 'Connected to MPD server'
          }))
          break
        case 'disconnected':
          playing.set(false)
          messages.update(m => m.concat({
            time: Date.now(),
            duration: 3000,
            type: 'error',
            text: 'Disconnected from MPD server'
          }))
          break
        case 'update':
          if (data.success) {
            messages.update(m => m.concat({
              time: Date.now(),
              duration: 3000,
              type: 'info',
              text: 'Update successful'
            }))
          } else {
            messages.update(m => m.concat({
              time: Date.now(),
              duration: 3000,
              type: 'error',
              text: `Update failed: ${data.message}`
            }))
          }
      }
    })
    ws.addEventListener('close', function () {
      connected = false
      messages.update(m => m.concat({
        time: Date.now(),
        duration: 3000,
        type: 'error',
        text: 'Disconnected from server'
      }))
    })
    ws.addEventListener('error', function (event) {
      messages.update(m => m.concat({
        time: Date.now(),
        duration: 3000,
        type: 'error',
        text: `Error: ${event.message}`
      }))
    })
    document.addEventListener('play', function (e) {
      if (connected) {
        runCommand('play_track', { id: e.detail })
      }
    })
    document.addEventListener('add_track', function (e) {
      if (connected) {
        runCommand('add_track', { path: e.detail })
      }
    })
    document.addEventListener('add_playlist', function (e) {
      if (connected) {
        runCommand('add_playlist', { name: e.detail })
      }
    })
    document.addEventListener('browse', function (e) {
      if (connected) {
        runCommand('browse', e.detail)
      }
    })
    document.addEventListener('queue', function (e) {
      if (connected) {
        runCommand('queue', e.detail)
      }
    })
  })

  const runCommand = (cmd, args = {}) => {
    if (connected) {
      ws.send(JSON.stringify({
        cmd_id: getCommand(cmd) || 0,
        ...args
      }))
      setTimeout(() => {
        if (cmd === 'prev' || cmd === 'next') {
          messages.update(m => m.concat({
            time: Date.now(),
            duration: 1000,
            type: 'info',
            text: `Playing: ${currentSong.title}`
          }))
        }
      }, 1000)
    }
  }
  const handleKeydown = (e) => {
    if (e.key === 'Enter') {
      runCommand('play')
    }
    if (e.key === ' ') {
      runCommand('pause')
    }
    if (e.key === 'ArrowLeft') {
      runCommand('prev')
    }
    if (e.key === 'ArrowRight') {
      runCommand('next')
    }
  }
</script>

<svelte:window on:keydown={handleKeydown}/>

<Toast/>

<main class="relative md:mx-1 overflow-auto" >
  <div class="min-h-screen flex flex-row bg-gray-100 overflow-auto">
    <SideBar />
    <div class="flex flex-col flex-grow overflow-auto">
      <NavBar on:settings={ () => showSettings = true } />
      <Router routes={routes} />
    </div>
  </div>
  <NowPlaying
    on:next={ () => runCommand('next') }
    on:toggle={ () => runCommand('pause') }
    on:prev={ () => runCommand('prev') }
    on:seek={ e => runCommand('seek', e.detail) } />
</main>

{#if showSettings}
  <Settings
    on:close={ () => showSettings = false }
    on:toggle={ e => runCommand(`toggle_${e.detail[0]}`, e.detail[1]) }/>
{/if}

