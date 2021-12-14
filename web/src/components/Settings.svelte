<script>
  import { createEventDispatcher, onDestroy } from 'svelte'
  import { fade, scale } from 'svelte/transition'

  import { status } from '../stores.js'

  const dispatch = createEventDispatcher()
  const close = () => dispatch('close')

  let modal
  let repeat = false
  let single = false
  let random = false
  let consume = false
  let crossfade = 0
  let tab = 0

  const handle_keydown = e => {
    if (e.key === 'Escape') {
      close()
      return
    }

    if (e.key === 'Tab') {
      // trap focus
      const nodes = modal.querySelectorAll('*')
      const tabbable = Array.from(nodes).filter(n => n.tabIndex >= 0)

      let index = tabbable.indexOf(document.activeElement)
      if (index === -1 && e.shiftKey) index = 0

      index += tabbable.length + (e.shiftKey ? -1 : 1)
      index %= tabbable.length

      tabbable[index].focus()
      e.preventDefault()
    }
  }

  const previously_focused = typeof document !== 'undefined' && document.activeElement

  if (previously_focused) {
    onDestroy(() => {
      previously_focused.focus()
    })
  }

  status.subscribe(({ repeat: r, single: s, random: ra, crossfade: c, consume: cn }) => {
    repeat = r == 1
    single = s == 1
    random = ra == 1
    crossfade = c
    consume = cn == 1
  })
  function set(cmd, val) {
    if (cmd !== 'crossfade') {
      dispatch('toggle', [cmd, { mode: val }])
    } else {
      dispatch('toggle', [cmd, { seconds: val }])
    }
  }
</script>

<svelte:window on:keydown={handle_keydown}/>

<div
  on:click={close}
  in:fade={{ duration: 100 }}
  out:fade={{ duration: 100 }}
  class="fixed top-0 left-0 w-screen h-screen z-40 bg-gray-900 opacity-75 backdrop-filter backdrop-blur-2xl"></div>

<div class="absolute top-1/2 left-1/2 w-full md:w-10/12 h-full md:h-auto -translate-x-1/2 -translate-y-1/2 z-50 p-1 md:rounded-3xl bg-gray-100 flex flex-col overflow-auto"
  role="dialog" aria-modal="true" in:scale={{ duration: 100 }} out:scale={{ duration: 100 }} bind:this={modal}>
  <div class="flex flex-col items-start p-4">
    <div class="flex items-center w-full">
      <div class="text-gray-900 font-medium text-3xl">Settings</div>
      <svg on:click={close} class="ml-auto fill-current text-gray-700 w-6 h-6 cursor-pointer" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 18 18">
        <path d="M14.53 4.53l-1.06-1.06L9 7.94 4.53 3.47 3.47 4.53 7.94 9l-4.47 4.47 1.06 1.06L9 10.06l4.47 4.47 1.06-1.06L10.06 9z"/>
      </svg>
    </div>
  </div>

  <div style='border-bottom: 1px solid #6366f1' class="mb-4">
    <ul class='flex cursor-pointer'>
      <li
        on:click={ () => tab = 0 }
        class="py-2 px-6 rounded-t-xl { tab === 0 ? 'bg-indigo-600 text-white' : 'bg-white hover:bg-indigo-400 hover:text-white' }">
        Playlist
      </li>
      <li
        on:click={ () => tab = 1 }
        class="py-2 px-6 rounded-t-xl { tab === 1 ? 'bg-indigo-600 text-white' : 'bg-white hover:bg-indigo-400 hover:text-white' }">
        General
      </li>
      <li
        on:click={ () => tab = 2 }
        class="py-2 px-6 rounded-t-xl { tab == 2 ? 'bg-indigo-600 text-white' : 'bg-white hover:bg-indigo-400 hover:text-white' }">
        Advanced
      </li>
    </ul>
  </div>

{#if tab == 0}
  <div class="flex flex-row justify-between w-full">
    <div class="flex flex-col items-center w-full pl-4">
      <label for="random" class="flex p-3 items-center cursor-pointer">
        <div class="px-2">Random</div>
        <div class="relative">
          <input id="random" type="checkbox" class="hidden" bind:checked={random} on:change={() => set('random', random)} />
          <div class="toggle-path bg-gray-200 w-9 h-5 rounded-full shadow-inner" ></div>
          <div class="toggle-circle absolute w-3.5 h-3.5 bg-white rounded-full shadow inset-y-0 left-0" ></div>
        </div>
      </label>
      <label for="toogleButton" class="flex p-3 items-center cursor-pointer">
        <div class="px-2">Single</div>
        <div class="relative">
          <input id="toogleButton" type="checkbox" class="hidden" bind:checked={single} on:change={() => set('single', single)} />
          <div class="toggle-path bg-gray-200 w-9 h-5 rounded-full shadow-inner" ></div>
          <div class="toggle-circle absolute w-3.5 h-3.5 bg-white rounded-full shadow inset-y-0 left-0" ></div>
        </div>
      </label>
      <label for="repeat" class="flex py-3 items-center cursor-pointer">
        <div class="px-2">Repeat</div>
        <div class="relative">
          <input id="repeat" type="checkbox" class="hidden" bind:checked={repeat} on:change={() => set('repeat', repeat)}  />
          <div class="toggle-path bg-gray-200 w-9 h-5 rounded-full shadow-inner"></div>
          <div class="toggle-circle absolute w-3.5 h-3.5 bg-white rounded-full shadow inset-y-0 left-0"></div>
        </div>
      </label>
      <label for="consume" class="flex py-3 items-center cursor-pointer">
        <div class="px-2">Consume</div>
        <div class="relative">
          <input id="consume" type="checkbox" class="hidden" bind:checked={consume} on:change={() => set('consume', consume)}  />
          <div class="toggle-path bg-gray-200 w-9 h-5 rounded-full shadow-inner"></div>
          <div class="toggle-circle absolute w-3.5 h-3.5 bg-white rounded-full shadow inset-y-0 left-0"></div>
        </div>
      </label>
      <label for="crossfade" class="flex py-3 items-center">
        <div class="px-2">Crossfade</div>
        <input type="number" min="0" max="20" id="crossfade" bind:value={crossfade} on:change={() => set('crossfade', crossfade)} 
          class="bg-white w-full h-8 px-4 pr-10 rounded-2xl border-2 text-sm">
      </label>
    </div>
  </div>
{:else if tab == 1}
  General
{:else}
  Advanced
{/if}
  <div class="ml-auto mb-5 mr-5">
    <button on:click={close}
      class="bg-indigo-500 hover:bg-indigo-700 text-white font-bold py-2 px-4 rounded-3xl">
      Ok
    </button>
    <button
      on:click={close}
      class="bg-transparent hover:bg-gray-200 text-gray-700 font-semibold py-2 px-4 border border-gray-500 hover:border-transparent rounded-3xl">
      Close
    </button>
  </div>
</div>

<style>
  .toggle-path {
    transition: background 0.3s ease-in-out;
  }
  .toggle-circle {
    top: 0.2rem;
    left: 0.25rem;
    transition: all 0.3s ease-in-out;
  }
  input:checked ~ .toggle-circle {
    transform: translateX(100%);
  }
  input:checked ~ .toggle-path {
    background-color:#6366f1;
  }
</style>

