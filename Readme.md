ofxSquashBuddies
================

!["Women players at the Harvard Club" by Leslie Jones](https://raw.githubusercontent.com/elliotwoods/ofxSquashBuddies/master/Women%20players%20at%20the%20Harvard%20club.jpg)
_Image courtesy of the Boston Public Library, Leslie Jones Collection._

ofxSquashBuddies is a very low-latency system for transmitting data across a network.

Typical 0.5 frames latency between 2 computers on a GigE connection for a 4MB image (e.g. 1080p YUY or 720p RGBA) stream.

Why is it so fast?
------------------

Due to its threaded stream architecture, it can:

* Send whilst it is compressing
* Decompress whilst it is receiving

This reduces the overall latency in the system since processes can happen simultanaously.

Requirements
============

* ofxAsio - https://github.com/elliotwoods/ofxAsio
* ofxSquash - https://github.com/elliotwoods/ofxSquash
* ofxAddonLib - https://github.com/elliotwoods/ofxAddonLib (optional for Visual Studio, not required on other platforms)

Compatability
=============

Tested with openFrameworks 0.9.1. This addon should work on all platforms where ofxSquash and ofxAsio are supported.

Credits
=======

ofxSquashBuddies is developed by Elliot Woods and Satoru Higa.

License
=======

ofxSquashBuddies
---------
> Copyright (c) 2015, Kimchi and Chips
>
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
