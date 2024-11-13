<script>
  import { devices, connectionStatus, selectedDevice } from '../../scripts/WebSocket.svelte';
  
  function handleDeviceSelect(device) {
    $selectedDevice = device;
  }
</script>

<section class="device-list">
  <h2 class="device-list__title">Devices</h2>
  {#if $connectionStatus !== 'connected'}
    <p>Status: {$connectionStatus}</p>
  {:else if $devices.length === 0}
    <p>No devices found</p>
  {:else}
    <ul class="device-list__list">
      {#each $devices as device, deviceIndex}
        <button 
          class="device-list__button"
          class:device-list__button-connected={device.dConnected}
          class:device-list__button-disconnected={!device.dConnected}
          class:device-list__button-selected={$selectedDevice?.dMac === device.dMac}
          on:click={() => handleDeviceSelect(device)}
        >
          {deviceIndex === $devices.length - 1 ? '└──' : '├──'} {device.dMac}|{device.dName}|{device.dId}|{device.dIp}
          {#if device.dSockets && device.dSockets.length > 0}
            <ul class="device-list__sockets">
              {#each device.dSockets as socket, socketIndex}
                <li>
                  {deviceIndex === $devices.length - 1 ? '    ' : '│   '}
                  {socketIndex === device.dSockets.length - 1 ? '└──' : '├──'} 
                  {socket.sId}|{socket.sName}|{socket.sState ? 'On' : 'Off'}|{socket.sInUse ? 'Active' : 'Inactive'}
                </li>
              {/each}
            </ul>
          {:else}
            <p>
              {deviceIndex === $devices.length - 1 ? '    ' : '│   '}└── No sockets available
            </p>
          {/if}
        </button>
      {/each}
    </ul>
  {/if}
</section>

<style lang="scss">
  @use '../../styles/variables' as *;

  .device-list {
    display: flex;
    flex-direction: column;
    align-items: left;
    width: 80%;
    margin: 0 auto;
    height: 40vh;
    &__title {
      font-size: $font-large;
      font-family: $main-font;
      color: $secondary-color;
      margin: 0.5rem 0;
    }
    &__list {
      background-color: $secondary-color;
      padding: 1rem;
      font-family: $mono-font;
      font-size: $font-medium;
      height: 100%;
      overflow-y: auto;
      list-style: none;
    }
    &__button {
      background: none;
      color: inherit;
      border: none;
      padding: 0;
      font: inherit;
      cursor: pointer;
      outline: inherit;
      text-align: left;
      width: 100%;
      &-connected {
        color: $primary-color;
      }
      
      &-disconnected {
        color: $accent-color;
      }
      
      &-selected {
        color: $highlight-color;
      }
    }
    &__sockets {
      list-style: none;
      padding-left: 3rem;
      font-family: $mono-font;
    }
  }
</style>
