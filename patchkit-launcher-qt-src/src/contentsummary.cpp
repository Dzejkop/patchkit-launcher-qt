/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include "contentsummary.h"

#include "logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QStringList>
#include <QVariantList>

const QString   ContentSummary::encryptionMethodToken   = QString("encryption_method");
const QString   ContentSummary::compressionMethodToken  = QString("compression_method");
const QString   ContentSummary::hashingMethodToken      = QString("hashing_method");
const QString   ContentSummary::hashCodeToken           = QString("hash_code");
const QString   ContentSummary::filesToken              = QString("files");
const QString   ContentSummary::hashesToken             = QString("hashes");
const QString   ContentSummary::hashToken               = QString("hash");
const QString   ContentSummary::pathToken               = QString("path");
const QString   ContentSummary::chunksToken             = QString("chunks");
const QString   ContentSummary::sizeToken               = QString("size");

const FileData& FileData::dummy = FileData("dummy", 0);

ContentSummary::ContentSummary()
    : m_isValid(false)
{
}

ContentSummary::ContentSummary(int t_chunkSize, THash t_hashCode
                               , QString t_encryptionMethod
                               , QString t_compressionMethod
                               , QString t_hashingMethod, QVector<THash> t_chunkHashes
                               , QVector<FileData> t_filesSummary)
    : m_chunkSize(t_chunkSize)
    , m_hashCode(t_hashCode)
    , m_encryptionMethod(t_encryptionMethod)
    , m_compressionMethod(t_compressionMethod)
    , m_hashingMethod(t_hashingMethod)
    , m_chunkHashes(t_chunkHashes)
    , m_filesSummary(t_filesSummary)
{
}

ContentSummary::ContentSummary(const QJsonDocument& t_document)
    : m_isValid(false)
{
    if (t_document.isEmpty() || t_document.isNull())
    {
        return;
    }

    if (!t_document.isObject())
    {
        return;
    }

    QJsonObject doc_object = t_document.object();

    if (!doc_object.contains(encryptionMethodToken))
    {
        return;
    }

    m_encryptionMethod = doc_object[encryptionMethodToken].toString();

    if (!doc_object.contains(compressionMethodToken))
    {
        return;
    }

    m_compressionMethod = doc_object[compressionMethodToken].toString();

    if (!doc_object.contains(hashingMethodToken))
    {
        return;
    }

    m_hashingMethod = doc_object[hashingMethodToken].toString();

    if (!doc_object.contains(hashCodeToken))
    {
        return;
    }

    bool ok;
    m_hashCode = doc_object[hashCodeToken].toString().toUInt(&ok, 16);

    if (!ok)
    {
        return;
    }

    if (!parseFiles(doc_object))
    {
        return;
    }

    if (!parseChunks(doc_object))
    {
        return;
    }

    m_isValid = true;
}

const bool ContentSummary::isValid() const
{
    return m_isValid;
}

const int ContentSummary::getChunkSize() const
{
    return m_chunkSize;
}

const THash ContentSummary::getChunkHash(int t_index, bool& t_outOfBounds) const
{
    t_outOfBounds = t_index < 0 || t_index >= m_chunkHashes.size();

    if (t_outOfBounds)
    {
        return 0;
    }
    else
    {
        return m_chunkHashes.at(t_index);
    }
}

const THash ContentSummary::getChunkHash(int t_index) const
{
    bool check;
    return getChunkHash(t_index, check);
}

const FileData& ContentSummary::getFileData(int t_index, bool& t_outOfBounds) const
{
    t_outOfBounds = t_index < 0 || t_index >= m_filesSummary.size();

    if (t_outOfBounds)
    {
        return FileData::dummy;
    }
    else
    {
        return m_filesSummary.at(t_index);
    }
}

const FileData& ContentSummary::getFileData(int t_index) const
{
    bool check;
    return getFileData(t_index, check);
}

const QString& ContentSummary::getEncryptionMethod() const
{
    return m_encryptionMethod;
}

const QString& ContentSummary::getCompressionMethod() const
{
    return m_compressionMethod;
}

const QString& ContentSummary::getHashingMethod() const
{
    return m_hashingMethod;
}

const THash ContentSummary::getHashCode() const
{
    return m_hashCode;
}

const int ContentSummary::getChunksCount() const
{
    return m_chunkHashes.size();
}

const int ContentSummary::getFilesCount() const
{
    return m_filesSummary.size();
}

QJsonDocument ContentSummary::toJson() const
{
    QJsonObject root;

    root[encryptionMethodToken] = getEncryptionMethod();
    root[compressionMethodToken] = getCompressionMethod();
    root[hashingMethodToken] = getHashingMethod();
    root[hashCodeToken] = QString::number(getHashCode(), 16);

    QJsonArray filesArray;

    for (const FileData& fileData : m_filesSummary)
    {
        QJsonObject fileObject;
        fileObject[pathToken] = fileData.path;
        fileObject[hashToken] = QString::number(fileData.hash, 16);

        filesArray.append(fileObject);
    }

    QJsonObject chunksObject;
    QJsonArray chunkHashes;

    for (THash hash : m_chunkHashes)
    {
        chunkHashes.append(QString::number(hash, 16));
    }

    chunksObject[sizeToken] = getChunkSize();
    chunksObject[hashesToken] = chunkHashes;

    root[chunksToken] = chunksObject;
    root[filesToken] = filesArray;

    return QJsonDocument(root);
}

bool ContentSummary::parseFiles(QJsonObject& t_document)
{
    if (!t_document.contains(filesToken))
    {
        return false;
    }

    QJsonArray files = t_document[filesToken].toArray();

    bool ok;
    for (QJsonValueRef f : files)
    {
        if (!f.isObject())
        {
            return false;
        }

        if (!(f.toObject().contains(hashToken) && f.toObject().contains(pathToken)))
        {
            return false;
        }

        QString path = f.toObject()[pathToken].toString();
        THash hash = f.toObject()[hashToken].toString().toUInt(&ok, 16);

        if (!ok)
        {
            return false;
        }

        m_filesSummary.push_back(FileData(path, hash));

    }

    return true;
}

bool ContentSummary::parseChunks(QJsonObject& t_document)
{
    if (!t_document.contains(chunksToken))
    {
        return false;
    }

    QJsonObject chunks = t_document[chunksToken].toObject();

    if (chunks == QJsonObject())
    {
        return false;
    }

    int chunks_size = chunks[sizeToken].toInt(-1);

    if (chunks_size == -1)
    {
        return false;
    }

    if (!chunks.contains(hashesToken))
    {
        return false;
    }

    QJsonArray hashes = chunks[hashesToken].toArray();

    if (hashes == QJsonArray())
    {
        return false;
    }

    QVector<THash> hash_list;

    bool ok;
    for (QJsonValueRef item : hashes)
    {
        hash_list.push_back(item.toString().toUInt(&ok, 16));
        if (!ok)
        {
            return false;
        }
    }

    m_chunkSize = chunks_size;
    m_chunkHashes = hash_list;

    return true;
}
