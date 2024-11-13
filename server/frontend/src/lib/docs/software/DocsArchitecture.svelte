<section>
  <h3 class="docs__subtitle" id="architecture">Architecture</h3>
  <p class="docs__content">
    All communication between different systems (e.g. the device, database, frontend) all go via the server. You can see the server as the manager, telling everyone what to do and making sure all communication goes seamlessly. This is done with different commutation protocols, depending on the different systems. 
  </p>

  <img src="src/assets/Architecture.png" alt="Architecture" class="docs__image">
  <p class="docs__image-caption">System Architecture of S3</p>
  <p class="docs__content">The system architecture consists out of four systems:</p>
  <ol class="docs__content-list">
    <li><a href="#server" class="docs__link">Server</a></li>
    <li><a href="#device" class="docs__link">Device</a></li>
    <li><a href="#frontend" class="docs__link">Frontend</a></li>
    <li><a href="#database" class="docs__link">Database</a></li>
  </ol>
  <p class="docs__content">
    We will now go through each system in detail.
  </p>
</section>