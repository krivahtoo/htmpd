<script>
  import { fly, fade } from 'svelte/transition'
  import SongList from '../components/SongList.svelte'
</script>

<section class="antialiased bg-gray-100 text-gray-600 h-screen px-4">
  <div class="flex flex-col justify-center">
    <!-- Table -->
    <SongList />
  </div>
</section>

