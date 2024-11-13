<script>
  import Header from '../lib/layout/Header.svelte';
  import DeviceList from '../lib/config/DeviceList.svelte';
  import DeviceConfig from '../lib/config/DeviceConfig.svelte';
  import { connectWebSocket } from '../scripts/WebSocket.svelte';
  import { onMount } from 'svelte';

  onMount(() => {
    connectWebSocket();
  });
</script>

<main>
  <Header />
  <DeviceList />
  <DeviceConfig />
</main>
