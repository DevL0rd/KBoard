#pragma once

#include "engineclient.h"
#include "transcriptwriter.h"

#include <QObject>
#include <QQmlEngine>
#include <QTimer>
#include <QVariantList>

class AudioCapture;
class VoiceModels;
class QMediaDevices;
class QThread;

class VoiceTyping : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)
    Q_PROPERTY(double level READ level NOTIFY levelChanged)
    Q_PROPERTY(QString partialText READ partialText NOTIFY partialTextChanged)
    Q_PROPERTY(QString committedText READ committedText NOTIFY committedTextChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QVariantList models READ models NOTIFY modelsChanged)
    Q_PROPERTY(QString modelId READ modelId NOTIFY modelChanged)
    Q_PROPERTY(QString modelName READ modelName NOTIFY modelChanged)
    Q_PROPERTY(QString modelVariant READ modelVariant NOTIFY modelChanged)
    Q_PROPERTY(int modelSizeMb READ modelSizeMb NOTIFY modelChanged)
    Q_PROPERTY(QStringList modelLanguages READ modelLanguages NOTIFY modelChanged)
    Q_PROPERTY(bool modelDownloaded READ isModelDownloaded NOTIFY modelsChanged)
    Q_PROPERTY(bool modelLoaded READ isModelLoaded NOTIFY backendChanged)
    Q_PROPERTY(double downloadProgress READ downloadProgress NOTIFY downloadProgressChanged)
    Q_PROPERTY(QString downloadingId READ downloadingId NOTIFY downloadProgressChanged)
    Q_PROPERTY(QString downloadError READ downloadError NOTIFY downloadErrorChanged)
    Q_PROPERTY(QString backend READ backend NOTIFY backendChanged)
    Q_PROPERTY(QString backendLabel READ backendLabel NOTIFY backendChanged)
    Q_PROPERTY(QString backendDescription READ backendDescription NOTIFY backendChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QVariantList microphones READ microphones NOTIFY microphonesChanged)
    Q_PROPERTY(QString microphone READ microphone NOTIFY microphoneChanged)
    Q_PROPERTY(double realTimeFactor READ realTimeFactor NOTIFY realTimeFactorChanged)

public:
    static VoiceTyping *instance();
    static VoiceTyping *create(QQmlEngine *, QJSEngine *);
    ~VoiceTyping() override;

    QString state() const;
    bool isPaused() const;
    double level() const;
    QString partialText() const;
    QString committedText() const;
    QString errorString() const;
    QVariantList models() const;
    QString modelId() const;
    QString modelName() const;
    QString modelVariant() const;
    int modelSizeMb() const;
    QStringList modelLanguages() const;
    bool isModelDownloaded() const;
    bool isModelLoaded() const;
    double downloadProgress() const;
    QString downloadingId() const;
    QString downloadError() const;
    QString backend() const;
    QString backendLabel() const;
    QString backendDescription() const;
    QVariantList devices();
    QVariantList microphones() const;
    QString microphone() const;
    double realTimeFactor() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void download(const QString &id);
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE bool deleteModel(const QString &id);
    Q_INVOKABLE void refreshDevices();
    Q_INVOKABLE void unloadModel();

Q_SIGNALS:
    void stateChanged();
    void pausedChanged();
    void levelChanged();
    void partialTextChanged();
    void committedTextChanged();
    void errorStringChanged();
    void modelsChanged();
    void modelChanged();
    void downloadProgressChanged();
    void downloadErrorChanged();
    void backendChanged();
    void devicesChanged();
    void microphonesChanged();
    void microphoneChanged();
    void realTimeFactorChanged();
    void transcriptCommitted(const QString &text);

private:
    explicit VoiceTyping(QObject *parent = nullptr);

    struct Session
    {
        quint64 id = 0;
        bool active = false;
        bool finishing = false;
        bool capturing = false;
        quint64 liveUtterance = 0;
        quint64 finalizedUtterance = 0;
        int pendingFinals = 0;
    };

    bool isCurrent(quint64 session) const;
    void connectCapture();
    void connectSpeech();
    void connectEngine();
    void connectModels();
    void connectSettings();
    bool canStart();
    void beginSession();
    void setState(const QString &state);
    void setError(const QString &error);
    void setPartialText(const QString &text);
    void setLevel(double level);
    void setPaused(bool paused);
    void updateActiveState();
    void onUtterance(quint64 utterance, const QList<float> &samples, bool endOfSpeech);
    void onDecoded(const DecodeRequest &request, const SpeechEngine::Result &result);
    void finishIfDone();
    void endSession();
    void closeSession();
    void failSession(const QString &error);
    void abortSession();
    void scheduleUnload();
    void onModelSettingsChanged();

    VoiceModels *m_models;
    EngineClient *m_engine;
    QThread *m_captureThread;
    AudioCapture *m_capture;
    QMediaDevices *m_mediaDevices;
    TranscriptWriter m_writer;
    QTimer m_unloadTimer;
    Session m_session;

    QString m_state = QStringLiteral("idle");
    QString m_errorString;
    QString m_partialText;
    QString m_microphone;
    double m_level = 0.0;
    double m_realTimeFactor = 0.0;
    bool m_paused = false;
};
