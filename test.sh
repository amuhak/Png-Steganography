make clean
make -j4
bin/encode ./image.png "A very very long string that I am using for profile guided optimization. This should tell the compiler what an average program run is doing, and fingers crossed, if everything goes well, the compiler will be able to make the final binary faster than it would ever be able to without all of this extra information. At worst? It should be about the same, so there is no real reason not to do this. Compile time you say. Well, that’s valid, you have to compile twice, so it takes 2x as long. I guess if you are in a hurry to compile this is a bad idea. 575 characters should do." ./out.png
bin/decode ./out.png
