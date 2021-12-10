<script>
  import Router from 'svelte-spa-router'
  import { onMount } from 'svelte'

  import NavBar from './components/NavBar.svelte'
  import SideBar from './components/SideBar.svelte'
  import NowPlaying from './components/NowPlaying.svelte'

  import routes from './routes'
  import { getCommand } from './utils.js'
  import { totalTime, queue, outputs, current, playing, status } from './stores.js'
  
  let ws
  let connected = false

  onMount(() => {
    ws = new WebSocket("ws://localhost:8000/ws");

    // Connection opened
    ws.addEventListener('open', function (event) {
      ws.send(JSON.stringify({
        cmd_id: getCommand('queue') || 0
      }))
      connected = true
    })
    ws.addEventListener('message', function (event) {
      let data = JSON.parse(event.data)
      switch (data.type) {
        case 'queue':
          queue.set(data.queue)
          totalTime.set(data.totalTime)
          break
        case 'outputs':
          outputs.set(data.outputs)
          break
        case 'status':
          current.set(data.song)
          // 1 = stoped 2 = playing, 3 = paused
          playing.set(data.state === 2)
          status.set(data)
          break
      }
    })
    ws.addEventListener('close', function (event) {
      connected = false
    })
  })

  const runCommand = (cmd, args = {}) => {
    if (connected) {
      ws.send(JSON.stringify({
        cmd_id: getCommand(cmd) || 0,
        ...args
      }))
    }
  }
  const handleKeydown = (e) => {
    // if (e.key === 'Enter') {
    //   runCommand('play')
    // }
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

<main class="mx-1 overflow-hidden" >
  <div class="min-h-screen flex flex-row bg-gray-100 overflow-hidden">
    <SideBar />
    <div class="flex flex-col flex-grow overflow-hidden">
      <NavBar />
      <Router routes={routes} />
    </div>
  </div>
  <NowPlaying
    on:next="{ () => runCommand('next') }"
    on:toggle="{ () => runCommand('pause') }"
    on:prev="{ () => runCommand('prev') }"
    on:seek="{ e => runCommand('seek', e.detail) }" />
</main>

