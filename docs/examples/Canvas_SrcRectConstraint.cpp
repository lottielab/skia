// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_SrcRectConstraint, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap redBorder;
    redBorder.allocPixels(SkImageInfo::MakeN32Premul(4, 4));
    SkCanvas checkRed(redBorder);
    checkRed.clear(SK_ColorRED);
    uint32_t checkers[][2] = { { SK_ColorBLACK, SK_ColorWHITE },
                               { SK_ColorWHITE, SK_ColorBLACK } };
    checkRed.writePixels(
            SkImageInfo::MakeN32Premul(2, 2), (void*) checkers, sizeof(checkers[0]), 1, 1);
    canvas->scale(16, 16);
    canvas->drawImage(redBorder.asImage(), 0, 0);
    canvas->resetMatrix();
    sk_sp<SkImage> image = redBorder.asImage();
    for (auto constraint : { SkCanvas::kStrict_SrcRectConstraint,
                             SkCanvas::kFast_SrcRectConstraint } ) {
        canvas->translate(80, 0);
        canvas->drawImageRect(image.get(), SkRect::MakeLTRB(1, 1, 3, 3),
                SkRect::MakeLTRB(16, 16, 48, 48),
                SkSamplingOptions(SkFilterMode::kLinear), nullptr, constraint);
    }
}
}  // END FIDDLE
