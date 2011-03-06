# Maintainer: Emil Renner Berthing <esmil@mailme.dk>

pkgname=lem-evdev
pkgver=0.1
pkgrel=1
pkgdesc="Event device library for the Lua Event Machine"
arch=('i686' 'x86_64' 'armv5tel' 'armv7l')
url="https://github.com/esmil/lem-evdev"
license=('GPL')
depends=('lem')
source=()

build() {
  cd "$startdir"

  make NDEBUG=1
}

package() {
  cd "$startdir"

  make DESTDIR="$pkgdir/" PREFIX='/usr' install
}

# vim:set ts=2 sw=2 et:
