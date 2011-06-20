Resine is a highly experimental C library oriented around image processing in the frequency domain. The intent is to set up a working public repository despite the early and incomplete state of the project. For a detailed list of immediate considerations, see the roadmap.

libresine is licensed under the GNU Lesser General Public License version 2. For further information, including conditions of use when linked with FFTW, see the COPYING file.

###Concept
Resine's algorithm is conceptually similar to the sinc filter, most commonly seen in Lanczos resampling. It utilizes the [real-even Fourier Transform](http://en.wikipedia.org/wiki/Discrete_cosine_transform) to operate on bitmap data in the frequency domain. The simplified idea is that in this state the image has an effectively infinite resolution, as it is composed entirely of coefficients to the cosine function. When the image is normalized and recomposed with new dimensions, the result is a resample of the original bitmap.

For some examples of the algorithm in action, see the project wiki.

While the C implementation is new, the initial concept and a Java proof-of-concept for Resine date to 2007.

###Gotchas
* Although this technique can produce some remarkable results on photographic and natural image content, the effect on graphics and animated content is significantly worse and produces noticeable ringing.
* The native transform functions are extremely slow and only mildly optimized beyond direct, canonical DCT implementations. Patches very welcome.
* Resine is not currently suited for resampling extremely large images. Depending on your system and amount of memory, 32 bit images beyond 100 megapixels may cause deadlocks. There is much room for improvement in the memory pipeline so expect this to improve in the future.

See the roadmap below for a list of currently known issues.

###Implementation
Currently, libresine supports three backends for computing the DCT: a set of (very slow) native functions, [KISS FFT](http://kissfft.sourceforge.net/), and [FFTW](http://www.fftw.org/). Available interfaces are determined at compile time and toggled in the provided Makefile. For client implementations, the interface used is abstracted behind a single integer field in Resine's info struct.

For client applications, there are two primary data structures to consider: `rsn_info` and `rsn_data`, of which only the former is vital to produce a working application.

rsn_info is given the original and requested dimensions as well as the number of color channels being operated on, in addition to a nested configuration struct which may be modified or simply use the configuration provided by rsn_defaults.

rsn_data is a package created to simplify passing the image state around at various stages of the resampling process. It may be deprecated in favor of simply passing primitive types, see the first roadmap item for info.

To facilitate use of the quickest codepaths in the default configuration, libresine's internal data structures for bitmap and frequency images mirror those of libpng/jpeg and FFTW, respectively.

That said, given an image "img" in png_bytepp/JSAMPIMAGE/rsn_image format, dimensions of 512x512x3, and a scaling factor of 2, all that is needed to perform resampling with libresine are the following two lines of code:

    rsn_info info = {rsn_defaults(),3,512,512,1024,1024};
    rsn_image out = resine(info,img);

###Roadmap
####libresine
* The notion of allocation greed and a self-contained image data structure will likely be abandoned in favor of transform algorithms that scale on the fly. Not only will this markedly improve the abysmal performance of the native transforms, it will remove the need for memset, whose behavior is undefined for floating point types.
* At present the native transforms are so unoptimized that they are essentially only included for completeness' sake. Potential improvements include threading, Fast DCT, and SIMD, assembly, and GPGPU optimizations.
* The current incarnation of the algorithm is its most basic -- it lacks any kind of windowing or other interesting modifications to the frequency domain such as artificial high frequency grain.

####Applications
* Several half-baked sample applications didn't make it into this commit, including a barebones profiling application, a program to composite bitmap images into another image's frequency space, and a basic video scaler using libavcodec.

####Build Process
* The project currently builds with a single simplistic makefile which requires manual configuration. In the future, a standard autoconf system should be put in place.
* Resine supports building on multiple platforms, but this hasn't been thoroughly tested and the script may need to be tweaked.

###Building
To build Resine, simply edit the relevant portions of the included makefile. Current build options include whether to build with FFTW and KISS FFT support, multithreading, and the floating point precision. 4, 8, and 16 byte floats are supported throughout the lib, and the correct precision FFTW will be linked as well.

The resine commandline application depends on a recent version of [libjpeg](http://www.ijg.org/) and [libpng](http://www.libpng.org/) to read/write images.