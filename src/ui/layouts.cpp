#include "layouts.h"
#include "kboardpaths.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#include <algorithm>

namespace
{
QString defaultLabel(const QString &type)
{
    if (type == QLatin1String("space")) {
        return QStringLiteral(" ");
    }
    return QString();
}

QString upper(const QString &text)
{
    if (text.size() != 1) {
        return text;
    }
    const QString result = text.toUpper();
    return result.size() == text.size() ? result : text;
}

QString substitute(QString text, const QJsonObject &layout)
{
    return text.replace(QLatin1String("{currency}"), layout.value(QLatin1String("currency")).toString(QStringLiteral("$")));
}

bool isRepeatType(const QString &type)
{
    return type == QLatin1String("backspace") || type == QLatin1String("arrow");
}
}

Layouts::Layouts(const QString &directory, QObject *parent)
    : QObject(parent)
    , m_directory(directory)
{
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &Layouts::reload);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &Layouts::reload);
    reload();
}

Layouts *Layouts::create(QQmlEngine *, QJSEngine *)
{
    static Layouts *layouts = new Layouts(KBoardPaths::dataFile(QStringLiteral("layouts")));
    QJSEngine::setObjectOwnership(layouts, QJSEngine::CppOwnership);
    return layouts;
}

QString Layouts::loadFile(const QFileInfo &info)
{
    QFile file(info.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return QStringLiteral("%1: %2").arg(info.fileName(), file.errorString());
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return QStringLiteral("%1: %2 at offset %3").arg(info.fileName(), parseError.errorString()).arg(parseError.offset);
    }
    const QJsonObject object = document.object();
    const QString kind = object.value(QLatin1String("kind")).toString();
    const QString id = object.value(QLatin1String("id")).toString(info.completeBaseName());
    if (kind == QLatin1String("letters")) {
        m_letters.insert(id, object);
    } else if (kind == QLatin1String("page")) {
        m_pages.insert(id, object);
    } else if (kind == QLatin1String("rows")) {
        const QJsonObject rows = object.value(QLatin1String("rows")).toObject();
        for (auto it = rows.begin(); it != rows.end(); ++it) {
            m_rows.insert(it.key(), it.value());
        }
    } else {
        return QStringLiteral("%1: unknown kind \"%2\"").arg(info.fileName(), kind);
    }
    return {};
}

void Layouts::watchFiles(const QFileInfoList &files)
{
    const QStringList watched = m_watcher.files() + m_watcher.directories();
    if (!watched.isEmpty()) {
        m_watcher.removePaths(watched);
    }
    if (!QDir(m_directory).exists()) {
        return;
    }
    m_watcher.addPath(m_directory);
    for (const QFileInfo &info : files) {
        m_watcher.addPath(info.absoluteFilePath());
    }
}

void Layouts::sortLayouts()
{
    m_order = m_letters.keys();
    std::sort(m_order.begin(), m_order.end(), [this](const QString &a, const QString &b) {
        return m_letters.value(a)
                   .value(QLatin1String("name"))
                   .toString()
                   .localeAwareCompare(m_letters.value(b).value(QLatin1String("name")).toString())
            < 0;
    });
}

void Layouts::reload()
{
    m_letters.clear();
    m_pages.clear();
    m_rows = {};
    m_order.clear();

    QStringList errors;
    const QDir dir(m_directory);
    if (!dir.exists()) {
        errors << QStringLiteral("Layout directory %1 does not exist").arg(m_directory);
    }
    const QFileInfoList files = dir.entryInfoList({QStringLiteral("*.json")}, QDir::Files, QDir::Name);
    for (const QFileInfo &info : files) {
        const QString error = loadFile(info);
        if (!error.isEmpty()) {
            errors << error;
        }
    }
    sortLayouts();
    if (m_letters.isEmpty()) {
        errors << QStringLiteral("No letter layouts found in %1").arg(m_directory);
    }
    watchFiles(files);

    m_errorString = errors.join(QLatin1Char('\n'));
    if (!m_errorString.isEmpty()) {
        qWarning().noquote() << "KBoard layouts:" << m_errorString;
    }
    ++m_revision;
    Q_EMIT changed();
}

QVariantList Layouts::available() const
{
    QVariantList list;
    for (const QString &id : m_order) {
        list << layout(id);
    }
    return list;
}

QString Layouts::errorString() const
{
    return m_errorString;
}

QString Layouts::directory() const
{
    return m_directory;
}

int Layouts::revision() const
{
    return m_revision;
}

bool Layouts::has(const QString &id) const
{
    return m_letters.contains(id);
}

QStringList Layouts::pages() const
{
    QStringList ids = m_pages.keys();
    ids.sort();
    return ids;
}

QVariantMap Layouts::layout(const QString &id) const
{
    if (!m_letters.contains(id)) {
        return {};
    }
    const QJsonObject object = m_letters.value(id);
    return {
        {QStringLiteral("id"), id},
        {QStringLiteral("name"), object.value(QLatin1String("name")).toString(id)},
        {QStringLiteral("shortName"), object.value(QLatin1String("shortName")).toString(id.toUpper())},
        {QStringLiteral("language"), object.value(QLatin1String("language")).toString()},
        {QStringLiteral("locale"), object.value(QLatin1String("locale")).toString()},
        {QStringLiteral("currency"), object.value(QLatin1String("currency")).toString(QStringLiteral("$"))},
    };
}

QJsonObject Layouts::namedRow(const QJsonObject &layout, const QString &name) const
{
    const QJsonObject overrides = layout.value(QLatin1String("rows")).toObject();
    if (overrides.contains(name)) {
        return overrides.value(name).toObject();
    }
    return m_rows.value(name).toObject();
}

bool Layouts::conditionHolds(const QString &condition, const QVariantMap &options) const
{
    if (condition.isEmpty()) {
        return true;
    }
    if (condition == QLatin1String("multipleLayouts")) {
        return options.value(QStringLiteral("multipleLayouts")).toBool();
    }
    if (condition == QLatin1String("singleLayout")) {
        return !options.value(QStringLiteral("multipleLayouts")).toBool();
    }
    if (condition.startsWith(QLatin1String("variant:"))) {
        return options.value(QStringLiteral("variant")).toString() == condition.mid(8);
    }
    return false;
}

QVariantMap Layouts::resolveKey(const QJsonObject &layout, const QJsonObject &key, bool rowGlide, bool rowSpecial) const
{
    QVariantMap result;
    const QString type = key.value(QLatin1String("type")).toString(QStringLiteral("char"));
    const QString label = substitute(key.value(QLatin1String("label")).toString(defaultLabel(type)), layout);
    const QString output = substitute(key.value(QLatin1String("output")).toString(label), layout);
    const QString shiftLabel = substitute(key.value(QLatin1String("shiftLabel")).toString(upper(label)), layout);
    const QString shiftOutput = substitute(
        key.value(QLatin1String("shiftOutput")).toString(key.contains(QLatin1String("shiftLabel")) ? shiftLabel : upper(output)), layout);

    QVariantList longPress;
    QVariantList shiftLongPress;
    const QJsonArray entries = key.value(QLatin1String("longPress")).toArray();
    for (const auto &entry : entries) {
        QVariantMap item;
        if (entry.isObject()) {
            const QJsonObject object = entry.toObject();
            const QString itemLabel = substitute(object.value(QLatin1String("label")).toString(), layout);
            item = {
                {QStringLiteral("label"), itemLabel},
                {QStringLiteral("output"), substitute(object.value(QLatin1String("output")).toString(itemLabel), layout)},
                {QStringLiteral("action"), object.value(QLatin1String("action")).toString()},
            };
        } else {
            const QString text = substitute(entry.toString(), layout);
            item = {{QStringLiteral("label"), text}, {QStringLiteral("output"), text}, {QStringLiteral("action"), QString()}};
        }
        longPress << item;
        QVariantMap shifted = item;
        if (shifted.value(QStringLiteral("action")).toString().isEmpty()) {
            shifted[QStringLiteral("label")] = upper(item.value(QStringLiteral("label")).toString());
            shifted[QStringLiteral("output")] = upper(item.value(QStringLiteral("output")).toString());
        }
        shiftLongPress << shifted;
    }

    const bool letter = type == QLatin1String("char") && label.size() == 1 && label.at(0).isLetter();
    result[QStringLiteral("type")] = type;
    result[QStringLiteral("label")] = label;
    result[QStringLiteral("output")] = output;
    result[QStringLiteral("shiftLabel")] = shiftLabel;
    result[QStringLiteral("shiftOutput")] = shiftOutput;
    result[QStringLiteral("sublabel")] = key.value(QLatin1String("sublabel")).toString();
    result[QStringLiteral("width")] = key.value(QLatin1String("width")).toDouble(1.0);
    result[QStringLiteral("grow")] = key.value(QLatin1String("grow")).toBool(false);
    result[QStringLiteral("longPress")] = longPress;
    result[QStringLiteral("shiftLongPress")] = shiftLongPress;
    result[QStringLiteral("hint")] = substitute(key.value(QLatin1String("hint")).toString(), layout);
    result[QStringLiteral("icon")] = key.value(QLatin1String("icon")).toString(type);
    result[QStringLiteral("key")] = key.value(QLatin1String("key")).toString();
    result[QStringLiteral("modifier")] = key.value(QLatin1String("modifier")).toString();
    result[QStringLiteral("page")] = key.value(QLatin1String("page")).toString();
    result[QStringLiteral("action")] = key.value(QLatin1String("action")).toString();
    result[QStringLiteral("glide")] = key.value(QLatin1String("glide")).toBool(rowGlide && letter);
    result[QStringLiteral("letter")] = letter;
    result[QStringLiteral("repeat")] = key.value(QLatin1String("repeat")).toBool(isRepeatType(type));
    result[QStringLiteral("special")]
        = key.value(QLatin1String("special")).toBool(rowSpecial || (type != QLatin1String("char") && type != QLatin1String("space")));
    return result;
}

QVariantMap Layouts::resolveRow(const QJsonObject &layout, const QJsonObject &row, const QVariantMap &options, int columns) const
{
    const bool glide = row.value(QLatin1String("glide")).toBool(true);
    const bool special = row.value(QLatin1String("special")).toBool(false);
    QVariantList keys;
    double units = 0;
    int growIndex = -1;
    const QJsonArray entries = row.value(QLatin1String("keys")).toArray();
    for (const auto &entry : entries) {
        const QJsonObject object = entry.toObject();
        if (!conditionHolds(object.value(QLatin1String("when")).toString(), options)) {
            continue;
        }
        QVariantMap key = resolveKey(layout, object, glide, special);
        units += key.value(QStringLiteral("width")).toDouble();
        if (key.value(QStringLiteral("grow")).toBool() && growIndex < 0) {
            growIndex = keys.size();
        }
        keys << key;
    }
    if (growIndex >= 0 && units < columns) {
        QVariantMap key = keys.at(growIndex).toMap();
        key[QStringLiteral("width")] = key.value(QStringLiteral("width")).toDouble() + (columns - units);
        keys[growIndex] = key;
    }
    QVariantMap result;
    result[QStringLiteral("keys")] = keys;
    result[QStringLiteral("height")] = row.value(QLatin1String("height")).toDouble(1.0);
    result[QStringLiteral("split")] = row.value(QLatin1String("split")).toInt(-1);
    return result;
}

QVariantList Layouts::resolveRows(const QJsonObject &layout, const QJsonArray &rows, const QVariantMap &options, int columns) const
{
    QVariantList result;
    const QString variant = options.value(QStringLiteral("variant")).toString();
    for (const auto &value : rows) {
        QJsonObject row = value.toObject();
        if (row.contains(QLatin1String("include"))) {
            const QString name = row.value(QLatin1String("include")).toString();
            const QJsonObject variantRow = variant.isEmpty() ? QJsonObject() : namedRow(layout, name + QLatin1Char('-') + variant);
            row = variantRow.isEmpty() ? namedRow(layout, name) : variantRow;
            if (row.isEmpty()) {
                qWarning().noquote() << "KBoard layouts: missing row" << name;
                continue;
            }
        }
        result << resolveRow(layout, row, options, columns);
    }
    return result;
}

QVariantMap Layouts::page(const QString &layoutId, const QString &pageId, const QVariantMap &options) const
{
    if (!m_letters.contains(layoutId)) {
        return {};
    }
    const QJsonObject source = m_letters.value(layoutId);
    QJsonObject pageObject;
    QJsonArray rows;
    int columns = 10;
    QString name;
    const bool letters = pageId == QLatin1String("letters");
    if (letters) {
        rows = source.value(QLatin1String("letters")).toArray();
        columns = source.value(QLatin1String("columns")).toInt(10);
        name = source.value(QLatin1String("name")).toString(layoutId);
    } else {
        const QJsonObject overrides = source.value(QLatin1String("pages")).toObject();
        pageObject = overrides.contains(pageId) ? overrides.value(pageId).toObject() : m_pages.value(pageId);
        if (pageObject.isEmpty()) {
            return {};
        }
        rows = pageObject.value(QLatin1String("rows")).toArray();
        columns = pageObject.value(QLatin1String("columns")).toInt(10);
        name = pageObject.value(QLatin1String("name")).toString(pageId);
    }

    const bool compact = pageId == QLatin1String("numpad") || pageId == QLatin1String("phone");
    QJsonArray prefix;
    if (!compact && options.value(QStringLiteral("desktopRow")).toBool()) {
        prefix.append(QJsonObject {{QStringLiteral("include"),
            options.value(QStringLiteral("functionRow")).toBool() ? QStringLiteral("function") : QStringLiteral("desktop")}});
    }
    if (letters && options.value(QStringLiteral("numberRow")).toBool()) {
        prefix.append(QJsonObject {{QStringLiteral("include"), QStringLiteral("numbers")}});
    }
    for (const auto &row : std::as_const(rows)) {
        prefix.append(row);
    }

    QVariantMap result;
    result[QStringLiteral("id")] = pageId;
    result[QStringLiteral("layout")] = layout(layoutId);
    result[QStringLiteral("name")] = name;
    result[QStringLiteral("columns")] = columns;
    result[QStringLiteral("rows")] = resolveRows(source, prefix, options, columns);
    return result;
}
