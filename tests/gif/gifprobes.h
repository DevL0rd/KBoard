#pragma once

#include "gifstore.h"

struct GifStoreProbe
{
    static KlipyClient *client(const GifStore &store) { return store.m_client; }
};
