<script>
  import { createEventDispatcher } from 'svelte';
  import { fly, fade } from 'svelte/transition'
  // import { elasticOut, expoOut } from 'svelte/easing'
  
  import { current, playing, status } from './../stores.js'
  import { secondsToString } from './../utils.js'

  let isPlaying = false
  let currentTime = 0
  let song = {}

  const dispatch = createEventDispatcher()

  current.subscribe(v => {
    song = v
  })
  playing.subscribe(v => {
    isPlaying = v
  })
  status.subscribe(v => {
    if (song.id === v.current) {
      currentTime = v.elapsedTime/1000 || 0
    }
  })
</script>

<div
  in:fly="{{ duration: 200, delay: 0, y: 200 }}"
  class="fixed flex backdrop-filter backdrop-blur-3xl bottom-2 w-11/12 h-20 z-50 ml-7 align-middle justify-items-center rounded-3xl bg-blue-100 bg-opacity-50 shadow-lg drop-shadow-2xl">
  <img
    in:fly="{{delay: 200, duration: 300, y: 100,}}"
    class="w-16 my-auto ml-6 rounded-full shadow-lg drop-shadow-lg overflow-hidden bg-blue-200"
    src="/logo.svg" 
    alt="{ song.title }">
  <div class="ml-4 my-auto w-1/6">
    <h1
      in:fade="{{delay: 250, duration: 300}}"
      class="mb-1 text-xl font-bold text-black">{ (song.title.length > 25) ? song.title.substr(0, 24) + '...' : song.title }</h1>
    <h2
      in:fade="{{delay: 500, duration: 300}}"
      class="text-xs text-gray-400">{ (song.artist.length > 25) ? song.artist.substr(0, 24) + '...' : song.artist }</h2>
  </div>
  <div class="flex ml-4 my-auto">
    <button class="rounded-full hover:bg-blue-300 focus:outline-none" on:click="{ () => dispatch('prev') }">
      <svg class="w-4 h-4 hover:stroke-current" viewBox="0 0 24 24" fill="none" stroke="#000" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <polygon points="19 20 9 12 19 4 19 20"></polygon>
        <line x1="5" y1="19" x2="5" y2="5"></line>
      </svg>
    </button>
    <button
      class="rounded-full w-10 h-10 flex items-center justify-center pl-0.5 mx-4 ring-1 ring-black hover:bg-blue-300 focus:outline-none"
      on:click="{ () => dispatch('toggle') }">
      <svg class="w-5 h-5" viewBox="0 0 24 24" fill="none" stroke="#000" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        {#if isPlaying}
          <line in:fade="{{ duration: 200 }}"  x1="5" y1="19" x2="5" y2="5"></line>
          <line in:fade="{{ duration: 200 }}" x1="17" y1="5" x2="17" y2="19"></line>
        {:else}
          <polygon in:fade="{{ duration: 200 }}" points="5 3 19 12 5 21 5 3"></polygon>
        {/if}
      </svg>
    </button>
    <button class="rounded-full hover:bg-blue-300 focus:outline-none" on:click="{ () => dispatch('next') }">
      <svg class="w-4 h-4" viewBox="0 0 24 24" fill="none" stroke="#000" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <polygon points="5 4 15 12 5 20 5 4"></polygon><line x1="19" y1="5" x2="19" y2="19"></line>
      </svg>
    </button>
  </div>
  <div class="relative w-full sm:w-1/2 md:w-7/12 lg:w-4/6 ml-5 my-auto">
    <!-- <div class="bg-blue-300 h-2 w-full rounded-lg"></div> -->
    <!-- <div -->
    <!--   class="bg-blue-500 h-2 rounded-lg absolute top-0" -->
    <!--   style="width: { currentTime/song.duration * 100}%;"> -->
    <!--   <div class="bg-blue-600 h-4 w-4 rounded-full absolute right-0 -top-1"></div> -->
    <!-- </div> -->
    <input
      class="rounded-lg overflow-hidden appearance-none bg-blue-300 h-3 w-full"
      type="range" min="0" max="{ song.duration }" step="1" value="{ currentTime }" />
  </div>
  <div class="flex justify-end w-full sm:w-auto pt-1 sm:pt-0 my-auto">
    <span class="text-xs text-gray-700 uppercase font-medium pl-2">
      { secondsToString(currentTime) }/{ secondsToString(song.duration) }
    </span>
  </div>
  <div class="flex justify-end ml-6 mr-4 my-auto">
    <button>
      <svg aria-hidden="true" focusable="false" data-prefix="far" data-icon="heart" class="w-5 h-5" role="img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512">
        <path fill="currentColor" d="M462.1 62.86C438.8 41.92 408.9 31.1 378.7 32c-37.49 0-75.33 15.4-103 43.98l-19.7 20.27l-19.7-20.27C208.6 47.4 170.8 32 133.3 32C103.1 32 73.23 41.93 49.04 62.86c-62.14 53.79-65.25 149.7-9.23 207.6l193.2 199.7C239.4 476.7 247.6 480 255.9 480c8.332 0 16.69-3.267 23.01-9.804l193.1-199.7C528.2 212.5 525.1 116.6 462.1 62.86zM437.6 237.1l-181.6 187.8L74.34 237.1C42.1 203.8 34.46 138.1 80.46 99.15c39.9-34.54 94.59-17.5 121.4 10.17l54.17 55.92l54.16-55.92c26.42-27.27 81.26-44.89 121.4-10.17C477.1 138.6 470.5 203.1 437.6 237.1z"></path>
      </svg>
    </button>
  </div>
</div>

<style>
  @media screen and (-webkit-min-device-pixel-ratio: 0) {
    input[type="range"]::-webkit-slider-thumb {
      width: 15px;
      -webkit-appearance: none;
      appearance: none;
      height: 15px;
      /* cursor: ew-resize; */
      background: #FFF;
      box-shadow: -405px 0 0 400px #6366f1;
      border-radius: 50%;
    }
  }
</style>

