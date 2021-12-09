<script>
  export let type = 'queue'

  import { onMount } from 'svelte'
  import { totalTime, queue } from './../stores.js'

  import Song from './Song.svelte'

  let songs = []
  let time = ''

  onMount(() => {
    switch (type) {
      case 'queue':
        queue.subscribe(q => {
          songs = q
        })
        totalTime.subscribe(t => {
          let h = Math.floor(t / 3600);
          let m = Math.floor(t % 3600 / 60);
          let s = Math.floor(t % 3600 % 60);

          time = `${h} hours ${m} minutes ${s} seconds`
        })
        break
      case 'browse':
        songs = []
        time = 0
        break
      case 'playlist':
        songs = []
        time = 0
        break
    }
  })
</script>

<div class="w-full max-h-screen mt-5 mb-40 max-w-6xl mx-auto bg-white shadow-lg rounded-3xl overflow-y-auto relative">
  <header class="flex sticky top-0 px-5 py-5 z-10 border-b border-gray-100 bg-gray-50">
    <h2 class="font-semibold text-gray-800">Songs</h2>
    <span class="ml-auto text-sm text-gray-400">
      Total Time { time }
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
            <Song {song} />
          {/each}
        </tbody>
      </table>
    </div>
  </div>
</div>
