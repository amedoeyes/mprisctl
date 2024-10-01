# Maintainer: Ahmed AbouEleyoun <amedoeyes@gmail.com>
pkgname="mprisctl"
pkgver="0.1.1"
pkgrel=1
pkgdesc="Command-line utility that allows you to control MPRIS-compatible media players"
url="https://github.com/amedoeyes/mprisctl"
arch=("x86_64")
license=("GPL3")
depends=("dbus")
makedepends=("xmake" "gcc" "git")
source=("git+https://github.com/amedoeyes/mprisctl.git")
sha256sums=("SKIP")

build() {
	cd "$srcdir/$pkgname"
	xmake f -m release
	xmake
}

package() {
	cd "$srcdir/$pkgname"
	xmake i -o "$pkgdir/usr" --root
}
