I occasionally find myself surfing image boards, including the chans. There is not much to say about this, except when it comes to animated gifs, which constantly are destroyed when scaled down for icons. This issue comes from the fact that gifs don't have to repaint the whole image each frame. Instead the file can specify only the differences, which is one reason it is so small. When many other libraries scale animated gifs with selectively updated regions, they screw up their calculations and cause abominations to be born. Samples are included for demonstration of incorrect scaling.

This is a tool to scale gifs correctly no matter if they have animation or transparency. The project focuses more on keeping sub-images aligned than on the issue of maintaining the pixel data in the scaled image, so the first scaling algorithm will be very simple.

I thank the following sites for the information needed to understand the gif format:
  http://www.matthewflickinger.com/lab/whatsinagif/bits_and_bytes.asp
  http://en.wikipedia.org/wiki/Graphics_Interchange_Format
