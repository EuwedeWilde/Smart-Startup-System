<script>
  import Header from './lib/Header.svelte';
  import DeviceList from './lib/DeviceList.svelte';
  import DeviceConfig from './lib/DeviceConfig.svelte';
  import { connectWebSocket } from './scripts/WebSocket.svelte';
  import { onMount } from 'svelte';

  onMount(() => {
    connectWebSocket();
  });
</script>

<main class="main">
  <Header />
  <DeviceList />
  <DeviceConfig />
</main>
