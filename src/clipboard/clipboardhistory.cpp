#include "clipboardhistory.h"
#include "clipboarddetect.h"
#include "clipboardroles.h"
#include "klipperbridge.h"
#include "systemclipboard.h"

#include "kboardsettings.h"

#include <KLocalizedString>

#include <algorithm>
#include <ranges>

ClipboardHistory::ClipboardHistory(const Options &options, QObject *parent)
    : QAbstractListModel(parent)
    , m_options(options)
    , m_images(options.cacheDirectory)
    , m_file(options.dataDirectory + QStringLiteral("/history.json"))
    , m_paster(m_images)
{
    m_guard.setSensitiveProbe(m_options.sensitiveProbe);
    if (m_options.clock) {
        m_expiry.setClock(m_options.clock);
    }
    connect(&m_expiry, &ClipboardExpiry::due, this, &ClipboardHistory::expireDue);
    connect(&m_latest, &ClipboardLatest::valueChanged, this, &ClipboardHistory::latestChanged);
    connect(&m_latest, &ClipboardLatest::freshChanged, this, &ClipboardHistory::latestFreshChanged);
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(250);
    connect(&m_saveTimer, &QTimer::timeout, this, &ClipboardHistory::saveNow);
    connectSettings();
    load();
    if (m_options.watchSystem) {
        startWatching();
    }
}

ClipboardHistory::~ClipboardHistory()
{
    if (m_saveTimer.isActive()) {
        saveNow();
    }
}

void ClipboardHistory::connectSettings()
{
    auto settings = KBoardSettings::self();
    connect(settings, &KBoardSettings::clipboardExpiryMinutesChanged, this, [this] {
        clearExpiringFlags();
        expireDue();
    });
    connect(settings, &KBoardSettings::clipboardMaxItemsChanged, this, &ClipboardHistory::enforceLimit);
    connect(settings, &KBoardSettings::clipboardPersistChanged, this, &ClipboardHistory::saveNow);
    connect(settings, &KBoardSettings::clipboardPausedChanged, this, &ClipboardHistory::pausedChanged);
    connect(settings, &KBoardSettings::clipboardEnabledChanged, this, &ClipboardHistory::enabledChanged);
}

void ClipboardHistory::startWatching()
{
    auto system = new SystemClipboard(this);
    m_watching = system->start();
    if (!m_watching) {
        setError(system->errorString());
    }
    m_paster.setSystemClipboard(system);
    connect(system, &SystemClipboard::copied, this, &ClipboardHistory::addMimeData);

    m_klipper = new KlipperBridge(this);
    connect(m_klipper, &KlipperBridge::connectedChanged, this, &ClipboardHistory::klipperConnectedChanged);
    connect(m_klipper, &KlipperBridge::historyReceived, this, &ClipboardHistory::mergeKlipperHistory);
    connect(KBoardSettings::self(), &KBoardSettings::clipboardImportKlipperChanged, m_klipper, [this] { m_klipper->fetch(false); });
    m_klipper->fetch(false);
}

int ClipboardHistory::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : int(m_entries.size());
}

QVariant ClipboardHistory::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return {};
    }
    return ClipboardRoles::value(m_entries.at(index.row()), role);
}

int ClipboardHistory::pinnedCount() const
{
    return int(std::ranges::count_if(m_entries, &ClipboardEntry::pinned));
}

void ClipboardHistory::setPaused(bool paused)
{
    if (paused != isPaused()) {
        KBoardSettings::setClipboardPaused(paused);
        KBoardSettings::self()->save();
    }
}

bool ClipboardHistory::isKlipperConnected() const
{
    return m_klipper && m_klipper->isConnected();
}

bool ClipboardHistory::addMimeData(const QMimeData *mime)
{
    const ClipboardDetect::CopiedContent content = ClipboardDetect::read(mime);
    if (content.secret) {
        m_guard.block(content.text);
        return false;
    }
    return content.image.isNull() ? addText(content.text) : addImage(content.image, content.imageMimeType);
}

bool ClipboardHistory::addText(const QString &text)
{
    if (text.trimmed().isEmpty() || !m_guard.allowsCopy(text)) {
        return false;
    }
    m_guard.forgive(text);
    const int existing = findText(text);
    if (existing >= 0) {
        return bumpExisting(existing);
    }
    insertEntry(ClipboardEntry::forText(text, m_expiry.now(), QStringLiteral("clipboard")), true);
    return true;
}

bool ClipboardHistory::addImage(const QImage &image, const QString &mimeType)
{
    if (image.isNull() || !m_guard.allowsCopy(QString())) {
        return false;
    }
    const QByteArray hash = ClipboardImageStore::fingerprint(image);
    const auto existing = std::ranges::find(m_entries, hash, &ClipboardEntry::imageHash);
    if (existing != m_entries.end()) {
        return bumpExisting(int(existing - m_entries.begin()));
    }
    ClipboardEntry entry = ClipboardEntry::forImage(hash, image.size(), mimeType, m_expiry.now(), QStringLiteral("clipboard"));
    m_images.insert(entry.id, image);
    insertEntry(std::move(entry), true);
    return true;
}

void ClipboardHistory::mergeKlipperHistory(const QStringList &history, bool onlyNewest)
{
    const QStringList items = onlyNewest ? history.mid(0, 1) : history;
    for (const QString &text : std::views::reverse(items)) {
        const bool accepted = m_guard.allowsImport(text) && findText(text) < 0 && (!onlyNewest || m_guard.allowsCopy(text));
        if (accepted) {
            insertEntry(ClipboardEntry::forText(text, m_expiry.now(), QStringLiteral("klipper")), false);
        }
    }
}

int ClipboardHistory::findText(const QString &text) const
{
    const auto it = std::ranges::find_if(m_entries, [&text](const ClipboardEntry &entry) { return !entry.image && entry.text == text; });
    return it == m_entries.cend() ? -1 : int(it - m_entries.cbegin());
}

void ClipboardHistory::insertEntry(ClipboardEntry entry, bool announce)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_entries.prepend(std::move(entry));
    endInsertRows();
    const ClipboardEntry &inserted = m_entries.constFirst();
    if (inserted.image && shouldPersist(inserted)) {
        m_images.persist(inserted.id);
    }
    if (announce) {
        m_latest.announce(inserted.id, ClipboardRoles::toMap(inserted));
    }
    enforceLimit();
    Q_EMIT countChanged();
    entriesChanged();
}

bool ClipboardHistory::bumpExisting(int row)
{
    m_entries[row].timestamp = m_expiry.now();
    m_entries[row].expiring = false;
    if (row > 0) {
        beginMoveRows(QModelIndex(), row, row, QModelIndex(), 0);
        m_entries.move(row, 0);
        endMoveRows();
    }
    Q_EMIT dataChanged(index(0), index(0), {ClipboardRoles::TimestampRole, ClipboardRoles::ExpiringRole});
    m_latest.announce(m_entries.constFirst().id, ClipboardRoles::toMap(m_entries.constFirst()));
    entriesChanged();
    return true;
}

void ClipboardHistory::removeEntries(QList<int> rows)
{
    if (rows.isEmpty()) {
        return;
    }
    std::ranges::sort(rows, std::greater<>());
    for (int row : std::as_const(rows)) {
        const ClipboardEntry entry = m_entries.at(row);
        beginRemoveRows(QModelIndex(), row, row);
        m_entries.removeAt(row);
        endRemoveRows();
        if (entry.image) {
            m_images.drop(entry.id);
        } else {
            m_guard.dismiss(entry.text);
        }
        m_latest.forget(entry.id);
    }
    Q_EMIT countChanged();
    entriesChanged();
}

void ClipboardHistory::enforceLimit()
{
    const int limit = KBoardSettings::clipboardMaxItems();
    QList<int> excess;
    const int total = int(m_entries.size());
    for (int row = total - 1; row >= 0 && total - int(excess.size()) > limit; --row) {
        if (!m_entries.at(row).pinned) {
            excess.append(row);
        }
    }
    removeEntries(excess);
}

void ClipboardHistory::entriesChanged()
{
    m_expiry.schedule(m_entries);
    m_saveTimer.start();
}

void ClipboardHistory::clearExpiringFlags()
{
    for (ClipboardEntry &entry : m_entries) {
        entry.expiring = false;
    }
    if (!m_entries.isEmpty()) {
        Q_EMIT dataChanged(index(0), index(int(m_entries.size()) - 1), {ClipboardRoles::ExpiringRole});
    }
}

void ClipboardHistory::expireDue()
{
    const ClipboardExpiry::Sweep sweep = m_expiry.sweep(m_entries);
    for (int row : sweep.expiring) {
        m_entries[row].expiring = true;
        Q_EMIT dataChanged(index(row), index(row), {ClipboardRoles::ExpiringRole});
    }
    removeEntries(sweep.expired);
    m_expiry.schedule(m_entries);
}

bool ClipboardHistory::shouldPersist(const ClipboardEntry &entry)
{
    return entry.pinned || KBoardSettings::clipboardPersist();
}

void ClipboardHistory::saveNow()
{
    m_saveTimer.stop();
    QList<ClipboardEntry> persisted;
    QSet<QString> keepImages;
    for (const ClipboardEntry &entry : std::as_const(m_entries)) {
        if (shouldPersist(entry)) {
            persisted.append(entry);
        }
        if (entry.image && shouldPersist(entry)) {
            keepImages.insert(entry.id);
            m_images.persist(entry.id);
        }
    }
    setError(m_file.save(persisted) ? QString() : m_file.errorString());
    m_images.prune(keepImages);
}

void ClipboardHistory::load()
{
    auto loaded = m_file.load();
    if (!loaded) {
        setError(m_file.errorString());
        return;
    }
    loaded->removeIf([this](const ClipboardEntry &entry) { return entry.image && !m_images.restore(entry.id); });
    beginResetModel();
    m_entries = std::move(*loaded);
    endResetModel();
    Q_EMIT countChanged();
    expireDue();
}

void ClipboardHistory::setError(const QString &error)
{
    if (m_errorString != error) {
        m_errorString = error;
        Q_EMIT errorStringChanged();
    }
}

bool ClipboardHistory::reportPaster(bool ok)
{
    setError(ok ? QString() : m_paster.errorString());
    return ok;
}

int ClipboardHistory::rowForId(const QString &id) const
{
    const auto it = std::ranges::find(m_entries, id, &ClipboardEntry::id);
    return it == m_entries.cend() ? -1 : int(it - m_entries.cbegin());
}

QVariantMap ClipboardHistory::get(int row) const
{
    return isValidRow(row) ? ClipboardRoles::toMap(m_entries.at(row)) : QVariantMap();
}

bool ClipboardHistory::paste(int row)
{
    if (!isValidRow(row) || !reportPaster(m_paster.paste(m_entries.at(row)))) {
        return false;
    }
    Q_EMIT pasted(row);
    return true;
}

bool ClipboardHistory::pasteLatest()
{
    const int row = rowForId(m_latest.id());
    if (row < 0) {
        setError(i18n("The recent copy is no longer in the clipboard history."));
        return false;
    }
    m_latest.dismiss();
    return paste(row);
}

bool ClipboardHistory::copyToClipboard(int row)
{
    return isValidRow(row) && reportPaster(m_paster.copy(m_entries.at(row)));
}

void ClipboardHistory::pin(int row, bool pinned)
{
    if (!isValidRow(row) || m_entries[row].pinned == pinned) {
        return;
    }
    ClipboardEntry &entry = m_entries[row];
    entry.pinned = pinned;
    entry.expiring = false;
    if (!pinned) {
        entry.timestamp = m_expiry.now();
    }
    Q_EMIT dataChanged(index(row), index(row), {ClipboardRoles::PinnedRole, ClipboardRoles::ExpiringRole, ClipboardRoles::TimestampRole});
    m_latest.refresh(entry.id, ClipboardRoles::toMap(entry));
    Q_EMIT countChanged();
    entriesChanged();
}

void ClipboardHistory::remove(int row)
{
    if (isValidRow(row)) {
        removeEntries({row});
    }
}

void ClipboardHistory::clear(bool includePinned)
{
    QList<int> rows;
    for (int row = 0; row < m_entries.size(); ++row) {
        if (includePinned || !m_entries.at(row).pinned) {
            rows.append(row);
        }
    }
    removeEntries(rows);
}
