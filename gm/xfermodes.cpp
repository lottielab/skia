/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/utils/SkTextUtils.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

enum SrcType {
    //! A WxH image with a rectangle in the lower right.
    kRectangleImage_SrcType               = 0x01,
    //! kRectangleImage_SrcType with an alpha of 34.5%.
    kRectangleImageWithAlpha_SrcType      = 0x02,
    //! kRectnagleImageWithAlpha_SrcType scaled down by half.
    kSmallRectangleImageWithAlpha_SrcType = 0x04,
    //! kRectangleImage_SrcType drawn directly instead in an image.
    kRectangle_SrcType                    = 0x08,
    //! Two rectangles, first on the right half, second on the bottom half.
    kQuarterClear_SrcType                 = 0x10,
    //! kQuarterClear_SrcType in a layer.
    kQuarterClearInLayer_SrcType          = 0x20,
    //! A W/2xH/2 transparent image.
    kSmallTransparentImage_SrcType        = 0x40,
    //! kRectangleImage_SrcType drawn directly with a mask.
    kRectangleWithMask_SrcType            = 0x80,

    kAll_SrcType                          = 0xFF, //!< All the source types.
    kBasic_SrcType                        = 0x03, //!< Just basic source types.
};

const struct {
    SkBlendMode fMode;
    int         fSourceTypeMask;  // The source types to use this
                                  // mode with. See draw_mode for
                                  // an explanation of each type.
                                  // PDF has to play some tricks
                                  // to support the base modes,
                                  // test those more extensively.
} gModes[] = {
    { SkBlendMode::kClear,        kAll_SrcType   },
    { SkBlendMode::kSrc,          kAll_SrcType   },
    { SkBlendMode::kDst,          kAll_SrcType   },
    { SkBlendMode::kSrcOver,      kAll_SrcType   },
    { SkBlendMode::kDstOver,      kAll_SrcType   },
    { SkBlendMode::kSrcIn,        kAll_SrcType   },
    { SkBlendMode::kDstIn,        kAll_SrcType   },
    { SkBlendMode::kSrcOut,       kAll_SrcType   },
    { SkBlendMode::kDstOut,       kAll_SrcType   },
    { SkBlendMode::kSrcATop,      kAll_SrcType   },
    { SkBlendMode::kDstATop,      kAll_SrcType   },

    { SkBlendMode::kXor,          kBasic_SrcType },
    { SkBlendMode::kPlus,         kBasic_SrcType },
    { SkBlendMode::kModulate,     kAll_SrcType   },
    { SkBlendMode::kScreen,       kBasic_SrcType },
    { SkBlendMode::kOverlay,      kBasic_SrcType },
    { SkBlendMode::kDarken,       kBasic_SrcType },
    { SkBlendMode::kLighten,      kBasic_SrcType },
    { SkBlendMode::kColorDodge,   kBasic_SrcType },
    { SkBlendMode::kColorBurn,    kBasic_SrcType },
    { SkBlendMode::kHardLight,    kBasic_SrcType },
    { SkBlendMode::kSoftLight,    kBasic_SrcType },
    { SkBlendMode::kDifference,   kBasic_SrcType },
    { SkBlendMode::kExclusion,    kBasic_SrcType },
    { SkBlendMode::kMultiply,     kAll_SrcType   },
    { SkBlendMode::kHue,          kBasic_SrcType },
    { SkBlendMode::kSaturation,   kBasic_SrcType },
    { SkBlendMode::kColor,        kBasic_SrcType },
    { SkBlendMode::kLuminosity,   kBasic_SrcType },
};

static void make_bitmaps(int w, int h, SkBitmap* src, SkBitmap* dst,
                         SkBitmap* transparent) {
    src->allocN32Pixels(w, h);
    src->eraseColor(SK_ColorTRANSPARENT);

    SkPaint p;
    p.setAntiAlias(true);

    SkRect r;
    SkScalar ww = SkIntToScalar(w);
    SkScalar hh = SkIntToScalar(h);

    {
        SkCanvas c(*src);
        p.setColor(ToolUtils::color_to_565(0xFFFFCC44));
        r.setWH(ww*3/4, hh*3/4);
        c.drawOval(r, p);
    }

    dst->allocN32Pixels(w, h);
    dst->eraseColor(SK_ColorTRANSPARENT);

    {
        SkCanvas c(*dst);
        p.setColor(ToolUtils::color_to_565(0xFF66AAFF));
        r.setLTRB(ww/3, hh/3, ww*19/20, hh*19/20);
        c.drawRect(r, p);
    }

    transparent->allocN32Pixels(w, h);
    transparent->eraseColor(SK_ColorTRANSPARENT);
}

static uint16_t gData[] = { 0xFFFF, 0xCCCF, 0xCCCF, 0xFFFF };

class XfermodesGM : public skiagm::GM {
    SkBitmap    fBG;
    SkBitmap    fSrcB, fDstB, fTransparent;

    /* The srcType argument indicates what to draw for the source part. Skia
     * uses the implied shape of the drawing command and these modes
     * demonstrate that.
     */
    void draw_mode(SkCanvas* canvas, SkBlendMode mode, SrcType srcType, SkScalar x, SkScalar y) {
        SkPaint p;
        SkSamplingOptions sampling;
        SkMatrix m;
        bool restoreNeeded = false;
        m.setTranslate(x, y);

        canvas->drawImage(fSrcB.asImage(), x, y, sampling, &p);
        p.setBlendMode(mode);
        switch (srcType) {
            case kSmallTransparentImage_SrcType: {
                m.postScale(SK_ScalarHalf, SK_ScalarHalf, x, y);

                SkAutoCanvasRestore acr(canvas, true);
                canvas->concat(m);
                canvas->drawImage(fTransparent.asImage(), 0, 0, sampling, &p);
                break;
            }
            case kQuarterClearInLayer_SrcType: {
                SkRect bounds = SkRect::MakeXYWH(x, y, SkIntToScalar(W),
                                                 SkIntToScalar(H));
                canvas->saveLayer(&bounds, &p);
                restoreNeeded = true;
                p.setBlendMode(SkBlendMode::kSrcOver);
                [[fallthrough]];
            }
            case kQuarterClear_SrcType: {
                SkScalar halfW = SkIntToScalar(W) / 2;
                SkScalar halfH = SkIntToScalar(H) / 2;
                p.setColor(ToolUtils::color_to_565(0xFF66AAFF));
                SkRect r = SkRect::MakeXYWH(x + halfW, y, halfW,
                                            SkIntToScalar(H));
                canvas->drawRect(r, p);
                p.setColor(ToolUtils::color_to_565(0xFFAA66FF));
                r = SkRect::MakeXYWH(x, y + halfH, SkIntToScalar(W), halfH);
                canvas->drawRect(r, p);
                break;
            }
            case kRectangleWithMask_SrcType: {
                canvas->save();
                restoreNeeded = true;
                SkScalar w = SkIntToScalar(W);
                SkScalar h = SkIntToScalar(H);
                SkRect r = SkRect::MakeXYWH(x, y + h / 4, w, h * 23 / 60);
                canvas->clipRect(r);
                [[fallthrough]];
            }
            case kRectangle_SrcType: {
                SkScalar w = SkIntToScalar(W);
                SkScalar h = SkIntToScalar(H);
                SkRect r = SkRect::MakeXYWH(x + w / 3, y + h / 3,
                                            w * 37 / 60, h * 37 / 60);
                p.setColor(ToolUtils::color_to_565(0xFF66AAFF));
                canvas->drawRect(r, p);
                break;
            }
            case kSmallRectangleImageWithAlpha_SrcType:
                m.postScale(SK_ScalarHalf, SK_ScalarHalf, x, y);
                [[fallthrough]];
            case kRectangleImageWithAlpha_SrcType:
                p.setAlpha(0x88);
                [[fallthrough]];
            case kRectangleImage_SrcType: {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->concat(m);
                canvas->drawImage(fDstB.asImage(), 0, 0, sampling, &p);
                break;
            }
            default:
                break;
        }

        if (restoreNeeded) {
            canvas->restore();
        }
    }

    void onOnceBeforeDraw() override {
        fBG.installPixels(SkImageInfo::Make(2, 2, kARGB_4444_SkColorType,
                                            kOpaque_SkAlphaType),
                          gData, 4);

        make_bitmaps(W, H, &fSrcB, &fDstB, &fTransparent);
    }

public:
    const static int W = 64;
    const static int H = 64;
    XfermodesGM() {}

protected:
    SkString getName() const override { return SkString("xfermodes"); }

    SkISize getISize() override { return SkISize::Make(1990, 570); }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        const SkScalar w = SkIntToScalar(W);
        const SkScalar h = SkIntToScalar(H);
        SkMatrix m;
        m.setScale(SkIntToScalar(6), SkIntToScalar(6));
        auto s = fBG.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                SkSamplingOptions(), m);

        SkPaint labelP;
        labelP.setAntiAlias(true);

        SkFont font = ToolUtils::DefaultPortableFont();

        const int kWrap = 5;

        SkScalar x0 = 0;
        SkScalar y0 = 0;
        for (int sourceType = 1; sourceType & kAll_SrcType; sourceType <<= 1) {
            SkScalar x = x0, y = y0;
            for (size_t i = 0; i < std::size(gModes); i++) {
                if ((gModes[i].fSourceTypeMask & sourceType) == 0) {
                    continue;
                }
                SkRect r{ x, y, x+w, y+h };

                SkPaint p;
                p.setStyle(SkPaint::kFill_Style);
                p.setShader(s);
                canvas->drawRect(r, p);

                canvas->saveLayer(&r, nullptr);
                draw_mode(canvas, gModes[i].fMode, static_cast<SrcType>(sourceType),
                          r.fLeft, r.fTop);
                canvas->restore();

                r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
                p.setStyle(SkPaint::kStroke_Style);
                p.setShader(nullptr);
                canvas->drawRect(r, p);

#if 1
                const char* label = SkBlendMode_Name(gModes[i].fMode);
                SkTextUtils::DrawString(canvas, label, x + w/2, y - font.getSize()/2,
                                        font, labelP, SkTextUtils::kCenter_Align);
#endif
                x += w + SkIntToScalar(10);
                if ((i % kWrap) == kWrap - 1) {
                    x = x0;
                    y += h + SkIntToScalar(30);
                }
            }
            if (y < 320) {
                if (x > x0) {
                    y += h + SkIntToScalar(30);
                }
                y0 = y;
            } else {
                x0 += SkIntToScalar(400);
                y0 = 0;
            }
        }
    }

private:
    using INHERITED = GM;
};
DEF_GM( return new XfermodesGM; )
