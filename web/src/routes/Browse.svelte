<script>
  import { onMount } from 'svelte'
  import { querystring } from 'svelte-spa-router'
  import List from '../components/browse/List.svelte'

  let search = new URLSearchParams('page=1&limit=20')

  onMount(() => {
    let limit = 20
    let page = 1
    if (search.has('page')) { page = parseInt(search.get('page')) }
    if (search.has('limit')) { limit = parseInt(search.get('limit')) }
    sendParams(page, limit)
    querystring.subscribe(v => {
      limit = 20
      page = 1
      search = new URLSearchParams(v)
      if (search.has('page')) { page = parseInt(search.get('page')) }
      if (search.has('limit')) { limit = parseInt(search.get('limit')) }
      sendParams(page, limit)
    })
  })

  const sendParams = (page, limit) => {
    let data = { offset: 0, limit: 30 }
    if (limit) { data.limit = limit }
    if (page) { data.offset = limit * ( page - 1 ) }
    let event = new CustomEvent('browse', { detail: data })
    document.dispatchEvent(event)
  }
</script>

<section class="antialiased bg-gray-100 text-gray-600 h-5/6 pb-48 md:px-4 overflow-auto">
  <div class="flex flex-col justify-center">
    <List />
  </div>
</section>

