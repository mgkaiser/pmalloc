# PMalloc

A portable implementation of malloc that can be used as "farmalloc"

As implemented you can up to 4GB of heap, but any individual allocation may be no more than 64k. This works well with the "large" memory model as implemented on Calypsi C on a 65816 processor.
