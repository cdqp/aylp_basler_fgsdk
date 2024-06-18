anyloop plugin for the MicroEnable 5 frame grabber
==================================================

Linux only for now. Only tested on a Mikrotron camera.


Installing
----------

### libaylp dependency

Symlink or copy the `libaylp` directory from anyloop to `libaylp`. For example:

```sh
ln -s $HOME/git/anyloop/libaylp libaylp
```

### basler-fgsdk5 dependency

Download Basler's Framegrabber SDK from
<https://www.baslerweb.com/en/downloads/software-downloads/> and either install
the SDK or symlink or copy it into a directory named `libfgsdk`.

Then, when running anyloop with this plugin, make sure you include
`libfgsdk/lib` in your `LD_LIBRARY_PATH`.

### building

Use meson:

```sh
meson setup build
meson compile -C build
```


aylp_basler_fgsdk.so
--------------------

Types and units: `[T_ANY, U_ANY] -> [T_MATRIX_UCHAR, U_COUNTS]`.

This device uses the [Basler Framegrabber SDK][fgsdk] to acquire images from a
Basler framegrabber. It has been tested only on the [microEnable 5 marathon
ACL][me5].

For an example configuration, which will grab frames and dump them over UDP, see
[conf_test.json](contrib/conf_test.json).

### Parameters

- `width` (integer) (required)
  - The width of the image that the camera should be capturing.
- `height` (integer) (required)
  - The height of the image that the camera should be capturing.
- `pitch` (float) (optional)
  - Distance between the camera's pixels, for updating the corresponding
    parameter in anyloop's pipeline. Defaults to 0.0.
- `fast` (boolean) (optional)
  - If this option is enabled, this device will stop calling the Basler SDK
    after initialization, and assume that the image pointer that we *would* have
    gotten from calls to `Fg_getImageEx` followed by `Fg_getImagePtrEx` will
    remain the same for every iteration of the loop. I have observed that the
    recommended SDK calls can take up to ~10 ms to run, and with this option
    enabled the latency goes down to effectively zero. The downside is that data
    corruption is possible if the framegrabber driver for some reason decides to
    change where in virtual memory the image data is stored. Defaults to false.



[fgsdk]: https://docs.baslerweb.com/frame-grabbers/framegrabber-sdk
[me5]: https://www.baslerweb.com/en/shop/microenable-5-marathon-acl

