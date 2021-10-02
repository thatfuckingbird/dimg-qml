/*
* Adapted from digiKam source code. Original license notice:
* Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
* Copyright (C) 2011-2021 by Gilles Caulier <caulier dot gilles at gmail dot com>
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/

#include "item.h"
#include "dimg/core/libs/metadataengine/dmetadata/dmetadata.h"
#include "dimg/core/libs/dimg/filters/icc/iccmanager.h"
#include "dimg/core/libs/dimg/filters/fx/filmgrainfilter.h"
#include <QPainter>
#include <QGuiApplication>
#include <QPixmapCache>
#include <QQueue>

constexpr int CACHE_MAX_COUNT = 2;

ImageZoomSettings::ImageZoomSettings()
    : m_zoom(1.0),
      m_zoomConst(1.0),
      m_zoomRatio(qApp->devicePixelRatio())
{
}

ImageZoomSettings::ImageZoomSettings(const QSize& imageSize, const QSize& originalSize)
    : m_zoom(1),
      m_zoomConst(1),
      m_zoomRatio(qApp->devicePixelRatio())
{
    setImageSize(imageSize, originalSize);
}

void ImageZoomSettings::setImageSize(const QSize& size, const QSize& originalSize)
{
    m_size = size;

    if (!originalSize.isNull() && originalSize.isValid())
    {
        m_zoomConst = m_size.width() / double(originalSize.width());
    }
    else
    {
        m_zoomConst = 1;
    }
}

double ImageZoomSettings::zoomFactor() const
{
    return m_zoom;
}

double ImageZoomSettings::realZoomFactor() const
{
    return (m_zoom / m_zoomRatio);
}

QSizeF ImageZoomSettings::imageSize() const
{
    return m_size;
}

QSizeF ImageZoomSettings::originalImageSize() const
{
    return (m_size / m_zoomConst);
}

QSizeF ImageZoomSettings::zoomedSize() const
{
    return (m_size / (m_zoomConst * m_zoomRatio) * m_zoom);
}

QRectF ImageZoomSettings::sourceRect(const QRectF& imageRect) const
{
    return mapZoomToImage(imageRect);
}

bool ImageZoomSettings::isFitToSize(const QSizeF& frameSize) const
{
    return (zoomFactor() == fitToSizeZoomFactor(frameSize));
}

void ImageZoomSettings::setZoomFactor(double zoom)
{
    m_zoom = zoom;
}

void ImageZoomSettings::fitToSize(const QSizeF& frameSize, FitToSizeMode mode)
{
    setZoomFactor(fitToSizeZoomFactor(frameSize, mode));
}

double ImageZoomSettings::fitToSizeZoomFactor(const QSizeF& frameSize, FitToSizeMode mode) const
{
    if (!frameSize.isValid() || !m_size.isValid())
    {
        return 1;
    }

    double zoom;

    if ((frameSize.width() / frameSize.height()) < (m_size.width() / m_size.height()))
    {
        zoom = m_zoomConst * m_zoomRatio * frameSize.width() / m_size.width();
    }
    else
    {
        zoom = m_zoomConst * m_zoomRatio * frameSize.height() / m_size.height();
    }

    // Zoom rounding down and scroll bars are never activated.

    zoom = floor(zoom * 100000 - 0.1) / 100000.0;

    if (mode == OnlyScaleDown)
    {
        // OnlyScaleDown: accept that an image is smaller than available space, don't scale up

        if ((frameSize.width() > originalImageSize().width()) && (frameSize.height() > originalImageSize().height()))
        {
            zoom = 1;
        }
    }

    return zoom;
}

QRectF ImageZoomSettings::mapZoomToImage(const QRectF& zoomedRect) const
{
    return QRectF(zoomedRect.topLeft() / (m_zoom / (m_zoomConst * m_zoomRatio)),
                  zoomedRect.size()    / (m_zoom / (m_zoomConst * m_zoomRatio)));
}

QRectF ImageZoomSettings::mapImageToZoom(const QRectF& imageRect) const
{
    return QRectF(imageRect.topLeft() * (m_zoom / (m_zoomConst * m_zoomRatio)),
                  imageRect.size()    * (m_zoom / (m_zoomConst * m_zoomRatio)));
}

QPointF ImageZoomSettings::mapZoomToImage(const QPointF& zoomedPoint) const
{
    return (zoomedPoint / (m_zoom / (m_zoomConst * m_zoomRatio)));
}

QPointF ImageZoomSettings::mapImageToZoom(const QPointF& imagePoint) const
{
    return (imagePoint * (m_zoom / (m_zoomConst * m_zoomRatio)));
}

inline static bool lessThanLimitedPrecision(double a, double b)
{
    return std::lround(a * 100000) < std::lround(b * 100000);
}

double ImageZoomSettings::snappedZoomStep(double nextZoom, const QSizeF& frameSize) const
{
    // If the zoom value gets changed from d->zoom to zoom
    // across 50%, 100% or fit-to-window, then return the
    // the corresponding special value. Otherwise zoom is returned unchanged.

    QList<double> snapValues;
    snapValues << 0.5;
    snapValues << 1.0;

    if (frameSize.isValid())
    {
        snapValues << fitToSizeZoomFactor(frameSize);
    }

    double currentZoom = zoomFactor();

    if (currentZoom < nextZoom)
    {
        foreach (double z, snapValues)
        {
            if (lessThanLimitedPrecision(currentZoom, z) && lessThanLimitedPrecision(z, nextZoom))
            {
                return z;
            }
        }
    }
    else
    {
        foreach (double z, snapValues)
        {
            if (lessThanLimitedPrecision(z, currentZoom) && lessThanLimitedPrecision(nextZoom, z))
            {
                return z;
            }
        }
    }

    return nextZoom;
}

double ImageZoomSettings::snappedZoomFactor(double zoom, const QSizeF& frameSize) const
{
    QList<double> snapValues;
    snapValues << 0.5;
    snapValues << 1.0;

    if (frameSize.isValid())
    {
        snapValues << fitToSizeZoomFactor(frameSize);
    }

    foreach (double z, snapValues)
    {
        if (fabs(zoom - z) < 0.05)
        {
            return z;
        }
    }

    return zoom;
}


DImgViewer::DImgViewer(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setRenderTarget(RenderTarget::FramebufferObject);

    setOpaquePainting(true);
    setFillColor(Qt::black);
}

void DImgViewer::paint(QPainter *painter)
{
    /*
    TODO
    Example: color management, raw decoding, metadata

    Digikam::DMetadata data;
    data.load(QStringLiteral("redtruck_GBR.jpg"));
    auto fields = data.getMetadataField(Digikam::MetadataInfo::Model).toString();
    auto fields2 = data.getPhotographInformation();

    Digikam::DRawDecoderSettings sett;
    sett.expoCorrection = true;
    sett.whiteBalanceArea = QRect{0,0,5,5};
    sett.whiteBalance = Digikam::DRawDecoderSettings::WhiteBalance::AERA;
    Digikam::DRawDecoding ecod{sett};

    Digikam::DImg scaledImage(QStringLiteral("redtruck_GBR.jpg"), nullptr, ecod);

    Digikam::IccManager m{scaledImage};
    m.transformForDisplay();
    auto pix                      = scaledImage.convertToPixmap();
    painter->drawPixmap(0, 0, pix);
    */

    QRectF exposedRect = painter->clipBoundingRect();
    if(exposedRect.isNull())
    {
        exposedRect = boundingRect();
    }
    QRect boundingRect = QRectF(QPointF(0, 0), m_zoomSettings.zoomedSize()).toAlignedRect();
    QRect   drawRect     = exposedRect.intersected(boundingRect).toAlignedRect();
    QRect   pixSourceRect;
    QPixmap pix;
    QSize   completeSize = boundingRect.size();

    /* For high resolution ("retina") displays, Mac OS X / Qt
     * report only half of the physical resolution in terms of
     * pixels, i.e. every logical pixels corresponds to 2x2
     * physical pixels. However, UI elements and fonts are
     * nevertheless rendered at full resolution, and pixmaps
     * as well, provided their resolution is high enough (that
     * is, higher than the reported, logical resolution).
     *
     * To work around this, we render the photos not a logical
     * resolution, but with the photo's full resolution, but
     * at the screen's aspect ratio. When we later draw this
     * high resolution bitmap, it is up to Qt to scale the
     * photo to the true physical resolution.  The ratio
     * computed below is the ratio between the photo and
     * screen resolutions, or equivalently the factor by which
     * we need to increase the pixel size of the rendered
     * pixmap.
     */

    const double ratio          = qApp->devicePixelRatio();

    QRect  scaledDrawRect = QRectF(ratio * drawRect.x(),
                                   ratio * drawRect.y(),
                                   ratio * drawRect.width(),
                                   ratio * drawRect.height()).toRect();

    if (findInCache(scaledDrawRect, &pix, &pixSourceRect))
    {
        if (pixSourceRect.isNull())
        {
            painter->drawPixmap(drawRect, pix);
        }
        else
        {
            painter->drawPixmap(drawRect, pix, pixSourceRect);
        }
    }
    else
    {
        // scale "as if" scaling to whole image, but clip output to our exposed region

        QSize scaledCompleteSize = QSizeF(ratio * completeSize.width(),
                                          ratio * completeSize.height()).toSize();
        Digikam::DImg scaledImage         = m_image.smoothScaleClipped(scaledCompleteSize.width(),
                                                               scaledCompleteSize.height(),
                                                               scaledDrawRect.x(),
                                                               scaledDrawRect.y(),
                                                               scaledDrawRect.width(),
                                                               scaledDrawRect.height());
        pix                      = scaledImage.convertToPixmap();
        insertIntoCache(scaledDrawRect, pix);
        painter->drawPixmap(drawRect, pix);
    }
}

void DImgViewer::setSource(const QString &source)
{
    if(m_source != source)
    {
        m_source = source;
        setImage(Digikam::DImg{m_source});
        emit sourceChanged(source);
    }
}

void DImgViewer::setImage(Digikam::DImg image)
{
    this->m_image = image;
    m_zoomSettings.setImageSize(image.size(), image.originalSize());
    clearCache();
    update();
}

QString DImgViewer::source() const
{
    return m_source;
}

void DImgViewer::fitToSize()
{
    m_zoomSettings.fitToSize(size());
    update();
}

ImageZoomSettings &DImgViewer::zoomSettings()
{
    return m_zoomSettings;
}

DImgViewer::~DImgViewer()
{
    clearCache();
}

bool DImgViewer::findInCache(const QRect &region, QPixmap * const pix, QRect * const source)
{
    decltype(m_cacheKeys)::iterator key;

    for (key = m_cacheKeys.begin(); key != m_cacheKeys.end();)
    {
        if (!key->first.contains(region))
        {
            ++key;
            continue;
        }

        if (!QPixmapCache::find(key->second, pix))
        {
            key = m_cacheKeys.erase(key);
            continue;
        }

        if (key->first == region)
        {
            *source = QRect();
        }
        else
        {
            QPoint startPoint = region.topLeft() - key->first.topLeft();
            *source           = QRect(startPoint, region.size());
        }

        return true;
    }

    return false;
}

void DImgViewer::insertIntoCache(const QRect &region, const QPixmap &pixmap)
{
    if (m_cacheKeys.size() >= CACHE_MAX_COUNT)
    {
        const auto key = m_cacheKeys.dequeue();
        QPixmapCache::remove(key.second);
    }

    m_cacheKeys.enqueue({region, QPixmapCache::insert(pixmap)});
}

void DImgViewer::clearCache()
{
    for(const auto& p: std::as_const(m_cacheKeys))
    {
        QPixmapCache::remove(p.second);
    }
    m_cacheKeys.clear();
}
