<script>
  import Header from '../lib/layout/Header.svelte';
  import DocsWelcome from '../lib/docs/welcome/+DocsWelcome.svelte';
  import DocsHardware from '../lib/docs/hardware/+DocsHardware.svelte';
  import DocsSoftware from '../lib/docs/software/+DocsSoftware.svelte';
  import DocsClosing from '../lib/docs/closing/+DocsClosing.svelte';
</script>

<main>
  <Header />
  <main class="docs">
    <DocsWelcome />
    <DocsHardware />
    <DocsSoftware />
    <DocsClosing />
  </main>
</main>



