# xsettingsd

xsettingsd is a daemon that implements the *XSETTINGS* specification.

It is intended to be small, fast, and minimally dependent on other
libraries. It can serve as an alternative to gnome-settings-daemon for
users who are not using the GNOME desktop environment but who still run
GTK+ applications and want to configure things such as themes, font
antialiasing/hinting, and UI sound effects.

## Installation

Requirements:

* C++ compiler
* [CMake] or [SCons]
* X11 headers (`libx11-dev` in Debian)
* GoogleTest (optional, `libgtest-dev` in Debian)

To compile and install using CMake:

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/path ..
make
make install
```

To run tests:

```
make test
```

To delete all installed files:

```
make uninstall
```

Alternatively, you can compile `xsettingsd` and `dump_xsettings` using SCons:

```
sudo apt-get install scons g++ libstdc++-dev libx11-dev
scons xsettingsd dump_xsettings
```

[CMake]: https://cmake.org/
[SCons]: https://scons.org/

## Configuration

You may wish to dump your existing settings to use as a starting point. To do
this, check that `gnome-settings-daemon` (or another program that implements
XSETTINGS) is running and run the `dump_xsettings` program. Your current
settings should be printed to `stdout` in `xsettingsd`'s configuration format.
To use this as your initial configuration, redirect the output of the program:

```
dump_xsettings >~/.xsettingsd
```

At startup, xsettingsd reads its configuration from `$HOME/.xsettingsd` by
default. If it encounters any errors, it exits; otherwise it becomes the
XSETTINGS manager on all screens.

After modifying the `.xsettingsd` file, you can trigger a configuration reload
by sending a `HUP` signal to xsettingsd, e.g.

```
killall -HUP xsettingsd
```

If there in a problem with the new configuration, xsettingsd will continue using
the previous version.

The format for the configuration file is simple. Setting names and their
corresponding values are whitespace-separated, with at most one setting per
line.

*   Integer values appear as bare decimal numbers.
*   String values are double-quoted.
*   Color values are `(R, G, B, A)` or `(R, G, B)` tuples, where each value
    ranges between 0 and 65535 (omitting the alpha value results in full
    opacity).
*   Full-line comments can be started with a `#` character.

Here is an example `.xsettingsd` file:

```
# Configure our fonts.
Xft/Antialias 1
Xft/HintStyle "hintfull"
Xft/Hinting 1
Xft/RGBA "none"
Xft/lcdfilter "none"

# Create a color setting (haven't seen these used anywhere, though).
MyFavoriteColor (33667, 48059, 38036, 32768)  # not really my favorite!

EscapedQuote "here's how to put a \" in a string!"
```

## Running

If you are using a `~/.xsession` file to control which programs are executed
when you start an X session, you can add the following near the top of it:

```
/path/to/xsettingsd &
```

## Settings

The following table lists some of things that can be configured in GTK+
applications using XSETTINGS. The **Details** column lists the corresponding
properties in the [GtkSettings class].

| **Name** | **Type** | **Description** | **Values** | **Details** |
|:---------|:---------|:----------------|:-----------|:------------|
| `Gtk/CursorThemeName` | string | cursor theme (see also [#23](https://github.com/derat/xsettingsd/issues/23)) | e.g. subdirectories of `/usr/share/icons` | [`gtk-cursor-theme-name`](https://docs.gtk.org/gtk4/property.Settings.gtk-cursor-theme-name.html) |
| `Gtk/DecorationLayout` | string | layout of gtk titlebar buttons | `:`=no titlebar buttons, `:minimize,maximize,close`=buttons right, see gtk documentation -> | [`gtk-decoration-layout`](https://docs.gtk.org/gtk4/property.Settings.gtk-decoration-layout.html) |
| `Net/CursorBlink` | integer | whether the (text editing) cursor should blink | `0`=no, `1`=yes | [`gtk-cursor-blink`](https://docs.gtk.org/gtk4/property.Settings.gtk-cursor-blink.html) |
| `Net/CursorBlinkTime` | integer | length of the cursor blink cycle, in milleseconds | `1200` (default), `500`, etc. | [`gtk-cursor-blink-time`](https://docs.gtk.org/gtk4/property.Settings.gtk-cursor-blink-time.html) |
| `Net/DndDragThreshold` | integer | number of pixels the cursor can move before dragging | `8` (default), `0`, etc. | [`gtk-dnd-drag-threshold`](https://docs.gtk.org/gtk4/property.Settings.gtk-dnd-drag-threshold.html) |
| `Net/DoubleClickDistance ` | integer | maximum distance allowed between two clicks for them to be considered a double click (in pixels) | `5` (default), `20`, etc. | [`gtk-double-click-distance`](https://docs.gtk.org/gtk4/property.Settings.gtk-double-click-distance.html) |
| `Net/DoubleClickTime` | integer | maximum time allowed between two clicks for them to be considered a double click (in milliseconds) | `250` (default), `500`, etc. | [`gtk-double-click-time`](https://docs.gtk.org/gtk4/property.Settings.gtk-double-click-time.html) |
| `Net/EnableEventSounds` | integer | whether to play event sounds | `0`=no, `1`=yes | [`gtk-enable-event-sounds`](https://docs.gtk.org/gtk4/property.Settings.gtk-enable-event-sounds.html) |
| `Net/EnableInputFeedbackSounds` | integer  | if event sounds are enabled, should they be played in response to input? | `0`=no, `1`=yes | [`gtk-enable-input-feedback-sounds`](https://docs.gtk.org/gtk4/property.Settings.gtk-enable-input-feedback-sounds.html) |
| `Net/IconThemeName` | string | icon theme | e.g. subdirectories of `/usr/share/icons` | [`gtk-icon-theme-name`](https://docs.gtk.org/gtk4/property.Settings.gtk-icon-theme-name.html) |
| `Net/SoundThemeName` | string | sound theme | ? | [`gtk-sound-theme-name`](https://docs.gtk.org/gtk4/property.Settings.gtk-sound-theme-name.html) |
| `Net/ThemeName` | string | widget theme | e.g. subdirectories of `/usr/share/themes` | [`gtk-theme-name`](https://docs.gtk.org/gtk4/property.Settings.gtk-theme-name.html) |
| `Xft/Antialias` | integer | text antialiasing | `0`=no, `1`=yes, `-1`=default | [`gtk-xft-antialias`](https://docs.gtk.org/gtk4/property.Settings.gtk-xft-antialias.html) |
| `Xft/DPI` | integer  | display DPI | `1024*dots/inch`, `-1`=default | [`gtk-xft-dpi`](https://docs.gtk.org/gtk4/property.Settings.gtk-xft-dpi.html) |
| `Xft/HintStyle` | string | text hinting style | `hintnone`, `hintslight`, `hintmedium`, `hintfull` | [`gtk-xft-hintstyle`](https://docs.gtk.org/gtk4/property.Settings.gtk-xft-hintstyle.html) |
| `Xft/Hinting` | integer | text hinting | `0`=no, `1`=yes, `-1`=default | [`gtk-xft-hinting`](https://docs.gtk.org/gtk4/property.Settings.gtk-xft-hinting.html) |
| `Xft/RGBA` | string | text subpixel rendering | `none`, `rgb`, `bgr`, `vrgb`, `vbgr` | [`gtk-xft-rgba`](https://docs.gtk.org/gtk4/property.Settings.gtk-xft-rgba.html) |

<https://www.freedesktop.org/wiki/Specifications/XSettingsRegistry/> also lists
standardized settings.

[GtkSettings class]: https://developer-old.gnome.org/gtk3/stable/GtkSettings.html

## Other Notes

Some applications (e.g. Firefox 3) don't seem to use XSETTINGS to control text
rendering. You can try additionally putting something like the following in
`~/.fonts.conf`:

```
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<fontconfig>
  <match target="font" >
    <edit mode="assign" name="rgba">
      <const>none</const>
    </edit>
  </match>
  <match target="font" >
    <edit mode="assign" name="hinting">
      <bool>true</bool>
    </edit>
  </match>
  <match target="font" >
    <edit mode="assign" name="hintstyle">
      <const>hintfull</const>
    </edit>
  </match>
  <match target="font" >
    <edit mode="assign" name="antialias">
      <bool>true</bool>
    </edit>
  </match>
</fontconfig>
```

and in `~/.Xresources` or `~/.Xdefaults`:

```
Xft.antialias:          1
Xft.hinting:            1
Xft.rgba:               none
Xft.hintstyle:          hintfull
```

(Tweak the values to your own preferences, of course.)
