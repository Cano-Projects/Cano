# Maintainer: CobbCoding 
pkgname=cano-git
_pkgname=cano
pkgver=r422.225d457
pkgrel=1
pkgdesc="Terminal-based modal text editor"
arch=('x86_64')
url="https://github.com/Cano-Projects/cano"
license=('Apache-2.0')
conflicts=('cano')
depends=('ncurses' 'glibc')
makedepends=('git' 'gcc')
source=("$_pkgname::git+https://github.com/Cano-Projects/$_pkgname.git")
md5sums=('SKIP')

pkgver() {
    cd "$_pkgname" 
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
    cd "$_pkgname"
    gcc nob.c -o nob
    ./nob
}

package() {
    cd "$_pkgname"
    mkdir -p "$pkgdir/usr/bin"
    mkdir -p "$pkgdir/usr/share"
    ./nob --prefix "$pkgdir" --install
    install -Dm755 ./README.md "$pkgdir/usr/share/doc/$_pkgname"
}
