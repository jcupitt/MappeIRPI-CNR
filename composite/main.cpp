/* compile with
 *
 * 	g++ -g -Wall main.cpp `pkg-config vips-cpp --cflags --libs`
 *
 */

#include <vips/vips8>

using namespace vips;

int
main(int argc, char **argv)
{
	if(VIPS_INIT(argv[0]))
		vips_error_exit(NULL);

	/* Disable te libvips cache -- it won't help and will just burn
	 * memory.
	 */
	vips_cache_set_max(0);

	VImage background = vips::VImage::new_from_file("background.tif"); 
	VImage overlay = vips::VImage::new_from_file("overlay.png"); 
	VImage marker = vips::VImage::new_from_file("marker.png"); 

	for(int i = 1; i < argc; i++) {
		VImage data = vips::VImage::new_from_file(argv[i], 
			VImage::option()->set("access", "sequential"));
		data = data.resize(3, 
			VImage::option()->set("kernel", VIPS_KERNEL_LINEAR));
		VImage image = background.composite2(data, VIPS_BLEND_MODE_OVER);
		image = image.composite2(overlay, VIPS_BLEND_MODE_OVER);

		/* we must make a unique string each time, or libvips will
		 * cache the result for us
		 */
		char str[256];
		vips_snprintf(str, 256, "Wed 26 Sep 21:42:47 BST 2018 - %d", i);
		VImage text = VImage::text(str,
			VImage::option()
				->set("height", 25)
                                ->set("width", 945)
				->set("font", "SFmono")
				->set("fontfile", "SFMono-Bold.otf"));

		/* The text image will be a single band. We need an image with
		 * an alpha.
		 *
		 * Make a constant image the same size as the text, then
		 * attach the text image as an alpha channel. We have to tag
		 * the image as black and white (b-w).
		 */
		VImage colour = text.new_from_image(255);
		text = colour.bandjoin(text);
		text = text.copy(VImage::option()
			->set("interpretation", "b-w"));
		image = image.composite2(text, VIPS_BLEND_MODE_OVER,
			VImage::option()
				->set("x", 10)
				->set("y", 10));

		int marker_x = 100;
		int marker_y = 200;
		image = image.composite(marker, VIPS_BLEND_MODE_OVER,
			VImage::option()
				->set("x", marker_x - marker.width() / 2)
				->set("y", marker_y - marker.height()));

		image.write_to_file("x.jpg");
	}

	return 0;
}


