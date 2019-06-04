/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019  natinusala

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <View.hpp>
#include <Animations.hpp>
#include <Application.hpp>

#include <math.h>

NVGcolor transparent = nvgRGBA(0, 0, 0, 0);

#define SHAKE_ANIMATION_DURATION 15

static int shakeAnimation(float t, float a) // a = amplitude
{
    // Damped sine wave
    float w = 0.8f;     // period
    float c = 0.35f;    // damp factor
    return roundf(a * exp(-(c * t)) * sin(w * t));
}

void View::shakeHighlight(FocusDirection direction)
{
    this->highlightShaking          = true;
    this->highlightShakeStart       = cpu_features_get_time_usec() / 1000;
    this->highlightShakeDirection   = direction;
    this->highlightShakeAmplitude   = rand() % 15 + 10;
}

float View::getAlpha()
{
    return this->alpha * (this->parent ? this->parent->getAlpha() : 1.0f);
}

NVGcolor View::a(NVGcolor color)
{
    NVGcolor newColor = color;
    newColor.a *= this->getAlpha();
    return newColor;
}

void View::frame(FrameContext *ctx)
{
    Style *style = Application::getStyle();

    nvgSave(ctx->vg);

    // Layout if needed
    if (this->dirty)
    {
        this->layout(ctx->vg, style);
        this->dirty = false;
    }

    if (this->alpha > 0.0f)
    {
        // Draw background
        this->drawBackground(ctx->vg, ctx, style);

        // Draw highlight background
        if (this->highlightAlpha > 0.0f)
            this->drawHighlight(ctx->vg, ctx->theme, this->highlightAlpha, style, true);

        // Draw the view
        this->draw(ctx->vg, this->x, this->y, this->width, this->height, style, ctx);

        // Draw highlight
        if (this->highlightAlpha > 0.0f)
            this->drawHighlight(ctx->vg, ctx->theme, this->highlightAlpha, style, false);
    }

    nvgRestore(ctx->vg);
}

// TODO: Slight glow all around
void View::drawHighlight(NVGcontext *vg, Theme *theme, float alpha, Style *style, bool background)
{
    unsigned insetTop, insetRight, insetBottom, insetLeft;
    this->getHighlightInsets(&insetTop, &insetRight, &insetBottom, &insetLeft);

    unsigned x      = this->x - insetLeft   - style->Highlight.strokeWidth/2;
    unsigned y      = this->y - insetTop    - style->Highlight.strokeWidth/2;
    unsigned width  = this->width   + insetLeft + insetRight  + style->Highlight.strokeWidth;
    unsigned height = this->height  + insetTop  + insetBottom + style->Highlight.strokeWidth;

    // Shake animation
    if (this->highlightShaking)
    {
        retro_time_t curTime = cpu_features_get_time_usec() / 1000;
        retro_time_t t = (curTime - highlightShakeStart) / 10;

        if (t >= SHAKE_ANIMATION_DURATION)
        {
            this->highlightShaking = false;
        }
        else
        {
            switch (this->highlightShakeDirection)
            {
                case FocusDirection::RIGHT:
                    x += shakeAnimation(t, this->highlightShakeAmplitude);
                    break;
                case FocusDirection::LEFT:
                    x -= shakeAnimation(t, this->highlightShakeAmplitude);
                    break;
                case FocusDirection::DOWN:
                    y += shakeAnimation(t, this->highlightShakeAmplitude);
                    break;
                case FocusDirection::UP:
                    y -= shakeAnimation(t, this->highlightShakeAmplitude);
                    break;
                default:
                    break;
            }
        }
    }

    // Draw
    if (background)
    {
        // Background
        nvgFillColor(vg, RGBAf(theme->highlightBackgroundColor.r, theme->highlightBackgroundColor.g, theme->highlightBackgroundColor.b, this->highlightAlpha));
        nvgBeginPath(vg);
        nvgRoundedRect(vg, x, y, width, height, style->Highlight.cornerRadius);
        nvgFill(vg);
    }
    else
    {
        // Shadow
        NVGpaint shadowPaint = nvgBoxGradient(vg,
            x, y + style->Highlight.shadowWidth,
            width, height,
            style->Highlight.cornerRadius*2, style->Highlight.shadowFeather,
            RGBA(0, 0, 0, style->Highlight.shadowOpacity * alpha), transparent);

        nvgBeginPath(vg);
        nvgRect(vg, x - style->Highlight.shadowOffset, y - style->Highlight.shadowOffset,
            width + style->Highlight.shadowOffset*2, height + style->Highlight.shadowOffset*3);
        nvgRoundedRect(vg, x, y, width, height, style->Highlight.cornerRadius);
        nvgPathWinding(vg, NVG_HOLE);
        nvgFillPaint(vg, shadowPaint);
        nvgFill(vg);

        // Border
        // TODO: Fix it on Switch
        float gradientX, gradientY, color;
        menu_animation_get_highlight(&gradientX, &gradientY, &color);

        NVGcolor pulsationColor = RGBAf((color * theme->highlightColor1.r) + (1-color) * theme->highlightColor2.r,
            (color * theme->highlightColor1.g) + (1-color) * theme->highlightColor2.g,
            (color * theme->highlightColor1.b) + (1-color) * theme->highlightColor2.b,
            alpha);

        NVGcolor borderColor = theme->highlightColor2;
        borderColor.a = 0.5f * alpha * this->getAlpha();

        NVGpaint border1Paint = nvgRadialGradient(vg,
            x + gradientX * width, y + gradientY * height,
            style->Highlight.strokeWidth*10, style->Highlight.strokeWidth*40,
            borderColor, transparent);

        NVGpaint border2Paint = nvgRadialGradient(vg,
            x + (1-gradientX) * width, y + (1-gradientY) * height,
            style->Highlight.strokeWidth*10, style->Highlight.strokeWidth*40,
            borderColor, transparent);

        nvgBeginPath(vg);
        nvgStrokeColor(vg, pulsationColor);
        nvgStrokeWidth(vg, style->Highlight.strokeWidth);
        nvgRoundedRect(vg, x, y, width, height, style->Highlight.cornerRadius);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokePaint(vg, border1Paint);
        nvgStrokeWidth(vg, style->Highlight.strokeWidth);
        nvgRoundedRect(vg, x, y, width, height, style->Highlight.cornerRadius);
        nvgStroke(vg);

        nvgBeginPath(vg);
        nvgStrokePaint(vg, border2Paint);
        nvgStrokeWidth(vg, style->Highlight.strokeWidth);
        nvgRoundedRect(vg, x, y, width, height, style->Highlight.cornerRadius);
        nvgStroke(vg);
    }
}

void View::setBackground(Background background)
{
    this->background = background;
}

void View::drawBackground(NVGcontext* vg, FrameContext *ctx, Style *style)
{
    switch (this->background)
    {
        case Background::SIDEBAR:
        {
            unsigned backdropHeight = style->Background.sidebarBorderHeight;

            // Solid color
            nvgBeginPath(vg);
            nvgFillColor(vg, a(ctx->theme->sidebarColor));
            nvgRect(vg, this->x, this->y + backdropHeight, this->width, this->height - backdropHeight * 2);
            nvgFill(vg);

            //Borders gradient
            // Top
            NVGpaint topGradient = nvgLinearGradient(vg, this->x, this->y + backdropHeight, this->x, this->y, a(ctx->theme->sidebarColor), transparent);
            nvgBeginPath(vg);
            nvgFillPaint(vg, topGradient);
            nvgRect(vg, this->x, this->y, this->width, backdropHeight);
            nvgFill(vg);

            // Bottom
            NVGpaint bottomGradient = nvgLinearGradient(vg, this->x, this->y + this->height - backdropHeight, this->x, this->y + this->height, a(ctx->theme->sidebarColor), transparent);
            nvgBeginPath(vg);
            nvgFillPaint(vg, bottomGradient);
            nvgRect(vg, this->x, this->y + this->height - backdropHeight, this->width, backdropHeight);
            nvgFill(vg);
            break;
        }
        case Background::DEBUG:
        {
            nvgFillColor(vg, RGB(255, 0, 0));
            nvgBeginPath(vg);
            nvgRect(vg, this->x, this->y, this->width, this->height);
            nvgFill(vg);
            break;
        }
        default:
            break;
    }
}

void View::setBoundaries(int x, int y, unsigned width, unsigned height)
{
    this->x         = x;
    this->y         = y;
    this->width     = width;
    this->height    = height;
}

void View::setParent(View *parent)
{
    this->parent = parent;
}

bool View::isFocused()
{
    return this->focused;
}

View* View::getParent()
{
    return this->parent;
}

void View::setWidth(unsigned width)
{
    this->width = width;
}

void View::setHeight(unsigned height)
{
    this->height = height;
}

int View::getX()
{
    return this->x;
}

int View::getY()
{
    return this->y;
}

unsigned View::getHeight()
{
    return this->height;
}

unsigned View::getWidth()
{
    return this->width;
}

void View::onFocusGained()
{
    this->focused = true;

    menu_animation_ctx_tag tag = (uintptr_t) this->highlightAlpha;

    menu_animation_ctx_entry_t entry;
    entry.cb            = nullptr;
    entry.duration      = VIEW_HIGHLIGHT_ANIMATION_DURATION;
    entry.easing_enum   = EASING_OUT_QUAD;
    entry.subject       = &this->highlightAlpha;
    entry.tag           = tag;
    entry.target_value  = 1.0f;
    entry.tick          = nullptr;
    entry.userdata      = nullptr;

    menu_animation_push(&entry);

    if (this->focusListener)
        this->focusListener(this);
}


void View::setFocusListener(function<void(View*)> listener)
{
    this->focusListener = listener;
}

/**
 * Fired when focus is lost
 */
void View::onFocusLost()
{
    this->focused = false;

    menu_animation_ctx_tag tag = (uintptr_t) this->highlightAlpha;

    menu_animation_ctx_entry_t entry;
    entry.cb            = nullptr;
    entry.duration      = VIEW_HIGHLIGHT_ANIMATION_DURATION;
    entry.easing_enum   = EASING_OUT_QUAD;
    entry.subject       = &this->highlightAlpha;
    entry.tag           = tag;
    entry.target_value  = 0.0f;
    entry.tick          = nullptr;
    entry.userdata      = nullptr;

    menu_animation_push(&entry);
}