#ifndef IMAGE_PAINTER_H__
#define IMAGE_PAINTER_H__

#include <boost/concept_check.hpp>
#include <boost/lexical_cast.hpp>

#include <gui/ContextSettings.h>
#include <gui/GlContext.h>
#include <gui/OpenGl.h>
#include <gui/IsImage.h>
#include <gui/Painter.h>
#include <gui/TextPainter.h>
#include <gui/Texture.h>
#include <util/Logger.h>
#include <util/point.hpp>
#include <util/rect.hpp>

static logger::LogChannel imagepainterlog("imagepainterlog", "[ImagePainter] ");

namespace gui {

template <typename Pointer>
struct init_ptr {

	// default for smart pointers: do nothing
	void operator()(Pointer&) {}
};

template <typename Pointer>
struct init_ptr<Pointer*> {

	// real pointers will be set to 0
	void operator()(Pointer*& p) {

		p = 0;
	}
};

/**
 * Class ImagePainter.
 *
 * Provides a painter for a generic Image. The Image has to provide:
 *
 *   • Image::value_type                  (the pixel type)
 *   • const value_type& operator()(x, y) (to access the pixels)
 *   • unsigned int width() and height()
 */
template <typename Image, typename Pointer = boost::shared_ptr<Image> >
class ImagePainter :
		public gui::Painter {

	BOOST_CONCEPT_ASSERT((IsImage<Image>));

	typedef typename Image::value_type value_type;
	typedef Pointer                    pointer_type;

public:


	/**
	 * Constructs a new image painter with the given name.
	 *
	 * @param reloadThread true, if this ImagePainter should use a separate
	 *                     texture reload thread.
	 */
	ImagePainter(bool reloadThread = false);

	/**
	 * Destroys texture and closes reload thread.
	 */
	~ImagePainter();

	/**
	 * Set the image to show.
	 *
	 * @param A pointer to the image to show.
	 * @param An optional mutex to guard data read operations to the image.
	 */
	void setImage(pointer_type image, boost::shared_mutex* imageMutex = 0);

	/**
	 * Enable image normalization. Currently, this only works for intensity
	 * images.
	 */
	void setNormalize(bool normalize = true) { _normalize = normalize; }

	/**
	 * Get a pointer to the image this painter is showing.
	 *
	 * @return Pointer to the image that this painter is showing.
	 */
	const pointer_type& getImage() { return _image; }

	/**
	 * Overwritten from painter.
	 */
	virtual bool draw(
		const util::rect<double>&  roi,
		const util::point<double>& resolution);

	/**
	 * Set a color to colorize the image.
	 */
	void setColor(float red, float green, float blue);

	/**
	 * Show the image transparent in the dark areas.
	 */
	void setTransparent(bool transparent);

	/**
	 * Indicate that the image has changed.
	 */
	void reload();

private:

	/**
	 * Reloading of the image.
	 */
	void update();

	/**
	 * Normalize and re-load image for intensity images.
	 */
	void loadNormalized(const boost::true_type& arithmetic);

	/**
	 * Normalize and re-load image for multi-channel images.
	 */
	void loadNormalized(const boost::false_type& arithmetic);

	/**
	 * Re-load image for intensity images.
	 */
	void load(const boost::true_type& arithmetic);

	/**
	 * Re-load image for multi-channel images.
	 */
	void load(const boost::false_type& arithmetic);

	/**
	 * Get the RGB values for hsv.
	 */
	void hsvToRgb(double h, double s, double v, unsigned char& r, unsigned char& g, unsigned char& b);

	/**
	 * Entry point for the texture reload thread.
	 */
	void reloadThread();

	/**
	 * Reloads a texture by creating a temporary texture and replacing it with
	 * the current one.
	 */
	void reloadTexture();

	// the image to observe (shared ownership)
	pointer_type _image;

	// normalize the image?
	bool _normalize;

	// a type to initialize the pointer
	init_ptr<pointer_type> _init_ptr;

	// the texture of the image to show (exclusivey owned)
	Texture* _imageTexture;

	// set to true if this image painter has a worker thread
	bool _hasReloadThread;

	// a worker thread to update the texture in the background
	boost::thread _reloadThread;

	// mutex to guard the image (not ours, has to be guaranteed to survive this
	// object)
	boost::shared_mutex* _imageMutex;

	// indicate the need to reload the texture to the texture thread
	bool _needReload;

	// color the image
	float _red, _green, _blue;

	// show the image transparent (dark = transparent)
	bool _transparent;
};

/******************
 * IMPLEMENTATION *
 ******************/

template <typename Image, typename Pointer>
ImagePainter<Image, Pointer>::ImagePainter(bool reloadThread) :
	_normalize(false),
	_imageTexture(0),
	_hasReloadThread(reloadThread),
	_needReload(true),
	_red(1.0),
	_green(1.0),
	_blue(1.0),
	_transparent(false) {

	LOG_ALL(imagepainterlog) << "initializing..." << std::endl;
	_init_ptr(_image);

	// launch a new worker thread
	if (_hasReloadThread) {

		LOG_ALL(imagepainterlog) << "launching texture worker thread..." << std::endl;

		_reloadThread = boost::thread(&ImagePainter<Image, Pointer>::reloadThread, this);
	}

	LOG_ALL(imagepainterlog) << "done initializing" << std::endl;
}


template <typename Image, typename Pointer>
ImagePainter<Image, Pointer>::~ImagePainter() {

	LOG_ALL(imagepainterlog) << "destroying..." << std::endl;

	if (_hasReloadThread) {

		_hasReloadThread = false;

		LOG_ALL(imagepainterlog) << "waiting for reload thread to finish..." << std::endl;
		_reloadThread.join();
	}

	LOG_ALL(imagepainterlog) << "deleting texture" << std::endl;

	if (_imageTexture != 0)
		delete _imageTexture;

	LOG_ALL(imagepainterlog) << "destroyed" << std::endl;
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::setImage(pointer_type image, boost::shared_mutex* imageMutex) {

	_image      = image;
	_imageMutex = imageMutex;

	LOG_ALL(imagepainterlog) << "size of image is "
	                         << _image->width()  << "x"
	                         << _image->height() << std::endl;

	setSize(0.0, 0.0, _image->width(), _image->height());

	update();
}

template <typename Image, typename Pointer>
bool
ImagePainter<Image, Pointer>::draw(
	const util::rect<double>&  roi,
	const util::point<double>& resolution) {

	// wait for image
	if (!_image) {

		LOG_ALL(imagepainterlog) << "have no image, yet" << std::endl;
		return false;
	}

	// wait for content
	if (_image->width() == 0 || _image->height() == 0) {

		LOG_ALL(imagepainterlog) << "image has zero size, yet" << std::endl;
		return false;
	}

	if (_hasReloadThread) {

		// come back later if the reload thread is done
		if (!_imageTexture) {

			// ensure that OpenGl operations are save
			OpenGl::Guard guard;

			// draw a dummy rectangle
			glColor3f(0.5, 0.7, 1.0);
			glBegin(GL_QUADS);
			glVertex2d(getSize().minX, getSize().minY); 
			glVertex2d(getSize().maxX, getSize().minY); 
			glVertex2d(getSize().maxX, getSize().maxY); 
			glVertex2d(getSize().minX, getSize().maxY); 
			glEnd();

			return false;
		}
	}

	LOG_ALL(imagepainterlog) << "drawing..." << std::endl;

	// ensure that OpenGl operations are save
	OpenGl::Guard guard;

	// draw a rectangle
	GLdouble  width  = _imageTexture->width();
	GLdouble  height = _imageTexture->height();

	glEnable(GL_TEXTURE_2D);

	_imageTexture->bind();

	glColor3f(_red, _green, _blue);
	if (_transparent) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	}
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 1.0); glVertex2d(0.0,   height); 
	glTexCoord2d(1.0, 1.0); glVertex2d(width, height); 
	glTexCoord2d(1.0, 0.0); glVertex2d(width, 0.0); 
	glTexCoord2d(0.0, 0.0); glVertex2d(0.0,   0.0); 
	glEnd();
	if (_transparent)
		glDisable(GL_BLEND);

	// a pixel is large enough to be written in
	if (resolution.x > 30) {

		// for every pixel
		for (int x = 0; x < _image->width(); x++) {
			for (int y = 0; y < _image->height(); y++) {

				// if visible
				if (util::rect<double>(x, y, x+1, y+1).intersects(roi)) {

					TextPainter valuePainter(boost::lexical_cast<std::string>((*_image)(x, y)));
					valuePainter.setTextSize(0.1);
					valuePainter.setTextColor((*_image)(x, y), 0.5 - (*_image)(x, y)/2.0, 0.5 + (*_image)(x, y)/2.0, (resolution.x - 30)/100);

					TextPainter positionPainter(boost::lexical_cast<std::string>(x) + ", " + boost::lexical_cast<std::string>(y));
					positionPainter.setTextSize(0.1);
					positionPainter.setTextColor((*_image)(x, y), 0.5 - (*_image)(x, y)/2.0, 0.5 + (*_image)(x, y)/2.0, (resolution.x - 30)/100);

					LOG_ALL(imagepainterlog) << "drawing text with roi "
					                      << (roi - util::point<double>(x, y)) << " and resolution "
					                      << resolution << std::endl;

					glTranslatef( x,  y, 0.0f);
					valuePainter.draw(roi - util::point<double>(x, y), resolution);
					glTranslatef( 0.0f,  0.1f, 0.0f);
					positionPainter.draw(roi - util::point<double>(x, y), resolution);
					glTranslatef(-x, -y - 0.1f, 0.0f);
				}
			}
		}
	}

	return false;
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::update() {

	if (!_hasReloadThread)
		reloadTexture();
	else
		_needReload = true;
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::reloadThread() {

	LOG_ALL(imagepainterlog) << "reload thread started" << std::endl;

	try {

		while (_hasReloadThread) {

			while (_needReload) {

				// set to false to see whether another thread changed it
				_needReload= false;

				LOG_ALL(imagepainterlog) << "texture needs to be reloaded" << std::endl;

				if (_imageMutex) {

					// lock access to the image
					boost::shared_lock<boost::shared_mutex> lockImage(*_imageMutex);

					reloadTexture();

				} else {

					reloadTexture();
				}

				// if _needReload is still false that's nice -- otherwise some other
				// thread changed it and we repeat the update...
			}

			// TODO: use barriers for that...
			usleep(1000);
		}

	} catch (boost::exception& e) {

		std::cerr << "details: " << std::endl
		          << boost::diagnostic_information(e)
		          << std::endl;

		throw &e;
	}
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::reloadTexture() {

	if (!_image || _image->width()*_image->height() == 0) {

		LOG_ALL(imagepainterlog) << "image was not initialised..." << std::endl;
		return;
	}

	// ensure that OpenGl operations are save
	OpenGl::Guard guard;

	if (!_imageTexture) {

		LOG_ALL(imagepainterlog) << "creating new texture "
		                         << _image->width() << "x" << _image->height() << std::endl;

		// crate new texture
		_imageTexture = new Texture(_image->width(), _image->height(), GL_RGBA);

	} else {

		if (_image->width()  != _imageTexture->width() ||
		    _image->height() != _imageTexture->height()) {

			LOG_ALL(imagepainterlog) << "resizing texture to "
			                         << _image->width() << "x" << _image->height() << std::endl;

			// resize texture
			_imageTexture->resize(_image->width(), _image->height());
		}
	}

	LOG_ALL(imagepainterlog) << "loading data" << std::endl;

	// load pixel data
	if (_normalize) {

		loadNormalized(boost::is_arithmetic<value_type>());

	} else {

		load(boost::is_arithmetic<value_type>());
	}

	// set reported size
	setSize(0.0, 0.0, _image->width(), _image->height());

	LOG_ALL(imagepainterlog) << "done (re)loading texture" << std::endl;
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::loadNormalized(const boost::true_type&) {

	bool       valid = false;
	value_type min = 0.0f;
	value_type max = 1.0f;

	// find min and max of image
	for (typename Image::iterator i = _image->begin(); i != _image->end(); i++) {

		if (valid) {

			min = std::min(min, *i);
			max = std::max(max, *i);

		} else {

			min = *i;
			max = *i;
			valid = true;
		}
	}

	// scale and shift to [0..1]
	_imageTexture->loadData(&(*_image->begin()), 1.0f/(max - min), -min/(max - min));
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::loadNormalized(const boost::false_type& arithmetic) {

	// for non-intensity images we leave it like that for the moment...
	_imageTexture->loadData(&(*_image->begin()));
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::load(const boost::true_type&) {

	bool       valid = false;
	value_type min = 0.0f;
	value_type max = 1.0f;

	// find min and max of image
	for (typename Image::iterator i = _image->begin(); i != _image->end(); i++) {

		if (valid) {

			min = std::min(min, *i);
			max = std::max(max, *i);

		} else {

			min = *i;
			max = *i;
			valid = true;
		}
	}

	if (max > 1.0) {

		// consider this image as a color index image
		std::vector<boost::array<unsigned char, 4> > colorImage;
		colorImage.reserve(_image->size());

		boost::array<unsigned char, 4> pixel;
		pixel[3] = 255;

		for (typename Image::iterator i = _image->begin(); i != _image->end(); i++) {

			// fire
			//float h = ((*i) - min)/(max - min);//fmod(static_cast<float>(*i)*M_PI, 1.0);

			// intensities above one are handled as color indices
			if (*i >= 1.0) {

				float h = fmod(static_cast<float>(*i)*M_PI, 1.0);
				float s = 0.5 + fmod(static_cast<float>(*i)*M_PI*2, 0.5);
				float v = (*i == 0 ? 0.0 : 0.75 + fmod(static_cast<float>(*i)*M_PI*3, 0.25));
				hsvToRgb(h, s, v, pixel[0], pixel[1], pixel[2]);

			// intensities below 1 are handled as grayscale
			} else {

				pixel[0] = (*i)*255.0;
				pixel[1] = (*i)*255.0;
				pixel[2] = (*i)*255.0;
			}
			colorImage.push_back(pixel);
		}

		_imageTexture->loadData(&(*colorImage.begin()));

	} else {

		// non-intensity images are just loaded
		_imageTexture->loadData(&(*_image->begin()));
	}
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::load(const boost::false_type& arithmetic) {

	// non-intensity images are just loaded
	_imageTexture->loadData(&(*_image->begin()));
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::hsvToRgb(double h, double s, double v, unsigned char& r, unsigned char& g, unsigned char& b) {

	if(s < 0) s = 0;
	if(s > 1) s = 1;
	if(v < 0) v = 0;
	if(v > 1) v = 1;

	if(s == 0) {
		r = (unsigned char)255.0*v;
		g = (unsigned char)255.0*v;
		b = (unsigned char)255.0*v;
	}

	h = fmod(h, 1.0); // want h to be in 0..1

	unsigned int i = h*6;
	double f = (h*6) - i;
	double p = v*(1.0f - s); 
	double q = v*(1.0f - s*f);
	double t = v*(1.0f - s*(1.0f-f));
	switch(i%6) {
	case 0:
		r = (unsigned char)255.0*v;
		g = (unsigned char)255.0*t;
		b = (unsigned char)255.0*p;
		return;
	case 1:
		r = (unsigned char)255.0*q;
		g = (unsigned char)255.0*v;
		b = (unsigned char)255.0*p;
		return;
	case 2:
		r = (unsigned char)255.0*p;
		g = (unsigned char)255.0*v;
		b = (unsigned char)255.0*t;
		return;
	case 3:
		r = (unsigned char)255.0*p;
		g = (unsigned char)255.0*q;
		b = (unsigned char)255.0*v;
		return;
	case 4:
		r = (unsigned char)255.0*t;
		g = (unsigned char)255.0*p;
		b = (unsigned char)255.0*v;
		return;
	case 5:
		r = (unsigned char)255.0*v;
		g = (unsigned char)255.0*p;
		b = (unsigned char)255.0*q;
		return;
	}
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::setColor(float red, float green, float blue) {

	_red = red;
	_green = green;
	_blue = blue;

	update();
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::setTransparent(bool transparent) {

	_transparent = transparent;

	update();
}

template <typename Image, typename Pointer>
void
ImagePainter<Image, Pointer>::reload() {

	update();
}

} // namespace gui

#endif // IMAGE_PAINTER_H__

