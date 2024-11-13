<section>
  <h4 class="docs__subtitle" id="frontend">Frontend</h4>
  <p class="docs__content">
    To test the frontend go to the frontend folder and run the following command:
  </p>
  <code class="docs__content-code">
    {@html `// --- Structure ---
Smart-Startup-System
└──server
   ├──backend
   ├──dist
   └──frontend
       └──... // all frontend files

// --- Commands ---
npm run dev // to test the frontend
npm run build // to build the frontend
serve -s dist // to serve the build frontend
`}
  </code>
  <p class="docs__content-footnote">
    Structure and commands
  </p>
  <p class="docs__content">
    The frontend is based on the frontend framework Svelte with Vite as development tool. I use SCSS as styling, svelte already compiles the SCSS, so no need to use an external compiler. I earlier I used React, but I switched, due to the native reactivity of svelte (instead of using useStates).
  </p>
  <h4 class="docs__subsubtitle" id="config">
    Config
  </h4>
  <p class="docs__content">
    The consist out of two parts, the device list and the device config. The device list is to select a device you want to modify. If active device is clicked, values should pop up in the config fields. You can modify these values. When save config is pressed, this send to the server with WebSocket, and that handles the rest.  You can find out how it works here:
  </p>
  <code class="docs__content-code">
    {@html `smart-startup-system/server/frontend/src/lib/config
smart-startup-system/server/frontend/src/scripts`}
  </code>
  <p class="docs__content-footnote">Folders for config page components and frontend WebSocket connection</p>
  <h4 class="docs__subsubtitle" id="schedule">  
    Schedule
  </h4>
  <p class="docs__content">
    This is a feature I still want to add later. This is to automatically let the sockets swich on a time schedule. 
  </p>
  <h4 class="docs__subsubtitle" id="docs">  
    Docs
  </h4>
  <p class="docs__content">
    You are reading this now. All of the content of the docs can be found in the docs folder split in multiple parts. Some files in the folders in the docs folder (what a sentence...) start with a +, this is to indicate that is the file that structures the content in that subfolder. 
  </p>
  <code class="docs__content-code">
    {@html `smart-startup-system/server/frontend/src
├───assets
├───lib
│   ├───config
│   ├───docs
│   │   ├───closing
│   │   ├───hardware
│   │   ├───software
│   │   └───welcome
│   └───layout
├───routes
├───scripts
└───styles`}
  </code>
  <p class="docs__content-footnote">
    Structure of the docs
  </p>
</section>