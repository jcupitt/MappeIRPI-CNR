/* compile with
 *
 * 	g++ -g -Wall main.cpp `pkg-config vips-cpp --cflags --libs`
 *
 *
 * composite1 takes 14.6s on the benchmark script on my laptop, this one is
 * about 8.2s.
 */

#include <vips/vips8>

using namespace vips;

/* Draw a one-band mask on an image in a constant colour. This is faster than
 * composite, especially for large images, since it only processes pixels in
 * the overlap area.
 *
 * This won't work if the overlay is partly outside the background. It needs
 * to have a little cropping code added for that.
 */
static VImage
draw_overlay(VImage background, VImage overlay, 
	int x, int y, std::vector<double> ink = {255})
{
	// cut out the part of the background we modify
	VImage tile = background.extract_area(x, y, 
		overlay.width(), overlay.height());

	// Make a constant image the size of the overlay area containing the
	// ink
	VImage ink_image = overlay.new_from_image(ink);

	// use the overlay as a mask to blend smoothly between the background
	// and the ink
	tile = overlay.ifthenelse(ink, tile, 
		VImage::option()
			->set("blend", TRUE));

	// and insert the modified tile back into the image
	return background.insert(tile, x, y);
}

/* Composite a small image into a much larger one.
 */
static VImage
composite_glyph(VImage background, VImage glyph, int x, int y)
{
	// cut out the part of the background we modify
	VImage tile = background.extract_area(x, y, 
		glyph.width(), glyph.height());

	// use the overlay as a mask to blend smoothly between the background
	// and the ink
	tile = tile.composite(glyph, VIPS_BLEND_MODE_OVER);

	// and insert the modified tile back into the image
	return background.insert(tile, x, y);
}

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
		image = draw_overlay(image, overlay, 0, 0, {0});

		/* we must make a unique string each time, or libvips will
		 * cache the result for us
		 */
		char str[256];
		vips_snprintf(str, 256, "Wed 26 Sep 21:42:47 BST 2018 - %d", i);
		VImage text = VImage::text(str,
			VImage::option()
				->set("font", "SFmono 18")
				->set("fontfile", "SFMono-Bold.otf"));

		image = draw_overlay(image, text, 10, 10);

		int marker_x = 100;
		int marker_y = 200;
		image = composite_glyph(image, marker, 
			marker_x - marker.width() / 2,
			marker_y - marker.height());

		image.write_to_file("x.jpg");
	}

	return 0;
}


