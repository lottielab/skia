// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(ImageInfo_MakeA8, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t pixels[][8] = { { 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00},
                            { 0x00, 0x7f, 0xff, 0x3f, 0x3f, 0x7f, 0x3f, 0x00},
                            { 0x3f, 0xff, 0x7f, 0x00, 0x7f, 0xff, 0x7f, 0x00},
                            { 0x00, 0x3f, 0x00, 0x00, 0x3f, 0x7f, 0x3f, 0x00},
                            { 0x3f, 0x7f, 0x7f, 0x3f, 0x00, 0x00, 0x00, 0x00},
                            { 0x7f, 0xff, 0xff, 0x7f, 0x00, 0x3f, 0x7f, 0x3f},
                            { 0x7f, 0xff, 0xff, 0x7f, 0x00, 0x7f, 0xff, 0x7f},
                            { 0x3f, 0x7f, 0x7f, 0x3f, 0x00, 0x3f, 0x7f, 0x3f} };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeA8(8, 8),
            (void*) pixels, sizeof(pixels[0]));
    SkPaint paint;
    canvas->scale(4, 4);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xFF007F00} ) {
        paint.setColor(color);
        canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
        canvas->translate(12, 0);
    }
}
}  // END FIDDLE
