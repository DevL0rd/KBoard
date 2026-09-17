pragma ComponentBehavior: Bound

import QtQuick
import org.devl0rd.kboard.platform

QtObject {
    readonly property int purpose: InputContext.contentPurpose
    readonly property bool sensitive: InputContext.sensitive
    readonly property bool terminal: purpose === InputContext.ContentPurpose.content_purpose_terminal
    readonly property string variant: ({
        [InputContext.ContentPurpose.content_purpose_email]: "email",
        [InputContext.ContentPurpose.content_purpose_url]: "url"
    })[purpose] ?? ""
    readonly property string page: ({
        [InputContext.ContentPurpose.content_purpose_digits]: "numpad",
        [InputContext.ContentPurpose.content_purpose_number]: "numpad",
        [InputContext.ContentPurpose.content_purpose_phone]: "phone"
    })[purpose] ?? "letters"
    readonly property bool smart: !sensitive && !terminal && variant === "" && page === "letters"
}
