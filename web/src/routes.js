import Home from './routes/Home.svelte'
import Search from './routes/Search.svelte'
import Browse from './routes/Browse.svelte'
import Playlist from './routes/Playlist.svelte'
import NotFound from './routes/NotFound.svelte'

export default {
    '/': Home,
    '/search/:query': Search,
    '/browse': Browse,
    '/playlist/:name': Playlist,
    // The catch-all route must always be last
    '*': NotFound
}
