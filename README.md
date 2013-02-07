NetKit
=======

NetKit is a cross platform framework that uses some modern C++ features to make it easier to write modern network aware apps. Primary consideration is code readability and ease of use, not performance or size.

I'm making liberal use of lambdas in an effort to make asynchronous networking elegant, simple to use, and baked in from the beginning.

I'm a big fan of libdispatch, so I'm using that to do eventing. On platforms other than OS X, I'm going to implement the small subset of libdispatch that I use in NetKit.

Finally, NetKit will initially work on OS X and Windows, as those are the only two platforms that I'm interested in. It should be pretty easy to port to any other platforms if there is any interest in doing so.
