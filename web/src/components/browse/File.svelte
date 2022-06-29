<script>
  export const type = ''
  export const genre = ''
  export let duration = 0
  export let title = '--'
  export let artist = '--'
  export const uri = ''
  export const year = ''
  export const album = ''

  import { secondsToString } from '../../utils.js'

  const addToPlaying = () => {}
</script>

<tr
  class="px-2 cursor-pointer hover:bg-gray-200"
  on:click={ addToPlaying }>
  <td class="p-2">./</td>
  <td class="p-2 text-sm leading-5 font-medium">{ artist } -  { title }</td>
  <td class="p-2">{ secondsToString(duration) }</td>
</tr>


