import Home from './routes/Home.svelte'
import Search from './routes/Search.svelte'
import NotFound from './routes/NotFound.svelte'

export default {
    '/': Home,
    '/search/:query': Search,
    // The catch-all route must always be last
    '*': NotFound
}
