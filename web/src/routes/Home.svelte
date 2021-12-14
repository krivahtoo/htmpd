<script>
  import { onMount } from 'svelte'
  import SongList from '../components/SongList.svelte'

  onMount(() => {
    const event = new Event('queue')
    document.dispatchEvent(event)
  })
</script>

<section class="antialiased bg-gray-100 text-gray-600 h-screen md:px-4">
  <div class="flex flex-col justify-center">
    <SongList />
  </div>
</section>

