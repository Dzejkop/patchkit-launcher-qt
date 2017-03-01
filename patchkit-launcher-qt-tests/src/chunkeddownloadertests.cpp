/*
* Copyright (C) Upsoft 2016
* License: https://github.com/patchkit-net/patchkit-launcher-qt/blob/master/LICENSE
*/

#include "catch.h"

#include <QtNetwork>
#include <QApplication>

#include "src/chunkeddownloader.h"
#include "src/timeoutexception.h"
#include "src/contentsummary.h"
#include "src/staledownloadexception.h"

#include "mockednam.h"
#include "custommacros.h"

SCENARIO("Testing how status codes affect the chunked downloader", "[chunked_downloader]")
{
    std::shared_ptr<CancellationTokenSource> tokenSource(new CancellationTokenSource());
    CancellationToken token(tokenSource);

    GIVEN("A mocked NAM which replies with various error status codes.")
    {
        QByteArray data = "ABCDEFGHIJ";

        ContentSummary summary(1, 0, "none", "none", "xxHash",
        {
            HashingStrategy::xxHash(data.mid(0, 1)),
            HashingStrategy::xxHash(data.mid(1, 1)),
            HashingStrategy::xxHash(data.mid(2, 1)),
            HashingStrategy::xxHash(data.mid(3, 1)),
            HashingStrategy::xxHash(data.mid(4, 1)),
            HashingStrategy::xxHash(data.mid(5, 1)),
            HashingStrategy::xxHash(data.mid(6, 1)),
            HashingStrategy::xxHash(data.mid(7, 1)),
            HashingStrategy::xxHash(data.mid(8, 1)),
            HashingStrategy::xxHash(data.mid(9, 1))
        },
        {});

        int namReplyDelay = 200;
        QStringList urls =
        {
            "link1",
            "link3",
            "link4",
            "link5"
        };

        MockedNAM nam;
        nam.push(urls[0], data, namReplyDelay, 100);
        nam.push(urls[1], data, namReplyDelay, 300);
        nam.push(urls[2], data, namReplyDelay, 400);
        nam.push(urls[3], data, namReplyDelay, 500);

        ChunkedDownloader downloader(&nam, summary, &HashingStrategy::xxHash, token);

        int permittedTimeout = 1000;

        THEN("An exception should occur and the download should fail for every one of them.")
        {
            for (QString& url : urls)
            {
                EXPECT_ANY(downloader.downloadFile(url, permittedTimeout));
            }
        }
    }
}

SCENARIO("Testing chunked downloader in multiple scenarios.", "[chunked_downloader]")
{
    std::shared_ptr<CancellationTokenSource> tokenSource(new CancellationTokenSource());
    CancellationToken token(tokenSource);

    GIVEN("A constant data set in chunks with a content summary.")
    {
        const QByteArray data = "ABCDEFGHIJ";

        ContentSummary summary(1, 0, "none", "none", "xxHash",
        {
            HashingStrategy::xxHash(data.mid(0, 1)),
            HashingStrategy::xxHash(data.mid(1, 1)),
            HashingStrategy::xxHash(data.mid(2, 1)),
            HashingStrategy::xxHash(data.mid(3, 1)),
            HashingStrategy::xxHash(data.mid(4, 1)),
            HashingStrategy::xxHash(data.mid(5, 1)),
            HashingStrategy::xxHash(data.mid(6, 1)),
            HashingStrategy::xxHash(data.mid(7, 1)),
            HashingStrategy::xxHash(data.mid(8, 1)),
            HashingStrategy::xxHash(data.mid(9, 1))
        },
        {});

        GIVEN("A mocked NAM wich replies to 'link' with the specified data in 300 ms.")
        {
            MockedNAM nam;

            nam.push("link", data, 300);

            ChunkedDownloader downloader(&nam, summary, &HashingStrategy::xxHash, token);

            THEN ("With a 1000 ms permitted timeout, the chunked download should succeed.")
            {
                QByteArray downloadedData = downloader.downloadFile("link", 1000);

                REQUIRE(downloadedData.toStdString() == data.toStdString());
            }

            THEN ("With a 200 ms permitted timeout, an exception should occur.")
            {
                EXPECT(downloader.downloadFile("link", 200), TimeoutException&);
            }
        }

        GIVEN("A mocked NAM which always corrupts the first reply.")
        {
            MockedNAM nam;

            nam.push("link", data, 300);
            nam.corrupt();

            THEN ("With a 1000 ms permitted timeout and 1000 ms permitted stale download timeout, the chunked download should succeed.")
            {
                ChunkedDownloader downloader(&nam, summary, &HashingStrategy::xxHash, token);

                QByteArray downloadedData = downloader.downloadFile("link", 1000);

                REQUIRE(downloadedData.toStdString() == data.toStdString());
            }
        }
    }
}

SCENARIO("Simulating RemotePatcherData's alternate url's functionality.", "[chunked_downloader]")
{
    std::shared_ptr<CancellationTokenSource> tokenSource(new CancellationTokenSource());
    CancellationToken token(tokenSource);

    GIVEN("A configuration permitting 1000ms stale timeout, 100ms minimal timeout, 300ms max timeout.")
    {
        const QByteArray data =         "ABCDEFGHIJ";

        int minTimeoutMsec = 100;
        int maxTimeoutMsec = 300;

        ContentSummary summary(1, 0, "none", "none", "xxHash",
        {
            HashingStrategy::xxHash(data.mid(0, 1)),
            HashingStrategy::xxHash(data.mid(1, 1)),
            HashingStrategy::xxHash(data.mid(2, 1)),
            HashingStrategy::xxHash(data.mid(3, 1)),
            HashingStrategy::xxHash(data.mid(4, 1)),
            HashingStrategy::xxHash(data.mid(5, 1)),
            HashingStrategy::xxHash(data.mid(6, 1)),
            HashingStrategy::xxHash(data.mid(7, 1)),
            HashingStrategy::xxHash(data.mid(8, 1)),
            HashingStrategy::xxHash(data.mid(9, 1))
        },
        {});

        GIVEN("A mocked NAM with 2 possible responses, both valid, the first responds in 400ms, the second in 200ms.")
        {
            MockedNAM nam;
            nam.push("link1", data, 400);
            nam.push("link2", data, 200);

            ChunkedDownloader downloader(&nam, summary, HashingStrategy::xxHash, token);

            int timeoutCount = 0;

            WHEN("Downloading from the first link, 2 timeouts should occur.")
            {
                try
                {
                    try
                    {
                        downloader.downloadFile("link1", minTimeoutMsec);
                    }
                    catch (TimeoutException&)
                    {
                        timeoutCount++;
                        downloader.downloadFile("link1", maxTimeoutMsec);
                    }
                }
                catch (TimeoutException&)
                {
                    timeoutCount++;
                }

                REQUIRE(timeoutCount == 2);
            }

            timeoutCount = 0;

            WHEN("Downloading from the second link, 1 timeout should occur and the download should succed the second time.")
            {
                QByteArray downloadedData;

                try
                {
                    try
                    {
                        downloader.downloadFile("link2", minTimeoutMsec);
                    }
                    catch (TimeoutException&)
                    {
                        timeoutCount++;
                        downloadedData = downloader.downloadFile("link2", maxTimeoutMsec);
                    }
                }
                catch (TimeoutException&)
                {
                    timeoutCount++;
                }

                CHECK(timeoutCount == 1);
                REQUIRE(downloadedData.toStdString() == data.toStdString());
            }
        }
    }
}
