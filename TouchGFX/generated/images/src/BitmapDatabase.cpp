// 4.23.0 0x0961cc04
// Generated by imageconverter. Please, do not edit!

#include <images/BitmapDatabase.hpp>
#include <touchgfx/Bitmap.hpp>

extern const unsigned char image_alternate_theme_images_backgrounds_320x240_gradient_dark[]; // BITMAP_ALTERNATE_THEME_IMAGES_BACKGROUNDS_320X240_GRADIENT_DARK_ID = 0, Size: 320x240 pixels
extern const unsigned char image_alternate_theme_images_widgets_button_regular_height_50_large_round_action[]; // BITMAP_ALTERNATE_THEME_IMAGES_WIDGETS_BUTTON_REGULAR_HEIGHT_50_LARGE_ROUND_ACTION_ID = 1, Size: 370x50 pixels
extern const unsigned char image_alternate_theme_images_widgets_button_regular_height_50_medium_rounded_pressed[]; // BITMAP_ALTERNATE_THEME_IMAGES_WIDGETS_BUTTON_REGULAR_HEIGHT_50_MEDIUM_ROUNDED_PRESSED_ID = 2, Size: 240x50 pixels

const touchgfx::Bitmap::BitmapData bitmap_database[] = {
    { image_alternate_theme_images_backgrounds_320x240_gradient_dark, 0, 320, 240, 0, 0, 320, ((uint8_t)touchgfx::Bitmap::RGB565) >> 3, 240, ((uint8_t)touchgfx::Bitmap::RGB565) & 0x7 },
    { image_alternate_theme_images_widgets_button_regular_height_50_large_round_action, 0, 370, 50, 24, 0, 322, ((uint8_t)touchgfx::Bitmap::ARGB8888) >> 3, 50, ((uint8_t)touchgfx::Bitmap::ARGB8888) & 0x7 },
    { image_alternate_theme_images_widgets_button_regular_height_50_medium_rounded_pressed, 0, 240, 50, 5, 0, 230, ((uint8_t)touchgfx::Bitmap::ARGB8888) >> 3, 50, ((uint8_t)touchgfx::Bitmap::ARGB8888) & 0x7 }
};

namespace BitmapDatabase
{
const touchgfx::Bitmap::BitmapData* getInstance()
{
    return bitmap_database;
}

uint16_t getInstanceSize()
{
    return (uint16_t)(sizeof(bitmap_database) / sizeof(touchgfx::Bitmap::BitmapData));
}
} // namespace BitmapDatabase
