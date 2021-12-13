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
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400">
          <svg aria-hidden="true" focusable="false" data-prefix="fas" data-icon="house-chimney" class="w-5 h-5" role="img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 576 512"><path fill="currentColor" d="M511.8 287.6L512.5 447.7C512.5 450.5 512.3 453.1 512 455.8V472C512 494.1 494.1 512 472 512H456C454.9 512 453.8 511.1 452.7 511.9C451.3 511.1 449.9 512 448.5 512H392C369.9 512 352 494.1 352 472V384C352 366.3 337.7 352 320 352H256C238.3 352 224 366.3 224 384V472C224 494.1 206.1 512 184 512H128.1C126.6 512 125.1 511.9 123.6 511.8C122.4 511.9 121.2 512 120 512H104C81.91 512 64 494.1 64 472V360C64 359.1 64.03 358.1 64.09 357.2V287.6H32.05C14.02 287.6 0 273.5 0 255.5C0 246.5 3.004 238.5 10.01 231.5L266.4 8.016C273.4 1.002 281.4 0 288.4 0C295.4 0 303.4 2.004 309.5 7.014L416 100.7V64C416 46.33 430.3 32 448 32H480C497.7 32 512 46.33 512 64V185L564.8 231.5C572.8 238.5 576.9 246.5 575.8 255.5C575.8 273.5 560.8 287.6 543.8 287.6L511.8 287.6z">
            </path>
          </svg>
        </span>
        <span class="text-sm font-medium">Home</span>
        <span class="ml-auto mr-6 text-sm bg-red-100 rounded-full px-3 py-px text-red-500">{ $queue.length }</span>
      </a>
    </li>
    <li>
      <a href="#/playlist" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400">
          <svg aria-hidden="true" focusable="false" data-prefix="fas" data-icon="list" class="w-5 h-5" role="img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512">
            <path fill="currentColor" d="M88 48C101.3 48 112 58.75 112 72V120C112 133.3 101.3 144 88 144H40C26.75 144 16 133.3 16 120V72C16 58.75 26.75 48 40 48H88zM480 64C497.7 64 512 78.33 512 96C512 113.7 497.7 128 480 128H192C174.3 128 160 113.7 160 96C160 78.33 174.3 64 192 64H480zM480 224C497.7 224 512 238.3 512 256C512 273.7 497.7 288 480 288H192C174.3 288 160 273.7 160 256C160 238.3 174.3 224 192 224H480zM480 384C497.7 384 512 398.3 512 416C512 433.7 497.7 448 480 448H192C174.3 448 160 433.7 160 416C160 398.3 174.3 384 192 384H480zM16 232C16 218.7 26.75 208 40 208H88C101.3 208 112 218.7 112 232V280C112 293.3 101.3 304 88 304H40C26.75 304 16 293.3 16 280V232zM88 368C101.3 368 112 378.7 112 392V440C112 453.3 101.3 464 88 464H40C26.75 464 16 453.3 16 440V392C16 378.7 26.75 368 40 368H88z">
            </path>
          </svg>
        </span>
        <span class="text-sm font-medium">Playlist</span>
        <span class="ml-auto mr-6 text-sm bg-red-100 rounded-full px-3 py-px text-red-500">0</span>
      </a>
    </li>
    <li>
      <a href="#/browse" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400">
          <svg aria-hidden="true" focusable="false" data-prefix="fas" data-icon="folder" class="w-5 h-5" role="img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512">
          <path fill="currentColor" d="M464 96h-192l-64-64h-160C21.5 32 0 53.5 0 80V160h512V144C512 117.5 490.5 96 464 96zM0 432C0 458.5 21.5 480 48 480h416c26.5 0 48-21.5 48-48V192H0V432z">
            </path>
          </svg>
        </span>
        <span class="text-sm font-medium">Browse</span>
        <span class="ml-auto mr-6 text-sm bg-red-100 rounded-full px-3 py-px text-red-500">0</span>
      </a>
    </li>
    <li>
      <a href="#/browse" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400">
          <svg aria-hidden="true" focusable="false" data-prefix="fas" data-icon="record-vinyl" class="w-5 h-5" role="img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 512 512">
            <path fill="currentColor" d="M256 160C202.9 160 160 202.9 160 256s42.92 96 96 96c53.08 0 96-42.92 96-96S309.1 160 256 160zM256 288C238.3 288 224 273.7 224 256s14.33-32 32-32c17.67 0 32 14.33 32 32S273.7 288 256 288zM256 0c-141.4 0-256 114.6-256 256s114.6 256 256 256c141.4 0 256-114.6 256-256S397.4 0 256 0zM256 384c-70.75 0-128-57.25-128-128s57.25-128 128-128s128 57.25 128 128S326.8 384 256 384z">
            </path>
          </svg>
        </span>
        <span class="text-sm font-medium">Albums</span>
        <span class="ml-auto mr-6 text-sm bg-red-100 rounded-full px-3 py-px text-red-500">0</span>
      </a>
    </li>
    <li>
      <a href="#/browse" class="flex flex-row items-center h-12 transform hover:translate-x-2 transition-transform ease-in duration-200 text-gray-500 hover:text-gray-800">
        <span class="inline-flex items-center justify-center h-12 w-12 text-lg text-gray-400">
          <svg aria-hidden="true" focusable="false" data-prefix="fas" data-icon="user" class="w-5 h-5" role="img" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 448 512">
            <path fill="currentColor" d="M224 256c70.7 0 128-57.31 128-128s-57.3-128-128-128C153.3 0 96 57.31 96 128S153.3 256 224 256zM274.7 304H173.3C77.61 304 0 381.6 0 477.3c0 19.14 15.52 34.67 34.66 34.67h378.7C432.5 512 448 496.5 448 477.3C448 381.6 370.4 304 274.7 304z">
            </path>
          </svg>
        </span>
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
