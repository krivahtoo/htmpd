<script>
  import { createEventDispatcher } from 'svelte';
  import { fly, fade } from 'svelte/transition'
  // import { elasticOut, expoOut } from 'svelte/easing'
  import ColorThief from 'colorthief'
  
  import { current, playing, status } from './../stores.js'
  import { secondsToString } from './../utils.js'

  let isPlaying = false
  let currentTime = 0
  let song = {}

  let startColor = 'rgba(0, 20, 255, 0.5)'
  let endColor = 'rgba(255, 20, 255, 0.5)'
  let textColor = 'rgba(255, 255, 255, 0.5)'

  const dispatch = createEventDispatcher()

  current.subscribe(v => {
    if (v && song.id !== v.id) song = v
  })
  playing.subscribe(v => {
    isPlaying = v
  })
  status.subscribe(v => {
    if (song.id === v.current) {
      currentTime = v.elapsedTime/1000 || 0
    }
    // Player stopped
    if (v.state == 1) {
      song = {
        title: "--",
        artist:"-",
        album:"-",
        duration:100,
        id:-1,
        pos:-1,
        uri:""
      }
      currentTime = 0
    }
  })

  function getColor(e) {
    const colorThief = new ColorThief()
    const color = colorThief.getColor(e.target)
    startColor = `rgb(${color[0]}, ${color[1]}, ${color[2]}, 0.5)`
    // textColor = `rgb(${255 - color[0]}, ${255-color[1]}, ${255-color[2]})`
    const [r, g, b] = colorThief.getPalette(e.target, 5)[1]
    endColor = `rgb(${r}, ${g}, ${b}, 0.5)`
    textColor = (color[0] * 0.299 + color[1] * 0.587 + color[2] * 0.114) > 186 ? '#000000' : '#FFFFFF'
  }
</script>

<div
  in:fly={{ duration: 200, delay: 0, y: 200 }}
  style="background: linear-gradient(to right bottom, { startColor }, { endColor } 90%); color: {textColor};"
  class="fixed flex backdrop-filter backdrop-blur-3xl bottom-2 w-11/12 h-20 z-50 ml-7 align-middle justify-items-center rounded-3xl bg-opacity-50 shadow-lg drop-shadow-2xl">
  <img
    in:fly={{delay: 200, duration: 300, y: 100,}}
    class="w-16 my-auto ml-6 rounded-full shadow-lg drop-shadow-lg overflow-hidden bg-blue-200"
    src={ song.uri }
    onerror="this.src='/logo.svg'"
    on:load={getColor}
    alt={ song.title }>
  <div class="ml-4 my-auto w-1/6">
    <h1
      in:fade={{delay: 250, duration: 300}}
      class="mb-1 text-xl font-bold">{ (song.title.length > 25) ? song.title.substr(0, 24) + '...' : song.title }</h1>
    <h2
      in:fade={{delay: 500, duration: 300}}
      class="text-xs opacity-70">{ (song.artist.length > 25) ? song.artist.substr(0, 24) + '...' : song.artist }</h2>
  </div>
  <div class="flex ml-4 my-auto">
    <button class="rounded-full hover:bg-blue-300 focus:outline-none" on:click={ () => dispatch('prev') }>
      <svg class="w-4 h-4 hover:stroke-current" viewBox="0 0 24 24" fill="none" stroke="{ textColor }" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <polygon points="19 20 9 12 19 4 19 20"></polygon>
        <line x1="5" y1="19" x2="5" y2="5"></line>
      </svg>
    </button>
    <button
      style="border:2px solid { textColor };"
      class="rounded-full w-10 h-10 flex items-center justify-center pl-0.5 mx-4 hover:bg-blue-300 focus:outline-none"
      on:click={ () => dispatch('toggle') }>
      <svg class="w-5 h-5" viewBox="0 0 24 24" fill="none" stroke={ textColor } stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        {#if isPlaying}
          <line in:fade={{ duration: 200 }}  x1="5" y1="19" x2="5" y2="5"></line>
          <line in:fade={{ duration: 200 }} x1="17" y1="5" x2="17" y2="19"></line>
        {:else}
          <polygon in:fade={{ duration: 200 }} points="5 3 19 12 5 21 5 3"></polygon>
        {/if}
      </svg>
    </button>
    <button class="rounded-full hover:bg-blue-300 focus:outline-none" on:click={ () => dispatch('next') }>
      <svg class="w-4 h-4" viewBox="0 0 24 24" fill="none" stroke={ textColor } stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <polygon points="5 4 15 12 5 20 5 4"></polygon><line x1="19" y1="5" x2="19" y2="19"></line>
      </svg>
    </button>
  </div>
  <div class="relative w-full sm:w-1/2 md:w-7/12 lg:w-4/6 ml-5 my-auto">
    <input
      class="rounded-lg overflow-hidden appearance-none bg-blue-300 h-3 w-full"
      type="range" min="0" max={ song.duration } step="1" bind:value={currentTime}
      on:change={ () => dispatch('seek', { id: song.id, pos: currentTime }) } />
  </div>
  <div class="flex justify-end w-full sm:w-auto pt-1 sm:pt-0 my-auto">
    <span class="text-xs uppercase font-medium pl-2">
      { secondsToString(currentTime) }/{ secondsToString(song.duration) }
    </span>
  </div>
  <div class="flex justify-end ml-6 mr-4 my-auto">
    <button>
      <svg aria-hidden="true" focusable="false" data-prefix="far" data-icon="heart" class="w-5 h-5" role="img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512">
        <path fill="currentColor" d="M462.1 62.86C438.8 41.92 408.9 31.1 378.7 32c-37.49 0-75.33 15.4-103 43.98l-19.7 20.27l-19.7-20.27C208.6 47.4 170.8 32 133.3 32C103.1 32 73.23 41.93 49.04 62.86c-62.14 53.79-65.25 149.7-9.23 207.6l193.2 199.7C239.4 476.7 247.6 480 255.9 480c8.332 0 16.69-3.267 23.01-9.804l193.1-199.7C528.2 212.5 525.1 116.6 462.1 62.86zM437.6 237.1l-181.6 187.8L74.34 237.1C42.1 203.8 34.46 138.1 80.46 99.15c39.9-34.54 94.59-17.5 121.4 10.17l54.17 55.92l54.16-55.92c26.42-27.27 81.26-44.89 121.4-10.17C477.1 138.6 470.5 203.1 437.6 237.1z"></path>
      </svg>
    </button>
    <button class="ml-3">
      <svg aria-hidden="true" focusable="false" data-prefix="fas" data-icon="volume-high" class="w-5 h-5" role="img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 640 512">
        <path fill="currentColor" d="M412.6 182c-10.28-8.334-25.41-6.867-33.75 3.402c-8.406 10.24-6.906 25.35 3.375 33.74C393.5 228.4 400 241.8 400 255.1c0 14.17-6.5 27.59-17.81 36.83c-10.28 8.396-11.78 23.5-3.375 33.74c4.719 5.806 11.62 8.802 18.56 8.802c5.344 0 10.75-1.779 15.19-5.399C435.1 311.5 448 284.6 448 255.1S435.1 200.4 412.6 182zM473.1 108.2c-10.22-8.334-25.34-6.898-33.78 3.34c-8.406 10.24-6.906 25.35 3.344 33.74C476.6 172.1 496 213.3 496 255.1s-19.44 82.1-53.31 110.7c-10.25 8.396-11.75 23.5-3.344 33.74c4.75 5.775 11.62 8.771 18.56 8.771c5.375 0 10.75-1.779 15.22-5.431C518.2 366.9 544 313 544 255.1S518.2 145 473.1 108.2zM534.4 33.4c-10.22-8.334-25.34-6.867-33.78 3.34c-8.406 10.24-6.906 25.35 3.344 33.74C559.9 116.3 592 183.9 592 255.1s-32.09 139.7-88.06 185.5c-10.25 8.396-11.75 23.5-3.344 33.74C505.3 481 512.2 484 519.2 484c5.375 0 10.75-1.779 15.22-5.431C601.5 423.6 640 342.5 640 255.1S601.5 88.34 534.4 33.4zM301.2 34.98c-11.5-5.181-25.01-3.076-34.43 5.29L131.8 160.1H48c-26.51 0-48 21.48-48 47.96v95.92c0 26.48 21.49 47.96 48 47.96h83.84l134.9 119.8C272.7 477 280.3 479.8 288 479.8c4.438 0 8.959-.9314 13.16-2.835C312.7 471.8 320 460.4 320 447.9V64.12C320 51.55 312.7 40.13 301.2 34.98z">
        </path>
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

