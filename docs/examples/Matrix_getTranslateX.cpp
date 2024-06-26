// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Matrix_getTranslateX, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    matrix.setTranslate(42, 24);
    SkDebugf("matrix.getTranslateX() %c= 42\n", matrix.getTranslateX() == 42 ? '=' : '!');
}
}  // END FIDDLE
