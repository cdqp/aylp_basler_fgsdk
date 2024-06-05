Anyloop plugin for the MicroEnable 5 frame grabber
==================================================

Linux only for now. Only tested on a Mikrotron camera.


libaylp dependency
------------------

Symlink or copy the `libaylp` directory from anyloop to `libaylp`. For example:

```sh
ln -s $HOME/git/anyloop/libaylp libaylp
```


basler-fgsdk5 dependency
------------------------

Download Basler's Framegrabber SDK from
<https://www.baslerweb.com/en/downloads/software-downloads/> and either install
the SDK or symlink or copy it into a directory named `libfgsdk`.

Then, when running anyloop with this plugin, make sure you include
`libfgsdk/lib` in your `LD_LIBRARY_PATH`.


Building
--------

Use meson:

```sh
meson setup build
meson compile -C build
```


