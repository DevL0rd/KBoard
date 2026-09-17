#include "modelcatalog.h"
#include "modeldownloader.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

namespace
{
void printModels(const ModelCatalog &catalog, const QString &directory)
{
    QTextStream out(stdout);
    for (const ModelEntry &entry : catalog.models()) {
        out << entry.id << '\t' << entry.name << ' ' << entry.variant << '\t'
            << entry.toVariantMap().value(QStringLiteral("sizeMb")).toInt() << " MiB";
        if (entry.id == catalog.defaultModelId()) {
            out << "\tdefault";
        }
        if (ModelCatalog::isDownloaded(entry, directory)) {
            out << "\tdownloaded";
        }
        out << Qt::endl;
    }
}

bool buildQueue(const ModelCatalog &catalog, const QStringList &ids, const QString &directory, QList<ModelEntry> *queue)
{
    QTextStream err(stderr);
    if (!ModelCatalog::isDownloaded(catalog.vad(), directory)) {
        queue->append(catalog.vad());
    }
    for (const QString &id : ids) {
        const ModelEntry *entry = catalog.find(id);
        if (!entry) {
            err << "Unknown model: " << id << Qt::endl;
            return false;
        }
        const bool queued = std::any_of(queue->cbegin(), queue->cend(), [entry](const ModelEntry &item) { return item.id == entry->id; });
        if (ModelCatalog::isDownloaded(*entry, directory)) {
            err << entry->id << " is already downloaded" << Qt::endl;
        } else if (!queued) {
            queue->append(*entry);
        }
    }
    return true;
}

int download(QCoreApplication &app, const QList<ModelEntry> &queue, const QString &directory)
{
    QTextStream err(stderr);
    ModelDownloader downloader;
    int lastPercent = -1;
    QObject::connect(&downloader, &ModelDownloader::progressChanged, &app, [&](double fraction) {
        const int percent = int(fraction * 100.0);
        if (fraction >= 0.0 && percent != lastPercent) {
            lastPercent = percent;
            err << "\rDownloading " << downloader.currentId() << ": " << percent << "%   " << Qt::flush;
        }
    });
    QObject::connect(&downloader, &ModelDownloader::entryFinished, &app,
        [&](const QString &id) { err << "\r" << id << " downloaded      " << Qt::endl; });
    QObject::connect(&downloader, &ModelDownloader::finished, &app, [&]() { app.exit(0); });
    QObject::connect(&downloader, &ModelDownloader::failed, &app, [&](const QString &, const QString &message) {
        err << Qt::endl << message << Qt::endl;
        app.exit(1);
    });
    QMetaObject::invokeMethod(&downloader, [&]() { downloader.fetch(queue, directory); }, Qt::QueuedConnection);
    return app.exec();
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("kboard-voice-model"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Download KBoard voice typing models"));
    parser.addHelpOption();
    const QCommandLineOption listOption({QStringLiteral("l"), QStringLiteral("list")}, QStringLiteral("List the available models"));
    const QCommandLineOption catalogOption(
        QStringLiteral("catalog"), QStringLiteral("Model catalogue file"), QStringLiteral("path"), ModelCatalog::defaultCatalogPath());
    const QCommandLineOption directoryOption(QStringLiteral("directory"), QStringLiteral("Folder to store models in"),
        QStringLiteral("path"), ModelCatalog::defaultModelsDirectory());
    parser.addOptions({listOption, catalogOption, directoryOption});
    parser.addPositionalArgument(
        QStringLiteral("model"), QStringLiteral("Model ids to download (default: the recommended model)"), QStringLiteral("[model...]"));
    parser.process(app);

    QString error;
    const ModelCatalog catalog = ModelCatalog::fromFile(parser.value(catalogOption), &error);
    if (!catalog.isValid()) {
        QTextStream(stderr) << error << Qt::endl;
        return 2;
    }
    const QString directory = parser.value(directoryOption);
    if (parser.isSet(listOption)) {
        printModels(catalog, directory);
        return 0;
    }
    const QStringList ids = parser.positionalArguments().isEmpty() ? QStringList {catalog.defaultModelId()} : parser.positionalArguments();
    QList<ModelEntry> queue;
    if (!buildQueue(catalog, ids, directory, &queue)) {
        return 2;
    }
    return queue.isEmpty() ? 0 : download(app, queue, directory);
}
