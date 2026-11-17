#pragma once

#include "clipboardexpiry.h"
#include "clipboardimagestore.h"

struct ClipboardExpiryProbe
{
    static bool isActive(const ClipboardExpiry &expiry) { return expiry.m_timer.isActive(); }
    static int interval(const ClipboardExpiry &expiry) { return expiry.m_timer.interval(); }
    static QDateTime nextEventTime(const ClipboardExpiry &expiry)
    {
        return expiry.m_timer.isActive() ? expiry.now().addMSecs(expiry.m_timer.interval()) : QDateTime();
    }
};

struct ClipboardImageStoreProbe
{
    static QString directory(const ClipboardImageStore &store) { return store.m_directory; }
};
