/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

/**
 * Draws text with random parameters. The text draws each get their own clip rect. It is also
 * used as a bench to measure how well the GPU backend combines draw ops for text draws.
 */

class VariedTextGM : public skiagm::GM {
public:
    VariedTextGM(bool effectiveClip, bool lcd)
        : fEffectiveClip(effectiveClip)
        , fLCD(lcd) {
    }

protected:
    SkString getName() const override {
        SkString name("varied_text");
        if (fEffectiveClip) {
            name.append("_clipped");
        } else {
            name.append("_ignorable_clip");
        }
        if (fLCD) {
            name.append("_lcd");
        } else {
            name.append("_no_lcd");
        }
        return name;
    }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onOnceBeforeDraw() override {
        fPaint.setAntiAlias(true);
        fFont.setEdging(fLCD ? SkFont::Edging::kSubpixelAntiAlias : SkFont::Edging::kAntiAlias);

        SkISize size = this->getISize();
        SkScalar w = SkIntToScalar(size.fWidth);
        SkScalar h = SkIntToScalar(size.fHeight);

        SkASSERTF(4 == std::size(fTypefaces), "typeface_cnt");
        fTypefaces[0] = ToolUtils::CreatePortableTypeface("sans-serif", SkFontStyle());
        fTypefaces[1] = ToolUtils::CreatePortableTypeface("sans-serif", SkFontStyle::Bold());
        fTypefaces[2] = ToolUtils::CreatePortableTypeface("serif", SkFontStyle());
        fTypefaces[3] = ToolUtils::CreatePortableTypeface("serif", SkFontStyle::Bold());

        SkRandom random;
        for (int i = 0; i < kCnt; ++i) {
            int length = random.nextRangeU(kMinLength, kMaxLength);
            char text[kMaxLength];
            for (int j = 0; j < length; ++j) {
                text[j] = (char)random.nextRangeU('!', 'z');
            }
            fStrings[i].set(text, length);

            fColors[i] = random.nextU();
            fColors[i] |= 0xFF000000;
            fColors[i] = ToolUtils::color_to_565(fColors[i]);

            constexpr SkScalar kMinPtSize = 8.f;
            constexpr SkScalar kMaxPtSize = 32.f;

            fPtSizes[i] = random.nextRangeScalar(kMinPtSize, kMaxPtSize);

            fTypefaceIndices[i] = random.nextULessThan(std::size(fTypefaces));

            SkRect r;
            fPaint.setColor(fColors[i]);
            fFont.setTypeface(fTypefaces[fTypefaceIndices[i]]);
            fFont.setSize(fPtSizes[i]);

            fFont.measureText(fStrings[i].c_str(), fStrings[i].size(), SkTextEncoding::kUTF8, &r);
            // The set of x,y offsets which place the bounding box inside the GM's border.
            SkRect safeRect = SkRect::MakeLTRB(-r.fLeft, -r.fTop, w - r.fRight, h - r.fBottom);
            if (safeRect.isEmpty()) {
                // If the bounds don't fit then allow any offset in the GM's border.
                safeRect = SkRect::MakeWH(w, h);
            }
            fOffsets[i].fX = random.nextRangeScalar(safeRect.fLeft, safeRect.fRight);
            fOffsets[i].fY = random.nextRangeScalar(safeRect.fTop, safeRect.fBottom);

            fClipRects[i] = r;
            fClipRects[i].offset(fOffsets[i].fX, fOffsets[i].fY);
            fClipRects[i].outset(2.f, 2.f);

            if (fEffectiveClip) {
                fClipRects[i].fRight -= 0.25f * fClipRects[i].width();
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        for (int i = 0; i < kCnt; ++i) {
            fPaint.setColor(fColors[i]);
            fFont.setSize(fPtSizes[i]);
            fFont.setTypeface(fTypefaces[fTypefaceIndices[i]]);

            canvas->save();
                canvas->clipRect(fClipRects[i]);
                canvas->translate(fOffsets[i].fX, fOffsets[i].fY);
                canvas->drawSimpleText(fStrings[i].c_str(), fStrings[i].size(), SkTextEncoding::kUTF8,
                                       0, 0, fFont, fPaint);
            canvas->restore();
        }

        // Visualize the clips, but not in bench mode.
        if (kBench_Mode != this->getMode()) {
            SkPaint wirePaint;
            wirePaint.setAntiAlias(true);
            wirePaint.setStrokeWidth(0);
            wirePaint.setStyle(SkPaint::kStroke_Style);
            for (int i = 0; i < kCnt; ++i) {
                canvas->drawRect(fClipRects[i], wirePaint);
            }
        }
    }

    bool runAsBench() const override { return true; }

private:
    inline static constexpr int kCnt = 30;
    inline static constexpr int kMinLength = 15;
    inline static constexpr int kMaxLength = 40;

    bool        fEffectiveClip;
    bool        fLCD;
    sk_sp<SkTypeface> fTypefaces[4];
    SkPaint     fPaint;
    SkFont      fFont;

    // precomputed for each text draw
    SkString        fStrings[kCnt];
    SkColor         fColors[kCnt];
    SkScalar        fPtSizes[kCnt];
    int             fTypefaceIndices[kCnt];
    SkPoint         fOffsets[kCnt];
    SkRect          fClipRects[kCnt];

    using INHERITED = skiagm::GM;
};

DEF_GM(return new VariedTextGM(false, false);)
DEF_GM(return new VariedTextGM(true, false);)
DEF_GM(return new VariedTextGM(false, true);)
DEF_GM(return new VariedTextGM(true, true);)
