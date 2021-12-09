<script>
  import { queue } from './../stores.js'
</script>

<div class="flex flex-col w-1/5 bg-white rounded-r-3xl overflow-hidden">
  <div class="flex items-center justify-center h-20 shadow-md">
    <h1 class="text-3xl uppercase text-indigo-500">htmpd</h1>
  </div>
  <ul class="flex flex-col py-4">
    <li>
      <a href="#/" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400"><i class="bx bx-home"></i></span>
        <span class="text-sm font-medium">Home</span>
        <span class="ml-auto mr-6 text-sm bg-red-100 rounded-full px-3 py-px text-red-500">{ $queue.length }</span>
      </a>
    </li>
    <li>
      <a href="#/playlist" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400"><i class="bx bx-music"></i></span>
        <span class="text-sm font-medium">Playlist</span>
        <span class="ml-auto mr-6 text-sm bg-red-100 rounded-full px-3 py-px text-red-500">1</span>
      </a>
    </li>
    <li>
      <a href="#/browse" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400"><i class="bx bx-drink"></i></span>
        <span class="text-sm font-medium">Browse</span>
      </a>
    </li>
    <li>
      <a href="#/browse" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400"><i class="bx bx-drink"></i></span>
        <span class="text-sm font-medium">Albums</span>
      </a>
    </li>
    <li>
      <a href="#/browse" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400"><i class="bx bx-drink"></i></span>
        <span class="text-sm font-medium">Artists</span>
      </a>
    </li>
    <li>
      <a href="#" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400"><i class="bx bx-bell"></i></span>
        <span class="text-sm font-medium">Notifications</span>
        <span class="ml-auto mr-6 text-sm bg-red-100 rounded-full px-3 py-px text-red-500">5</span>
      </a>
    </li>
    <li>
      <a href="#" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400"><i class="bx bx-log-out"></i></span>
        <span class="text-sm font-medium">Settings</span>
      </a>
    </li>
  </ul>
</div>
