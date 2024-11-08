<script>
  import { Router, Link, Route } from "svelte-routing";
  import Config from "./routes/Config.svelte";
  import Docs from "./routes/Docs.svelte";

  export let url = "";
</script>

<Router {url}>
  <div>
    <Route path="/"  component={Config}></Route>
    <Route path="/docs"  component={Docs}></Route>
  </div>
</Router>