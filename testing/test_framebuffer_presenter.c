#include "common.h"
#include "video/enums.h"
#include "video/framebuffer/presenter.h"
#include "video/software/compositor.h"
#include "video/video.h"

#include <stdint.h>
#include <string.h>

static void test_frame_validation(void) {
    const uint32_t pixels[2] = {0xff112233u, 0xffaabbccu};
    framebuffer_image frame = {pixels, 2, 1, 8, FRAMEBUFFER_FORMAT_ABGR8888};

    CU_ASSERT_TRUE(framebuffer_image_valid(&frame));
    CU_ASSERT_FALSE(framebuffer_image_valid(NULL));

    frame.pixels = NULL;
    CU_ASSERT_FALSE(framebuffer_image_valid(&frame));
    frame.pixels = pixels;

    frame.width = 0;
    CU_ASSERT_FALSE(framebuffer_image_valid(&frame));
    frame.width = 2;

    frame.pitch = 4;
    CU_ASSERT_FALSE(framebuffer_image_valid(&frame));
    frame.pitch = 8;

    frame.height = 0;
    CU_ASSERT_FALSE(framebuffer_image_valid(&frame));
    frame.height = 1;

    frame.format = (framebuffer_pixel_format)99;
    CU_ASSERT_FALSE(framebuffer_image_valid(&frame));
}

static void test_presenter_selection(void) {
    framebuffer_presenter presenter = {0};
    const int count = framebuffer_presenter_count();
    CU_ASSERT_TRUE(count >= 0);
    CU_ASSERT_PTR_NULL(framebuffer_presenter_name(-1));
    CU_ASSERT_PTR_NULL(framebuffer_presenter_name(count));
    CU_ASSERT_FALSE(framebuffer_presenter_create(NULL, NULL));
    CU_ASSERT_FALSE(framebuffer_presenter_create(&presenter, "nonexistent"));
    if(count > 0) {
        const char *name = framebuffer_presenter_name(0);
        CU_ASSERT_PTR_NOT_NULL_FATAL(name);
        CU_ASSERT_TRUE_FATAL(framebuffer_presenter_create(&presenter, name));
        CU_ASSERT_PTR_NOT_NULL(presenter.ctx);
        CU_ASSERT_PTR_NOT_NULL(presenter.open);
        CU_ASSERT_PTR_NOT_NULL(presenter.present);
        CU_ASSERT_PTR_NOT_NULL(presenter.close);
        CU_ASSERT_STRING_EQUAL(presenter.get_name(), name);
        framebuffer_presenter_destroy(&presenter);
        CU_ASSERT_PTR_NULL(presenter.ctx);
        CU_ASSERT_TRUE(framebuffer_presenter_create(&presenter, NULL));
        framebuffer_presenter_destroy(&presenter);
    }
}

static void test_software_pixels(void) {
    software_compositor comp = {0};
    CU_ASSERT_TRUE_FATAL(software_compositor_init(&comp));
    vga_palette palette = {0};
    palette.colors[1] = (vga_color){255, 0, 0};
    palette.colors[2] = (vga_color){0, 255, 0};
    palette.colors[3] = (vga_color){0, 0, 255};
    palette.colors[0x12] = (vga_color){32, 64, 128};
    palette.colors[121] = (vga_color){6, 7, 8};
    software_compositor_set_palette(&comp, &palette);

    vga_remap_tables remaps = {0};
    for(int row = 0; row < VGA_REMAP_COUNT; row++) {
        for(int index = 0; index < VGA_PALETTE_SIZE; index++) {
            remaps.tables[row].data[index] = (vga_pixel)index;
        }
    }
    remaps.tables[2].data[1] = 3;
    remaps.tables[3].data[1] = 3;
    remaps.tables[0].data[1] = 2;
    software_compositor_set_remaps(&comp, &remaps);

    vga_pixel data[] = {1, 2};
    surface sprite = {0};
    sprite.w = 2;
    sprite.h = 1;
    sprite.data = data;
    sprite.transparent = -1;
    irect dst = irect_create(0, 0, 2, 1);
    software_compositor_prepare(&comp, 0);
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, 0);
    const uint32_t *rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xff0000ffu);
    CU_ASSERT_EQUAL(rgba[1], 0xff00ff00u);
    CU_ASSERT_EQUAL(rgba[2], 0xff000000u);

    dst = irect_create(0, 0, 2, 1);
    software_compositor_draw(&comp, &sprite, &dst, 2, 1, 0, 0, 255, FLIP_NONE, 0);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xffff0000u);
    CU_ASSERT_EQUAL(rgba[1], 0xff00ff00u);

    software_compositor_prepare(&comp, 0);
    dst = irect_create(0, 0, 2, 1);
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_HORIZONTAL, 0);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xff00ff00u);
    CU_ASSERT_EQUAL(rgba[1], 0xff0000ffu);

    software_compositor_prepare(&comp, 0);
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 0, FLIP_NONE, 0);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xff000000u);

    software_compositor_prepare(&comp, 0);
    sprite.transparent = 2;
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, 0);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xff0000ffu);
    CU_ASSERT_EQUAL(rgba[1], 0xff000000u);

    /* Sprite-source remap is applied when the sprite is drawn. */
    sprite.transparent = -1;
    sprite.w = 1;
    software_compositor_prepare(&comp, 0);
    dst = irect_create(0, 0, 1, 1);
    software_compositor_draw(&comp, &sprite, &dst, 2, 0, 0, 0, 255, FLIP_NONE, SPRITE_REMAP);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xffff0000u);

    /* Destination-dependent shadow remaps preserve the underlying index. */
    software_compositor_prepare(&comp, 0);
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, 0);
    software_compositor_draw(&comp, &sprite, &dst, -1, 1, 0, 0, 255, FLIP_NONE, SPRITE_SHADOW);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xff00ff00u);

    /* Translucency uses the destination brightness ramp before resolving. */
    software_compositor_prepare(&comp, 0);
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, 0);
    data[0] = 0x10;
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, SPRITE_TRANSLUCENT);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xff804020u);

    /* Credits add palette indices rather than blending RGB channels. */
    data[0] = 1;
    software_compositor_prepare(&comp, FBUFOPT_CREDITS);
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, 0);
    data[0] = 2;
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, SPRITE_INDEX_ADD);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xff080706u);

    /* Out-of-range additive palette indices resolve to black, not palette[255]. */
    software_compositor_prepare(&comp, FBUFOPT_CREDITS);
    data[0] = 1;
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, 0);
    data[0] = 5;
    software_compositor_draw(&comp, &sprite, &dst, 0, 0, 0, 0, 255, FLIP_NONE, SPRITE_INDEX_ADD);
    rgba = software_compositor_resolve(&comp);
    CU_ASSERT_EQUAL(rgba[0], 0xff000000u);

    software_compositor_close(&comp);
    CU_ASSERT_PTR_NULL(comp.indexed);
    CU_ASSERT_PTR_NULL(comp.rgba);
}

static void test_offscreen_area(void) {
    software_compositor comp = {0};
    CU_ASSERT_TRUE_FATAL(software_compositor_init(&comp));
    software_compositor_prepare(&comp, 0);
    vga_pixel data[] = {7};
    surface sprite = {0};
    sprite.w = 1;
    sprite.h = 1;
    sprite.data = data;
    sprite.transparent = -1;
    irect dest = irect_create(0, 0, 1, 1);
    software_compositor_draw(&comp, &sprite, &dest, 0, 0, 0, 0, 255, FLIP_NONE, 0);
    const irect area = irect_create(0, 0, 1, 1);
    surface capture = {0};
    software_compositor_capture_area(&comp, &area, &capture);
    CU_ASSERT_PTR_NOT_NULL_FATAL(capture.data);
    CU_ASSERT_EQUAL(capture.data[0], 7);
    surface_free(&capture);
    software_compositor_close(&comp);
}

static void test_clipped_scaling(void) {
    software_compositor comp = {0};
    CU_ASSERT_TRUE_FATAL(software_compositor_init(&comp));
    vga_palette palette = {0};
    for(int index = 1; index <= 4; index++) {
        palette.colors[index] = (vga_color){(uint8_t)index, 0, 0};
    }
    software_compositor_set_palette(&comp, &palette);
    software_compositor_prepare(&comp, 0);

    vga_pixel data[] = {1, 2, 3, 4};
    surface sprite = {0};
    sprite.w = 2;
    sprite.h = 2;
    sprite.data = data;
    sprite.transparent = -1;
    const irect destination = irect_create(-1, -1, 4, 4);
    software_compositor_draw(&comp, &sprite, &destination, 0, 0, 0, 255, 255, FLIP_NONE, 0);
    const uint32_t *rgba = software_compositor_resolve(&comp);

    CU_ASSERT_EQUAL(rgba[0], 0xff000001u);
    CU_ASSERT_EQUAL(rgba[1], 0xff000002u);
    CU_ASSERT_EQUAL(rgba[NATIVE_W], 0xff000003u);
    CU_ASSERT_EQUAL(rgba[NATIVE_W + 1], 0xff000004u);

    software_compositor_close(&comp);
}

void framebuffer_presenter_test_suite(CU_pSuite suite) {
    ADD_TEST("Frame validation", test_frame_validation);
    ADD_TEST("Presenter selection", test_presenter_selection);
    ADD_TEST("Software indexed pixel compositing", test_software_pixels);
    ADD_TEST("Software offscreen capture", test_offscreen_area);
    ADD_TEST("Software clipped pixel-center scaling", test_clipped_scaling);
}
