#include "klipyapi.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace Klipy
{

namespace
{

QString tr(const char *text)
{
    return QCoreApplication::translate("Klipy", text);
}

QString text(const QJsonObject &object, const char *key)
{
    return object.value(QLatin1String(key)).toString();
}

Rendition parseRendition(const QJsonValue &value)
{
    const QJsonObject object = value.toObject();
    return {text(object, "url"), object.value(QStringLiteral("width")).toInt(), object.value(QStringLiteral("height")).toInt(),
        object.value(QStringLiteral("size")).toInteger()};
}

QJsonObject renditionJson(const Rendition &rendition)
{
    return {{QStringLiteral("url"), rendition.url}, {QStringLiteral("width"), rendition.width},
        {QStringLiteral("height"), rendition.height}, {QStringLiteral("size"), rendition.size}};
}

Rendition pick(const QJsonObject &files, std::initializer_list<std::pair<const char *, const char *>> order)
{
    for (const auto &[size, format] : order) {
        Rendition rendition = parseRendition(files.value(QLatin1String(size)).toObject().value(QLatin1String(format)));
        if (!rendition.url.isEmpty()) {
            return rendition;
        }
    }
    return {};
}

void collectMessages(const QJsonValue &value, QStringList *messages)
{
    if (value.isString()) {
        messages->append(value.toString());
        return;
    }
    const QVariantList children = value.isArray() ? value.toArray().toVariantList() : value.toObject().toVariantMap().values();
    for (const QVariant &child : children) {
        collectMessages(QJsonValue::fromVariant(child), messages);
    }
}

bool openEnvelope(const QByteArray &json, QJsonValue *data, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        *error = tr("KLIPY sent an unreadable response (%1)").arg(parseError.errorString());
        return false;
    }
    if (!document.object().value(QStringLiteral("result")).toBool()) {
        const QString message = errorMessage(json);
        *error = message.isEmpty() ? tr("KLIPY rejected the request") : message;
        return false;
    }
    *data = document.object().value(QStringLiteral("data"));
    return true;
}

}

bool Item::isValid() const
{
    return !id.isEmpty() && !slug.isEmpty() && !preview.url.isEmpty() && !shareGif.url.isEmpty();
}

QJsonObject Item::toJson() const
{
    return {{QStringLiteral("id"), id}, {QStringLiteral("slug"), slug}, {QStringLiteral("title"), title}, {QStringLiteral("type"), type},
        {QStringLiteral("blur_preview"), blurPreview}, {QStringLiteral("preview"), renditionJson(preview)},
        {QStringLiteral("share_gif"), renditionJson(shareGif)}, {QStringLiteral("link_gif"), renditionJson(linkGif)}};
}

Item Item::fromJson(const QJsonObject &object)
{
    Item item;
    item.id = text(object, "id");
    item.slug = text(object, "slug");
    item.title = text(object, "title");
    item.type = text(object, "type");
    item.blurPreview = text(object, "blur_preview");
    item.preview = parseRendition(object.value(QStringLiteral("preview")));
    item.shareGif = parseRendition(object.value(QStringLiteral("share_gif")));
    item.linkGif = parseRendition(object.value(QStringLiteral("link_gif")));
    return item;
}

Item parseItem(const QJsonObject &object)
{
    Item item;
    const QJsonValue id = object.value(QStringLiteral("id"));
    item.id = id.isString() ? id.toString() : QString::number(id.toInteger());
    item.slug = text(object, "slug");
    item.title = text(object, "title");
    item.type = text(object, "type");
    item.blurPreview = text(object, "blur_preview");
    const QJsonObject files = object.value(QStringLiteral("file")).toObject();
    item.preview = pick(files, {{"sm", "webp"}, {"sm", "gif"}, {"xs", "webp"}, {"xs", "gif"}, {"md", "webp"}, {"md", "gif"}});
    item.shareGif = pick(files, {{"md", "gif"}, {"hd", "gif"}, {"sm", "gif"}});
    item.linkGif = pick(files, {{"hd", "gif"}, {"md", "gif"}});
    return item;
}

bool parsePage(const QByteArray &json, Page *page, QString *error)
{
    QJsonValue data;
    if (!openEnvelope(json, &data, error)) {
        return false;
    }
    const QJsonObject object = data.toObject();
    if (!object.value(QStringLiteral("data")).isArray()) {
        *error = tr("KLIPY response has no items");
        return false;
    }
    page->items.clear();
    const QJsonArray items = object.value(QStringLiteral("data")).toArray();
    for (const auto &value : items) {
        Item item = parseItem(value.toObject());
        if (item.isValid()) {
            page->items.append(std::move(item));
        }
    }
    page->currentPage = object.value(QStringLiteral("current_page")).toInt();
    page->perPage = object.value(QStringLiteral("per_page")).toInt();
    page->hasNext = object.value(QStringLiteral("has_next")).toBool();
    return true;
}

bool parseCategories(const QByteArray &json, QList<Category> *categories, QString *error)
{
    QJsonValue data;
    if (!openEnvelope(json, &data, error)) {
        return false;
    }
    const QJsonValue list = data.toObject().value(QStringLiteral("categories"));
    if (!list.isArray()) {
        *error = tr("KLIPY response has no categories");
        return false;
    }
    categories->clear();
    for (const auto &value : list.toArray()) {
        const QJsonObject object = value.toObject();
        Category category {text(object, "category"), text(object, "query"), text(object, "preview_url")};
        if (!category.name.isEmpty() && !category.query.isEmpty()) {
            categories->append(category);
        }
    }
    return true;
}

bool parseStringList(const QByteArray &json, QStringList *strings, QString *error)
{
    QJsonValue data;
    if (!openEnvelope(json, &data, error)) {
        return false;
    }
    if (!data.isArray()) {
        *error = tr("KLIPY response has no suggestions");
        return false;
    }
    strings->clear();
    for (const auto &value : data.toArray()) {
        if (!value.toString().isEmpty()) {
            strings->append(value.toString());
        }
    }
    return true;
}

bool parseShare(const QByteArray &json, QString *error)
{
    QJsonValue data;
    return openEnvelope(json, &data, error);
}

QString errorMessage(const QByteArray &json)
{
    const QJsonObject root = QJsonDocument::fromJson(json).object();
    QStringList messages;
    collectMessages(root.value(QStringLiteral("errors")), &messages);
    collectMessages(root.value(QStringLiteral("message")), &messages);
    messages.removeAll(QString());
    return messages.join(QLatin1Char(' '));
}

}
