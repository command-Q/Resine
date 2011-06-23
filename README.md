Resine is an experimental C library oriented around image resampling in the frequency domain. The project is quite young; for a list of immediate considerations, see the roadmap.

libresine is licensed under the GNU Lesser General Public License version 2. For further information, including conditions of use when linked with FFTW, see the COPYING file.

###Concept
Resine's algorithm utilizes a type-II [Discrete Cosine Transform](http://en.wikipedia.org/wiki/Discrete_cosine_transform) to operate on bitmap data in the frequency domain. Since the coefficients given by this transform may be reapplied to the basis functions at an arbitrary scale, they may be treated as resolution-independent data points. Applying an inverse transform at a different scale results in a resampled representation of the input bitmap.
In more familiar spectral processing terms, every image can be considered a low-pass rendering of a theoretical infinitely-detailed source. Resine would then merely produce another view of this rendering with respect to the hypothetical source.
This can be implemented with existing transform functions by simply cropping/expanding the normalized frequency image relative to the DC (in fact this is how it is currently implemented).

For some examples of the algorithm in action, see the project wiki.

While the C implementation is new, the initial concept and a Java proof-of-concept for Resine date to 2007.

###Caveats
* Although this technique with no modifications can produce remarkable results on photographic and natural image content, the effect on graphics and animated content may be significantly worse and produce noticeable ringing. See the wiki examples page and libresine roadmap #1.
* Resine is not currently suited for resampling extremely large images. There is much room for improvement in the memory pipeline so expect this to improve in the future. See also libresine roadmap #2.

###Implementation
Currently, libresine supports three backends for computing the DCT: a set of (very slow) native functions, [KISS FFT](http://kissfft.sourceforge.net/), and [FFTW](http://www.fftw.org/). Available interfaces are determined at compile time and toggled in the provided Makefile. For client implementations, the interface used is abstracted behind a single integer field in Resine's info struct.

For client applications, there are two primary data structures to consider: `rsn_info` and `rsn_data`, of which only the former is vital to produce a working application.

rsn_info is given the original and requested dimensions as well as the number of color channels being operated on, in addition to a nested configuration struct which may be modified or simply use the configuration provided by rsn_defaults.

rsn_data is a package created to simplify passing the image state around at various stages of the resampling process. It may be deprecated in favor of simply passing primitive types, see the first roadmap item for info.

To facilitate use of the quickest codepaths in the default configuration, libresine's internal data structures for bitmap and frequency images mirror those of libpng/jpeg and FFTW, respectively.

That said, given an image "img" in png_bytepp/JSAMPIMAGE/rsn_image format, dimensions of 512x512x3, and a scaling factor of 2, all that is needed to perform resampling with libresine are the following two lines of code (only one if the info struct is a compound literal):

    rsn_info info = {rsn_defaults(),3,512,512,1024,1024};
    rsn_image out = resine(info,img);

###Roadmap
####libresine
* The current incarnation of the algorithm is its most basic -- it lacks any kind of windowing or other interesting modifications to the frequency domain such as artificial high frequency grain. The need for experimentation in this area contributes strongly to the next item.
* Much of the architecture at this time is focused around making it easy to experiment in the frequency domain, at a particular cost to resource usage and with a certain level of disregard for encapsulation. The "greed" management is a specific symptoms of this. Non-experimental releases will be able to slim down considerably both in terms of resources and API.
* At present the native transforms are so unoptimized that they are essentially only included for completeness' sake. Potential improvements include threading, Fast DCT, and SIMD, assembly, and GPGPU optimizations.
* Dimensionality and bitdepth limitations (indeed the only things tying the library to image data) are imposed for simplicity and will be lifted when appropriate.

####Applications
* Several half-baked sample applications didn't make it into this commit, including a barebones profiling application, a program to composite bitmap images into another image's frequency space, and a basic video scaler using libavcodec.

####Build Process
* The project currently builds with a single simplistic makefile which requires manual configuration. In the future, a standard autoconf system should be put in place.
* Resine supports building on multiple platforms, but this hasn't been thoroughly tested and the script may need to be tweaked.

###Building
To build Resine, simply edit the relevant portions of the included makefile. Current build options include whether to build with FFTW and KISS FFT support, multithreading, and the floating point precision. 4, 8, and 16 byte floats are supported throughout the lib, and the correct precision FFTW will be linked as well.

The resine commandline application depends on a recent version of [libjpeg](http://www.ijg.org/) and [libpng](http://www.libpng.org/) to read/write images.