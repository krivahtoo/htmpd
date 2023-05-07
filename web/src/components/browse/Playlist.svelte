<script>
  export let type
  export let path = ''
  export let last_modified

  import { toDate, formatDistance } from 'date-fns'

  const addToPlaying = () => {
    let event = new CustomEvent('add_playlist', { detail: path })
    document.dispatchEvent(event)
  }
</script>

<tr
  class="px-2 cursor-pointer hover:bg-gray-200"
  on:click={ addToPlaying }>
  <td class="p-2">./</td>
  <td class="p-2 text-sm leading-5 font-medium">{ path }</td>
  <td class="p-2">{ formatDistance(toDate(last_modified*1000), new Date(), { addSuffix: true }) }</td>
</tr>

