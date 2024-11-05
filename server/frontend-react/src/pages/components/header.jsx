function HeaderComponent() {
  return (
    <header className="header">
      <h1 className="header__title">Smart Startup System</h1>
      <nav className="header__nav">
        <ul className="header__links">
          <li><a className="header__link" href="/config">Config</a></li>
          <li><a className="header__link" href="/status">Status</a></li>
          <li><a className="header__link" href="/about">About</a></li>
        </ul>
      </nav>
    </header>
  )
}

export default HeaderComponent