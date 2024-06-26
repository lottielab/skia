// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Image_BorrowTextureFrom, 256, 128, false, 3) {
void draw(SkCanvas* canvas) {
    auto dContext = GrAsDirectContext(canvas->recordingContext());
    if (!dContext) {
        return;
    }

    canvas->scale(.25f, .25f);
    int x = 0;
    for (auto origin : { kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin } ) {
        sk_sp<SkImage> image = SkImages::BorrowTextureFrom(dContext,
                                                           backEndTexture,
                                                           origin,
                                                           kRGBA_8888_SkColorType,
                                                           kOpaque_SkAlphaType,
                                                           nullptr);
        canvas->drawImage(image, x, 0);
        x += 512;
    }
}
}  // END FIDDLE
