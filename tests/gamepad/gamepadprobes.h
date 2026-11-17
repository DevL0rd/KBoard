#pragma once

#include "navigationrepeat.h"
#include "steamdedupe.h"

struct NavigationRepeaterProbe
{
    static int step(const NavigationRepeater &repeater) { return repeater.m_step; }
    static bool isRepeating(const NavigationRepeater &repeater) { return repeater.m_timer.isActive(); }
    static int pendingDelay(const NavigationRepeater &repeater) { return repeater.m_timer.isActive() ? repeater.m_timer.interval() : -1; }
};

struct SteamDedupeProbe
{
    static quint32 physicalFor(const SteamDedupe &dedupe, quint32 virtualId) { return dedupe.m_virtualToPhysical.value(virtualId, 0); }

    static bool hasUnpairedVirtual(const SteamDedupe &dedupe)
    {
        for (const SteamDedupe::Device &device : dedupe.m_devices) {
            if (device.steamVirtual && !dedupe.isPairedVirtual(device.id)) {
                return true;
            }
        }
        return false;
    }
};
