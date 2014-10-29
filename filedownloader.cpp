#include "filedownloader.h"

FileDownloader::FileDownloader(QUrl imageUrl, QObject *parent) :
    QObject(parent)
{
    connect(&m_WebCtrl, SIGNAL(finished(QNetworkReply*)),
                SLOT(fileDownloaded(QNetworkReply*)));

    QNetworkRequest request(imageUrl);
    m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader()
{

}

void FileDownloader::SL_DownloadedContract()
{
    QString qstrContents(downloadedData());
    std::cout << "\nSLOT Downloaded Data: \n";
    std::cout << qstrContents.toStdString();
    std::cout << "\n============DATA========\n";
    // ----------------------------
       int32_t nAdded;
     // OT Server relationship with Sopro Client
       if (!qstrContents.isEmpty())
       nAdded = OTAPI_Wrap::It()->AddServerContract(qstrContents.toStdString());
        else
        {
            std::cout << "Failed Importing Server Contract. Failed trying to import contract. Is it already in the wallet?";
            return;
        }

}


void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{
    m_DownloadedData = pReply->readAll();
    //emit a signal
    pReply->deleteLater();
    SL_DownloadedContract();
    emit downloaded();
}

QByteArray FileDownloader::downloadedData() const
{
    return m_DownloadedData;
}


