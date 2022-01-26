<script>
  import { onMount } from "svelte"
  import { querystring } from 'svelte-spa-router'

  export let offset = 2
  export let path = '#/'
  export let pagination = {
    offset: 0,
    limit: 20,
    total: 0
  }

  let page = 1
  let last_page = 1
  let pages = []
  let search = new URLSearchParams('page=1&limit=20')

  onMount(() => {
    let val = Math.ceil(pagination.offset/pagination.limit)
    page = val > 0 ? val : 1 
    last_page = Math.ceil(pagination.total/pagination.limit)
    pages = genPages(last_page, page, offset)
    querystring.subscribe(v => {
      let limit = 20
      search = new URLSearchParams(v)
      if (search.has('page')) { page = parseInt(search.get('page')) }
      if (search.has('limit')) { limit = parseInt(search.get('limit')) }
      pagination.limit = limit
      pagination.offset = limit * ( page - 1 )
      last_page = Math.ceil(pagination.total/pagination.limit)
      pages = genPages(last_page, page, offset)
    })
  })

  // Generate path with querystring e.g. #/browse?page=2&limit=10
  function getPath(page = 1) {
    search.set("page", page)
    return `${path}?${search.toString()}`
  }

  // Generate pages to be displayed. e.g. 1 ... 5 6 7 8 9 ... 20
  // with offset of 2                  current page ↑
  function genPages(size, curr = 1, offset = 3) {
    return [...Array(size).keys()]
      .map(i => i + 1)
      .filter(i => i == 1
        || i === size
        || i === curr
        || ((curr + offset) >= i && curr < i)
        || ((curr - offset) <= i && curr > i)
      ).map(i => {
        return {
          page: i,
          current: (i === curr) ? true : false
        }
      })
  }

</script>

<nav class="ml-5 pb-48">
  <ul class="flex flex-wrap md:inline-flex">
    {#if page > 1}
      <li class="flex ml-3">
        <a href={ getPath(page-1) }
          class="bg-indigo-500 hover:bg-indigo-700 text-white font-bold py-2 px-4 rounded-3xl">
          Previous
        </a>
      </li>
    {/if}
    {#each pages as val}
      <li class="flex ml-3">
        <a href={ getPath(val.page) }
          class="bg-white border { val.current ? 'border-indigo-500 text-indigo-500' : 'border-gray-300 text-gray-500' } hover:bg-gray-100 hover:text-gray-700 rounded-3xl leading-tight py-2 px-3 dark:bg-gray-800 dark:border-gray-700 dark:text-gray-400 dark:hover:bg-gray-700 dark:hover:text-white">
          { val.page }
        </a>
      </li>
      {#if val.page === 1 && page > (offset + 2)}
        <li class="ml-3">
          <span class="text-gray-300 py-2 px-1">
            &hellip;
          </span>
        </li>
      {:else if val.page === (page + offset) && last_page > (val.page + 1) }
        <li class="ml-3">
          <span class="text-gray-300 py-2 px-1">
            &hellip;
          </span>
        </li>
      {/if}
    {/each}
    {#if last_page > page}
      <li class="flex ml-3">
        <a href={ getPath(page+1) }
          class="bg-indigo-500 hover:bg-indigo-700 text-white font-bold py-2 px-4 rounded-3xl">
          Next
        </a>
      </li>
    {/if}
  </ul>
</nav>

