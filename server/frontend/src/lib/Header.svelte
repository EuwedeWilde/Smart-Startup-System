<header class="header">
  <h1 class="header__title">Smart Startup System</h1>
  <p class="header__subtitle">By Euwe de Wilde & Interactive Matter</p>
  <nav class="header__nav">
    <a class="header__nav-item" href="/">Config</a>
    <a class="header__nav-item" href="/Schedule">Schedule</a>
    <a class="header__nav-item" href="/docs">Docs</a>
  </nav>
</header>

<style lang="scss">
  @use '../styles/variables' as *;
  
  .header {
    background-color: $secondary-color;
    color: $primary-color;
    height: 20vh;
    width: 80%;
    margin: 0 auto;
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
    &__title {
      font-size: $font-large;
      font-family: $main-font;
    }
    &__subtitle {
      font-size: $font-medium;
      font-style: italic;
      font-family: $main-font;
    }
    &__nav {
      margin-top: 1rem;
      display: flex;
      gap: 1rem;
      &-item {
        color: $primary-color;
        text-decoration: none;
        font-family: $main-font;
        &:hover {
          color: $highlight-color;
        }
      }

    }
  }
</style>
