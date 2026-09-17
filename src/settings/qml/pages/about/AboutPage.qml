pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.devl0rd.kboard.settings
import "Credits.js" as Credits

SettingsPage {
    preview: AboutPreview {}

    Section {
        title: "KBoard"

        SettingRow {
            label: "Version"
            description: SystemInfo.shortDescription + " · Qt " + SystemInfo.qtVersion + " · KDE Frameworks " + SystemInfo.frameworksVersion
            iconName: "help-about"

            QQC2.Label {
                text: SystemInfo.version
                font.weight: Font.DemiBold
            }
        }

        SettingRow {
            label: "Links"
            iconName: "internet-services"

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                QQC2.Button {
                    text: "Source code"
                    icon.name: "vcs-normal"
                    onClicked: Qt.openUrlExternally(Credits.repository)
                }

                QQC2.Button {
                    text: "Report a bug"
                    icon.name: "tools-report-bug"
                    onClicked: Qt.openUrlExternally(SystemInfo.bugAddress)
                }
            }
        }
    }

    Section {
        title: "Credits"

        Repeater {
            model: SystemInfo.authors

            SettingRow {
                required property var modelData
                label: modelData.name
                description: [modelData.task, modelData.emailAddress].filter(Boolean).join(" · ")
                iconName: "user"
            }
        }
    }

    Section {
        title: "Licences"

        SettingRow {
            label: "KBoard"
            description: "Free software under the GNU General Public License, version 3"
            iconName: "license"

            QQC2.Button {
                text: "Read licence"
                icon.name: "document-open"
                onClicked: Qt.openUrlExternally(Credits.repository + "/blob/main/LICENSE")
            }
        }

        Repeater {
            model: Credits.notices

            FormNotice {
                required property var modelData
                notice: modelData
            }
        }
    }
}
