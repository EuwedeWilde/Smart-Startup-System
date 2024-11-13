<script>
  import { devices, selectedDevice, getWebSocket } from '../../scripts/WebSocket.svelte';
  $: deviceName = $selectedDevice?.dConnected ? ($selectedDevice?.dName || '') : '';
  $: deviceId = $selectedDevice?.dConnected ? ($selectedDevice?.dId || '') : '';
  $: deviceIp = $selectedDevice?.dConnected ? ($selectedDevice?.dIp || '') : '';
  $: deviceGroup = $selectedDevice?.dConnected ? ($selectedDevice?.dGroup || '') : '';

  $: if ($selectedDevice && !$selectedDevice.dConnected) {
    deviceName = '';
    deviceId = '';
    deviceIp = '';
    deviceGroup = '';
  }

  function handleSubmit(event) {
    event.preventDefault();
    if ($selectedDevice) {
      const updatedSockets = $selectedDevice.dSockets.map(socket => ({
        ...socket,
        sName: socket.sName || `Socket ${socket.sId}`,
        sGroup: socket.sGroup || '',
        sInUse: socket.sInUse || false
      }));

      const config = {
        dMac: $selectedDevice.dMac,
        dId: deviceId,
        dName: deviceName,
        dIp: deviceIp,
        dGroup: deviceGroup,
        dSockets: updatedSockets
      };

      const ws = getWebSocket();
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
          config: {
            device: $selectedDevice.dMac,
            config: config
          }
        }));
      }
    }
  }

  function toggleSocket(socketId) {
    if ($selectedDevice) {
      const socket = $selectedDevice.dSockets.find(s => s.sId === socketId);
      if (socket) {
        const ws = getWebSocket();
        if (ws && ws.readyState === WebSocket.OPEN) {
          socket.sState = !socket.sState;
          
          // Update the devices store to trigger reactivity
          $devices = $devices.map(device => 
            device.dMac === $selectedDevice.dMac 
              ? {...device, dSockets: device.dSockets.map(s => 
                  s.sId === socketId ? {...s, sState: socket.sState} : s
                )}
              : device
          );
          
          // Update selected device to match
          $selectedDevice = $devices.find(d => d.dMac === $selectedDevice.dMac);

          ws.send(JSON.stringify({
            control: {
              device: $selectedDevice.dMac,
              control: [{
                sId: socketId,
                sState: socket.sState
              }]
            }
          }));
        }
      }
    }
  }

  let activeTab = 'device'; 

  // Add new variable for selected socket
  let selectedSocketId = '';
  $: selectedSocket = $selectedDevice?.dSockets?.find(s => s.sId === selectedSocketId);
</script>

<section class="device-config">
  <h2 class="device-config__title">Device Config</h2>
  
  <div class="device-config__config-field">
    <div class="device-config__tabs">
      <button 
        class="device-config__tabs-button"
        class:device-config__tabs-button-active={activeTab === 'device'}
        on:click={() => activeTab = 'device'}
      >
        Device Configuration
      </button>
      <button 
        class="device-config__tabs-button"
        class:device-config__tabs-button-active={activeTab === 'socket'}
        on:click={() => activeTab = 'socket'}
      >
        Socket Configuration
      </button>
    </div>

    <form class="device-config__form" on:submit={handleSubmit}>
      <div class="device-config__form-content">
        <!-- Device Config -->
        {#if activeTab === 'device'}
          <div class="device-config__form-group">
            <label class="device-config__form-label" for="name">Device Name</label>
            <input class="device-config__form-input"
              type="text" 
              id="name" 
              bind:value={deviceName}
              disabled={!$selectedDevice || !$selectedDevice.dConnected}
            />
            
            <label class="device-config__form-label" for="id">Device ID</label>
            <input class="device-config__form-input"
              type="text" 
              id="id" 
              value={deviceId}
              disabled 
            />
            
            <label class="device-config__form-label" for="ip">Device IP</label>
            <input class="device-config__form-input"
              type="text" 
              id="ip" 
              bind:value={deviceIp}
              disabled={!$selectedDevice || !$selectedDevice.dConnected}
            />

            <label class="device-config__form-label" for="group">Device Group</label>
            <input class="device-config__form-input"
              type="text" 
              id="group" 
              bind:value={deviceGroup}
              disabled={!$selectedDevice || !$selectedDevice.dConnected}
            />
          </div>
        {/if}

        {#if activeTab === 'socket'}
          {#if $selectedDevice?.dSockets && $selectedDevice.dConnected}
            <div class="device-config__socket">
              <div class="device-config__socket-select">
                <label class="device-config__socket-label" for="socket-select">Socket Select</label>
                <select 
                  class="device-config__socket-dropdown"
                  id="socket-select"
                  bind:value={selectedSocketId}
                >
                  <option  value="">Choose a socket...</option>
                  {#each $selectedDevice.dSockets as socket}
                    <option class="device-config__socket-option" value={socket.sId}>
                      {socket.sName || `Socket ${socket.sId}`}
                    </option>
                  {/each}
                </select>
              </div>

              {#if selectedSocket}
                <div class="device-config__socket-selected">                  
                  <label class="device-config__socket-label" for="socket-name"> Socket Name</label>
                  <input 
                    class="device-config__socket-input"
                    type="text" 
                    id="socket-name"
                    bind:value={selectedSocket.sName}
                    placeholder={`Socket ${selectedSocket.sId}`}
                  />

                  <label class="device-config__socket-label" for="socket-group">Socket Group</label>
                  <input 
                    class="device-config__socket-input"
                    type="text" 
                    id="socket-group"
                    bind:value={selectedSocket.sGroup}
                    placeholder="Socket Group"
                  />

                  <label class="device-config__socket-label" for="socket-inuse">Socket State</label>
                  <button 
                    class="device-config__socket-button"
                    class:device-config__socket-button--active={selectedSocket.sState}
                    type="button" 
                    on:click={() => toggleSocket(selectedSocket.sId)}
                  >
                    {selectedSocket.sState ? 'Turn Off' : 'Turn On'}
                  </button>
                </div>
              {/if}
            </div>
          {:else if $selectedDevice && !$selectedDevice.dConnected}
            <p class="device-config__message">Device is disconnected. Configuration unavailable.</p>
          {/if}
        {/if}
      </div>

      <button 
        class="device-config__form-button" 
        type="submit" 
        disabled={!$selectedDevice || !$selectedDevice.dConnected}
      >
        Save Configuration
      </button>
    </form>
  </div>
</section>


<style lang="scss">
  @use '../../styles/variables' as *;

  .device-config {
    height:40vh;
    width: 80%;
    margin: 0 auto;
    display: flex;
    flex-direction: column;
    overflow: hidden;

    &__title {
      font-size: $font-large;
      font-family: $main-font;
      color: $secondary-color;
      margin: 0.5rem 0;
    }
    &__config-field {
      background-color: $secondary-color;
      height: 100%;
      display: flex;
      flex-direction: column;
      
    }
    &__form {
      display: flex;
      flex-direction: column;
      padding: 1rem;

      &-content {
        flex: 1;
        overflow: auto;  // Allow scrolling of content if needed
      }

      &-group {
        display: grid;
        grid-template-columns: repeat(2, 1fr);
        align-items: center;
      }
      &-label {
        font-family: $mono-font;
        color: $primary-color;
        align-self: center;
      }
      &-input {
        font-family: $mono-font;
        color: $primary-color;
        background: none;
        border: 1px solid $primary-color;
        padding: 0;
        font-family: $mono-font;
        font-size: $font-medium;
        cursor: pointer;
        padding: 0 0.5rem;
        text-align: right;
      }
      &-button {
        position: absolute;
        bottom: 0.5rem;
        left: 0;
        right: 0;
        background: none;
        border: none;
        padding: 0.5rem;
        margin: 0 auto;
        width: calc(80% - 2rem);
        font: inherit;
        cursor: pointer;
        color: $primary-color;
        font-size: $font-medium;
        font-family: $mono-font;

        &:hover {
          color: $highlight-color;
        }
      }
    }

    &__tabs {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      &-button {
        background: none;
        border: none;
        font: inherit;
        cursor: pointer;
        color: $primary-color;
        background-color: $accent-color;
        padding: 0.5rem;
        font-size: $font-medium;
        font-family: $mono-font;
        &-active {
          background-color: $secondary-color;
        }

        &:hover {
          color: $highlight-color;
        }

      }
    }

    &__socket {
      display: flex;
      flex-direction: column;

      &-label {
        font-family: $mono-font;
        color: $primary-color;
        align-self: center;
      }
      &-select {
        display: grid;
        grid-template-columns: repeat(2, 1fr);
        align-items: center;
      }

      &-selected {
        display: grid;
        grid-template-columns: repeat(2, 1fr);
        align-items: center;
      }

      &-dropdown {
        font-family: $mono-font;
        color: $primary-color;
        background: none;
        border: 1px solid $primary-color;
        font-size: $font-medium;
        cursor: pointer;
        width: 100%;
        direction: rtl;
        padding: 0 0.5rem;

        &:focus {
          outline: none;
          border-color: $highlight-color;
        }
      }

      &-option {
        background-color: $secondary-color;
        color: $primary-color;
        border: none;
      }

      &-input {
        font-family: $mono-font;
        color: $primary-color;
        background: none;
        border: 1px solid $primary-color;
        font-size: $font-medium;
        text-align: right;
        padding: 0 0.5rem;

        &:focus {
          outline: none;
          border-color: $highlight-color;
        }
      }

      &-button {
        grid-column: 2;
        background: none;
        border: 1px solid $primary-color;
        color: $primary-color;
        font-family: $mono-font;
        font-size: $font-medium;
        cursor: pointer;
        text-align: right;
        padding: 0 0.5rem;


        &:hover {
          color: $highlight-color;
        }

        &--active {
          background-color: $primary-color;
          color: $secondary-color;

          &:hover {
            color: $highlight-color;
          }
        }
      }
    }

    &__message {
      color: $primary-color;
      font-family: $mono-font;
      text-align: center;
      padding: 1rem;
    }

  }
</style>

