<script>
  import { onMount } from 'svelte'
  import { browse, stats } from '../../stores.js'

  import File from './File.svelte'
  import Folder from './Folder.svelte'
  import Playlist from './Playlist.svelte'
  import Pagination from '../Pagination.svelte'

  let list = []
  let time = ''
  let pagination = {
    offset: 0,
    limit: 20,
    total: 0
  }
  let currentStats = {
    dbPlayTime: 0
  }

  onMount(() => {
    browse.subscribe(b => {
      list = b.browse
      pagination.offset = b.offset
      pagination.limit = b.limit
      pagination.total = b.total
    })
    stats.subscribe(v => currentStats = v)
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

<div class="w-full max-h-full mt-5 mb-5 max-w-6xl mx-auto bg-white shadow-sm rounded-3xl overflow-auto relative">
  <header class="flex sticky top-0 px-5 py-5 z-10 border-b border-gray-100 bg-gray-50">
    <h2 class="font-semibold text-gray-800">Browse</h2>
    <span class="ml-auto text-sm text-gray-400">
      Total DB Time { formatTime(currentStats.dbPlayTime) }
    </span>
  </header>
  <table class="border-collapse table-auto w-full mb-5 whitespace-no-wrap bg-white table-striped relative overflow-auto">
    <thead class="text-xs sticky top-1 font-semibold uppercase text-gray-400 bg-gray-50">
      <tr>
        <th class="p-1"></th>
        <th class="p-2 whitespace-nowrap">
          <div class="font-semibold text-left">Title</div>
        </th>
        <th class="p-2 whitespace-nowrap">
          <div class="font-semibold text-center">Duration/ Last Modified</div>
        </th>
      </tr>
    </thead>
    <tbody class="text-sm divide-y divide-gray-100">
      {#each list as value}
        {#if value.type == 'file'}
          <File {...value} />
        {:else if value.type == 'directory'}
          <Folder {...value} />
        {:else}
          <Playlist {...value} />
        {/if}
      {/each}
    </tbody>
  </table>
  {#if pagination.total > 0}
    <Pagination {pagination} path="#/browse" />
  {/if}
</div>


