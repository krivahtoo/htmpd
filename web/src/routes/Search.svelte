<script>
  export let params = { query: '' };

  let query = '';
  $: {
    query = ''
    if (params && params.query) {
      query = params.query
    }
  }
</script>

<h1>Lorem ipsum</h1>

<style>
  h1 {
    color: #008cff;
    text-transform: uppercase;
    font-size: 4em;
    font-weight: 100;
  }
</style>

