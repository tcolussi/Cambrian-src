#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <core/handlers/contacthandler.hpp>
#include <opentxs/OTAPI.hpp>
#include <opentxs/OTAPI_Exec.hpp>
#include <opentxs/OTAsymmetricKey.hpp>
#include <opentxs/OTRecordList.hpp>
#include <opentxs/OTCaller.hpp>

class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QUrl imageUrl, QObject *parent = 0);

    virtual ~FileDownloader();

    QByteArray downloadedData() const;


signals:
        void downloaded();

private slots:

    void fileDownloaded(QNetworkReply* pReply);


private:
    void SL_DownloadedContract();
    QNetworkAccessManager m_WebCtrl;

    QByteArray m_DownloadedData;

};

#endif // FILEDOWNLOADER_H
