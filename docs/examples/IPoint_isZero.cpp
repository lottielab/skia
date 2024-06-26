// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(IPoint_isZero, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkIPoint pt = { 0, -0};
    SkDebugf("pt.isZero() == %s\n", pt.isZero() ? "true" : "false");
}
}  // END FIDDLE
