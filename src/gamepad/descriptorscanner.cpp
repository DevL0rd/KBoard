#include "descriptorscanner.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>

#include <algorithm>
#include <array>
#include <climits>
#include <unistd.h>

namespace
{
constexpr int NetlinkProtocolColumn = 1;
constexpr int NetlinkInodeColumn = 9;

QString readLink(const QString &path)
{
    std::array<char, PATH_MAX> buffer {};
    const ssize_t length = ::readlink(QFile::encodeName(path).constData(), buffer.data(), buffer.size() - 1);
    return length > 0 ? QFile::decodeName(QByteArray(buffer.data(), length)) : QString();
}
}

namespace DescriptorScanner
{
Snapshot snapshot(const QString &fdDirectory)
{
    Snapshot result;
    const QStringList entries = QDir(fdDirectory).entryList(QDir::AllEntries | QDir::System | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        bool ok = false;
        const int descriptor = entry.toInt(&ok);
        if (ok) {
            result.insert(descriptor, readLink(fdDirectory + QLatin1Char('/') + entry));
        }
    }
    return result;
}

QSet<quint64> ueventSocketInodes(const QString &netlinkTable)
{
    QSet<quint64> inodes;
    QFile file(netlinkTable);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return inodes;
    }
    file.readLine();
    while (!file.atEnd()) {
        const QStringList columns = QString::fromLatin1(file.readLine()).split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (columns.size() > NetlinkInodeColumn && columns.at(NetlinkProtocolColumn).toInt() == NetlinkKobjectUevent) {
            inodes.insert(columns.at(NetlinkInodeColumn).toULongLong());
        }
    }
    return inodes;
}

bool isInputTarget(const QString &target, const QSet<quint64> &ueventInodes)
{
    static const QStringList devicePrefixes
        = {QStringLiteral("/dev/input/event"), QStringLiteral("/dev/hidraw"), QStringLiteral("/dev/bus/usb/")};
    const bool device = std::any_of(
        devicePrefixes.cbegin(), devicePrefixes.cend(), [&target](const QString &prefix) { return target.startsWith(prefix); });
    if (device) {
        return true;
    }
    static const QRegularExpression socketPattern(QStringLiteral("^socket:\\[(\\d+)\\]$"));
    const QRegularExpressionMatch match = socketPattern.match(target);
    return match.hasMatch() && ueventInodes.contains(match.captured(1).toULongLong());
}

QList<int> inputDescriptors(const Snapshot &baseline, const Snapshot &current, const QSet<quint64> &ueventInodes)
{
    QList<int> descriptors;
    for (auto it = current.cbegin(); it != current.cend(); ++it) {
        const auto before = baseline.constFind(it.key());
        const bool preexisting = before != baseline.cend() && before.value() == it.value();
        if (!preexisting && isInputTarget(it.value(), ueventInodes)) {
            descriptors.append(it.key());
        }
    }
    std::sort(descriptors.begin(), descriptors.end());
    return descriptors;
}
}
