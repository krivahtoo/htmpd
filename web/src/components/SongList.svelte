<script>
  export let type = 'queue'

  import { onMount } from 'svelte'
  import { totalTime, queue, browse, stats } from './../stores.js'

  import Song from './Song.svelte'

  let songs = []
  let time = ''
  let currentStats = {}

  onMount(() => {
    switch (type) {
      case 'queue':
        queue.subscribe(q => songs = q)
        totalTime.subscribe(t => time = formatTime(t))
        break
      case 'browse':
        stats.subscribe(v => currentStats = v)
        browse.subscribe(b => {
          if (b && b.files) {
            songs = b.files
          }
        })
        time = 0
        break
      case 'playlist':
        songs = []
        time = 0
        break
    }
  })

  const formatTime = (t) => {
    t = parseInt(t)
    if (t <= 0) return t
    let h = Math.floor(t / 3600)
    let m = Math.floor(t % 3600 / 60)
    let s = Math.floor(t % 3600 % 60)

    return `${h} hours ${m} minutes ${s} seconds`
  }
</script>

<div class="w-full max-h-screen mt-5 mb-40 max-w-6xl mx-auto bg-white shadow-lg rounded-3xl overflow-y-auto relative">
  <header class="flex sticky top-0 px-5 py-5 z-10 border-b border-gray-100 bg-gray-50">
    <h2 class="font-semibold text-gray-800">Songs</h2>
    <span class="ml-auto text-sm text-gray-400">
    {#if type == 'browse'}
      Total DB Time { formatTime(currentStats.dbPlayTime) }
    {:else}
      Total Time { time }
    {/if}
    </span>
  </header>
  <div class="p-3 relative" style="height: 450px;">
    <div class="overflow-x-auto bg-white rounded-lg shadow overflow-y-auto relative">
      <table class="border-collapse table-auto w-full mb-11 whitespace-no-wrap bg-white table-striped relative">
        <thead class="text-xs sticky top-1 font-semibold uppercase text-gray-400 bg-gray-50">
          <tr>
            <th class="p-1">Id</th>
            <th class="p-2 whitespace-nowrap">
              <div class="font-semibold text-left">Title</div>
            </th>
            <th class="p-2 whitespace-nowrap">
              <div class="font-semibold text-left">Album</div>
            </th>
            <th class="p-2 whitespace-nowrap">
              <div class="font-semibold text-center">Duration</div>
            </th>
          </tr>
        </thead>
        <tbody class="text-sm divide-y divide-gray-100">
          {#each songs as song}
            <Song {song} {type} />
          {/each}
        </tbody>
      </table>
    </div>
  </div>
</div>

