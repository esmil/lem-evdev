lem-evdev
=========


About
-----

lem-evdev is a library for the [Lua Event Machine][lem]
which allows you to get events from Linux event devices without blocking
the main loop.

The Linux event devices are usually located under `/dev/input/event<n>`,
and can be used to get raw input events from keyboards, mice, joysticks,
as well the lid switch, special keys and accelerometer on many laptops.

[lem]: https://github.com/esmil/lem


Installation
------------

Get the source and do

    make
    make install

This installs the library under `/usr/local/lib/lua/5.1/`.
Use

    make PREFIX=<your custom path> install

to install to `<your custom path>/lib/lua/5.1/`.


Usage
-----

Import the module using something like

    local evdev = require 'lem.evdev'

This sets `evdev` to a table with the following functions.

* __evdev.open(path)__

  Opens the event device at the given `path` and returns a new device object.

  Returns `nil` followed by an error message on error.

The metatable of those device objects can be found under __evdev.Device__.

The following methods are available on devices.

* __dev:close()__

  Closes the device. If the device is busy, this also interrupts the IO
  action on the device.

  Returns `true` on succes or otherwise `nil` followed by an error message.
  If the device is already closed the error message will be `'already closed'`.

* __dev:interrupt()__

  Interrupt any coroutine waiting for IO on the device.

  Returns `true` on success and `nil, 'not busy'` if no coroutine is for IO
  on the device.

* __dev:get()__

  Get the next event from the device. On success this method will
  return 4 values:

    - type: a string describing the event type.
    - code: the event code.
    - value: a value associated to the event.
    - timestamp: a unix timestamp in seconds, but with microsecond resolution.

  If there are no unread events the current coroutine will be suspended
  until there is.

  Returns `nil` followed by an error message on error.
  If another coroutine is waiting for IO on the device the error message
  will be `'busy'`.
  If the device was interrupted (eg. by another coroutine calling
  `dev:interrupt()`, or `dev:close()`) the error message will be
  `'interrupted'`.
  If the device was closed the error message will be `'closed'`.


License
-------

lem-evdev is free software. It is distributed under the terms of the
[GNU General Public License][gpl].

[gpl]: http://www.fsf.org/licensing/licenses/gpl.html


Contact
-------

Please send bug reports, patches, feature requests, praise and general gossip
to me, Emil Renner Berthing <esmil@mailme.dk>.
