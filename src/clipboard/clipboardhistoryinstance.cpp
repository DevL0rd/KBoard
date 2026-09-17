#include "clipboardhistory.h"
#include "clipboardimageprovider.h"
#include "clipboardroles.h"

#include "inputcontext.h"
#include "kboardpaths.h"

namespace
{
ClipboardHistory *s_instance = nullptr;
}

void ClipboardHistory::setInstance(ClipboardHistory *history)
{
    s_instance = history;
}

ClipboardHistory *ClipboardHistory::instance()
{
    if (!s_instance) {
        Options options;
        options.dataDirectory = KBoardPaths::userDataFile(QStringLiteral("clipboard"));
        options.cacheDirectory = KBoardPaths::cacheDirectory() + QStringLiteral("/clipboard");
        options.watchSystem = true;
        options.sensitiveProbe = [] { return InputContext::instance()->isSensitive(); };
        s_instance = new ClipboardHistory(options);
    }
    return s_instance;
}

ClipboardHistory *ClipboardHistory::create(QQmlEngine *engine, QJSEngine *)
{
    auto history = instance();
    QJSEngine::setObjectOwnership(history, QJSEngine::CppOwnership);
    if (engine && !engine->imageProvider(ClipboardRoles::imageProviderId())) {
        engine->addImageProvider(ClipboardRoles::imageProviderId(), new ClipboardImageProvider(history));
    }
    return history;
}
