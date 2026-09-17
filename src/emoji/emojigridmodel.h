#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>

class EmojiStore;

class EmojiGridModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(int kaomojiColumns READ kaomojiColumns WRITE setKaomojiColumns NOTIFY kaomojiColumnsChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QVariantList sections READ sections NOTIFY sectionsChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY sectionsChanged)
    Q_PROPERTY(bool stale READ isStale NOTIFY staleChanged)

public:
    enum Role
    {
        KindRole = Qt::UserRole + 1,
        TitleRole,
        SectionRole,
        CellsRole,
    };
    Q_ENUM(Role)

    explicit EmojiGridModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int columns() const;
    void setColumns(int columns);
    int kaomojiColumns() const;
    void setKaomojiColumns(int columns);
    QString searchText() const;
    void setSearchText(const QString &text);
    QVariantList sections() const;
    bool isStale() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE int rowForSection(const QString &id) const;
    Q_INVOKABLE QString sectionAt(int row) const;

Q_SIGNALS:
    void columnsChanged();
    void kaomojiColumnsChanged();
    void searchTextChanged();
    void sectionsChanged();
    void staleChanged();

private:
    enum class Kind
    {
        Header,
        Emoji,
        Kaomoji,
        Empty,
    };

    struct Row
    {
        Kind kind;
        QString section;
        QString title;
        QStringList cells;
        bool stored = false;
    };

    QVariantMap cellFor(const Row &row, const QString &cell) const;
    QVariantList cellsFor(const Row &row) const;
    void rebuild();
    void appendSearchResults(const QString &query);
    void appendRecents();
    void appendGroups();
    void appendKaomoji();
    void appendHeader(const QString &section, const QString &title);
    void appendCells(const QString &section, const QStringList &cells, Kind kind, bool stored = false);
    void markStale();
    void refreshCells();

    EmojiStore *m_store;
    QList<Row> m_rows;
    int m_columns = 8;
    int m_kaomojiColumns = 3;
    QString m_searchText;
    bool m_stale = false;
};
