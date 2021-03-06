ofxSquashBuddies
================

!["Women players at the Harvard Club" by Leslie Jones](https://raw.githubusercontent.com/elliotwoods/ofxSquashBuddies/master/Women%20players%20at%20the%20Harvard%20club.jpg)
_Image courtesy of the Boston Public Library, Leslie Jones Collection._

ofxSquashBuddies is a very low-latency system for transmitting data across a network.

When transmitting a 4MB image over a dedicated GigE connection, we had __< 1 frame latency__ 30fps, at less than 5% CPU on both sender and receiver. 1080p YUY, 720p RGBA or a Kinect V2 RGB+D+Skeleton frame are all approximately 4MB.

Why is it so fast?
------------------

We've made quite a few speed optimisations:

* Send whilst we are compressing (i.e. don't wait for 1 to finish before starting the next one on another thread. This means same amount of computation is required but overall latency is significantly reduced).
* Decompress whilst we are receiving
* We use the [Density](https://github.com/centaurean/density) compression algorithm (approx 2x faster than Snappy at around the same compression ratio).
* We use [ASIO](http://think-async.com/) socket library (as used by GigE Vision libraries).
* We use blocking networking on dedicated threads with condition variable syncing (this gives more performance than async routines).

All of these systems are open source and cross platform.

Requirements
============

* ofxAsio - https://github.com/elliotwoods/ofxAsio
* ofxSquash - https://github.com/elliotwoods/ofxSquash
* ofxAddonLib - https://github.com/elliotwoods/ofxAddonLib (optional for Visual Studio, not required on other platforms)

Usage
=====

Sending
-------

```c++
void ofApp::setup(){
	this->sender.init("127.0.0.1", 4444);
}

void ofApp::update(){
	this->video.update();
	if (this->video.isFrameNew()) {
		this->sender.send(this->video.getPixels());
	}
}
```

Receiver
-------

```c++
void ofApp::setup(){
	this->receiver.init(4444);
}

void ofApp::update(){
	if (this->receiver.isFrameNew()) {
		this->receiver.receive(this->image.getPixels());
	}
}
```

Compatability
=============

Tested with openFrameworks 0.9.1 on OSX and Windows. This addon should work on all platforms where ofxSquash and ofxAsio are supported.

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
