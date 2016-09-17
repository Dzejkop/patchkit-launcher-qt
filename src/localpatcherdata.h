/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#pragma once

#include <QObject>
#include <QString>

#include "data.h"
#include "quazipfile.h"

class LocalPatcherData : public QObject
{
    Q_OBJECT

public:
    bool isInstalled();

    bool isInstalledSpecific(int t_version, const Data& t_data);

    void install(const QString& t_downloadedPath, const Data& t_data, int t_version);

    void start(const Data& t_data);

private:
    void uninstall();

    static void writeFileContents(const QString& t_filePath, const QString& t_fileContents);

    static QString readFileContents(const QString& t_filePath);

    static bool checkIfFilesExist(const QStringList& t_filesList);

    static void createDirIfNotExists(const QString& t_dirPath);

    static void extractZip(const QString& t_zipFilePath,
                           const QString& t_extractDirPath,
                           QStringList& t_extractedEntriesList);

    static void extractDirZipEntry(const QString& t_zipEntryPath);

    static void extractFileZipEntry(QuaZipFile& t_zipEntry, const QString& t_zipEntryPath);

    static bool isDirZipEntry(const QString& t_zipEntryName);

    static void copyDeviceData(QIODevice& t_readDevice, QIODevice& t_writeDevice);

    int readVersion();

    static QString getPatcherId(const Data& t_data);

    static int parseVersionInfoToNumber(const QString& t_versionInfoFileContents);

    void readPatcherManifset(QString& t_exeFileName,
                             QString& t_exeArguments) const;

    QString formatPatcherManifestString(const QString& t_stringToFormat,
                                        const QByteArray& t_encodedApplicationSecret) const;
};