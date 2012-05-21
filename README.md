Resine is a small library for image resampling via Fourier interpolation.

###Building
To build, simply edit the relevant portions of the included makefile and use `make`. Current build options include FFTW and KISS FFT support, multithreading, and the floating point precision. 4, 8, and 16 byte floats are supported throughout the lib, and the relevant precision FFTW will be linked as well.

The resine commandline application depends on a recent version of [libjpeg](http://www.ijg.org/) and [libpng](http://www.libpng.org/) to read/write images.

##License
libresine is licensed under the GNU Lesser General Public License version 2. For further information, including conditions of use when linked with FFTW, see the COPYING file.


###Concept
Resine utilizes a type-II [Discrete Cosine Transform](http://en.wikipedia.org/wiki/Discrete_cosine_transform) to derive a [trigonometric interpolant](http://en.wikipedia.org/wiki/Trigonometric_interpolation) for a set of bitmap data. Since the coefficients given by this transform may be reapplied to the basis functions at an arbitrary scale, they may be treated as resolution-independent data points. Applying an inverse transform at a different scale results in a resampled representation of the input bitmap.
The most straightforward means to achieve this is simply cropping/expanding the coefficient matrix given by typical transform functions, anchored relative to the DC (in fact this is how it is currently implemented).

For some examples, see the project wiki.

###Caveats
* Although this method can produce remarkable results with photographic and natural (band-limited) content, the effect on graphics is often significantly worse and produces noticeable ringing. See the wiki examples page and roadmap item #1.
* Resine is not currently suited for resampling extremely large images. There is plenty of room for improvement in the memory pipeline so this ought to improve in the future. See roadmap item #2.

###Implementation
Currently, libresine supports three backends for computing the DCT: a set of (very slow) native functions, [KISS FFT](http://kissfft.sourceforge.net/), and [FFTW](http://www.fftw.org/). Available interfaces are determined at compile time and toggled in the provided Makefile. For clients, the interface is abstracted behind a single struct parameter.

For client applications, there are two primary data structures to consider: `rsn_info` and `rsn_data`, of which only the former is vital to produce a working application.

rsn_info takes the original and requested image dimensions as well as the number of channels, in addition to a nested configuration struct which may be modified or simply passed from rsn_defaults.

rsn_data is a package created to simplify passing the image state around at various stages of the resampling process. It may be deprecated in favor of simply passing primitive types, see the second roadmap item for info.

To facilitate the quickest codepaths in the default configuration, libresine's internal data structures for bitmap and frequency images mirror those of libpng/jpeg and FFTW, respectively.

As an example, given an image "img" in png_bytepp/JSAMPIMAGE/rsn_image format, dimensions of 512x512x3, and a scaling factor of 2, the least needed to perform resampling is a single line of code:

    rsn_image out = resine((rsn_info){rsn_defaults(),3,512,512,1024,1024},img);

###Roadmap
####libresine
* The current incarnation of the algorithm is its most basic -- it does no special treatment of frequency coefficients such as other forms of windowing or artificial sharpening. The need for experimentation contributes to the next item.
* Much of the library is constructed for easy experimentation with the frequency domain, at a particular cost to resources and with a certain level of disregard for encapsulation (the "greed" setting is symptomatic of this). Non-experimental releases will be able to slim down considerably both in terms of resources and API.
* At present the native transforms are so suboptimal that they are essentially only included for completeness. Potential improvements include threading, Fast DCT, and SIMD optimizations.
* Dimensionality and bitdepth limitations ought to be lifted.

####Build Process
* The project builds with a simple makefile which needs manual configuration. In the future, an autoconf system (or at least configure script) should be put in place.
