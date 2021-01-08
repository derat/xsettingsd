# xsettingsd

xsettingsd is a daemon that implements the *XSETTINGS* specification.

It is intended to be small, fast, and minimally dependent on other
libraries.  It can serve as an alternative to gnome-settings-daemon for
users who are not using the GNOME desktop environment but who still run
GTK+ applications and want to configure things such as themes, font
antialiasing/hinting, and UI sound effects.

## Build instructions

requirements:

* C++ compiler
* CMake
* X11 headers (`libx11-dev` in Debian)
* GoogleTest (optional, `libgtest-dev` in Debian)

execute build:

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/path ..
make
make install
```

run tests:

```
make test
```

delete all installed files:

```
make uninstall
```

## Documentation

Documentation is available at https://github.com/derat/xsettingsd/wiki.

## Contact

Daniel Erat <dan-xsettingsd@erat.org>
