#pragma once

#include <QHash>
#include <QList>
#include <QSet>
#include <QString>

namespace DescriptorScanner
{
constexpr int NetlinkKobjectUevent = 15;

using Snapshot = QHash<int, QString>;

Snapshot snapshot(const QString &fdDirectory = QStringLiteral("/proc/self/fd"));
QSet<quint64> ueventSocketInodes(const QString &netlinkTable = QStringLiteral("/proc/self/net/netlink"));
bool isInputTarget(const QString &target, const QSet<quint64> &ueventInodes);
QList<int> inputDescriptors(const Snapshot &baseline, const Snapshot &current, const QSet<quint64> &ueventInodes);
}
