/*
 * MediaObject.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "MediaObject.h"

using namespace libdash::framework::input;

using namespace dash::mpd;
using namespace dash::network;
using namespace dash::metrics;

MediaObject::MediaObject    (ISegment *segment, IRepresentation *rep) :
             segment        (segment),
             rep            (rep)
{
    InitializeConditionVariable (&this->stateChanged);
    InitializeCriticalSection   (&this->stateLock);
}
MediaObject::~MediaObject   ()
{
    if(this->state == IN_PROGRESS)
    {
        this->segment->AbortDownload();
        this->OnDownloadStateChanged(ABORTED);
    }
    this->segment->DetachDownloadObserver(this);
    this->WaitFinished();

    DeleteConditionVariable (&this->stateChanged);
    DeleteCriticalSection   (&this->stateLock);
}

bool                MediaObject::StartDownload          ()
{
    this->segment->AttachDownloadObserver(this);
    return this->segment->StartDownload();
}
void                MediaObject::AbortDownload          ()
{
    this->segment->AbortDownload();
    this->OnDownloadStateChanged(ABORTED);
}
void                MediaObject::WaitFinished           ()
{
    EnterCriticalSection(&this->stateLock);

    while(this->state != COMPLETED && this->state != ABORTED)
        SleepConditionVariableCS(&this->stateChanged, &this->stateLock, INFINITE);

    LeaveCriticalSection(&this->stateLock);
}
int                 MediaObject::Read                   (uint8_t *data, size_t len)
{
    int read = this->segment->Read(data, len);
    for (int i = 0; i < read; i++) {
        segmentBytes.push_back(data[i]);
    }
    //DASH AUTHENTICATION
    if (read == 0) {
        SHA512 calculatedHash;
        byte calculatedHashBuffer[2 * SHA512::DIGESTSIZE];

        byte* segmentBytesArray = &segmentBytes[0];
        std::cout << segmentBytes.size() << std::endl;
        ArraySource source(segmentBytesArray, segmentBytes.size(), true,
                 new HashFilter(calculatedHash,
                 new HexEncoder(new ArraySink(calculatedHashBuffer, 2 * SHA512::DIGESTSIZE))));
        std::string calculatedHashString((const char*)calculatedHashBuffer, 2 * SHA512::DIGESTSIZE);

        if (calculatedHashString == this->GetHash()) {
            std::cout << "Valid hash for segment with hash value " << std::endl << this->GetHash() << std::endl;
        } else {
            std::cout << "Authentication Failure!" << std::endl;
            std::cout << "Invalid hash for hash value " << this->GetHash() << std::endl;
            std::cout << "Calculated has was " << calculatedHashString << std::endl;
            quick_exit(1);
        }
    }
    return read;
}
int                 MediaObject::Peek                   (uint8_t *data, size_t len)
{
    return this->segment->Peek(data, len);
}
int                 MediaObject::Peek                   (uint8_t *data, size_t len, size_t offset)
{
    return this->segment->Peek(data, len, offset);
}
IRepresentation*    MediaObject::GetRepresentation      ()
{
    return this->rep;
}
void                MediaObject::OnDownloadStateChanged (DownloadState state)
{
    EnterCriticalSection(&this->stateLock);

    this->state = state;

    WakeAllConditionVariable(&this->stateChanged);
    LeaveCriticalSection(&this->stateLock);
}
void                MediaObject::OnDownloadRateChanged  (uint64_t bytesDownloaded)
{
}
const std::vector<ITCPConnection *>&    MediaObject::GetTCPConnectionList   () const
{
    return this->segment->GetTCPConnectionList();
}
const std::vector<IHTTPTransaction *>&  MediaObject::GetHTTPTransactionList () const
{
    return this->segment->GetHTTPTransactionList();
}

//DASH AUTHENTICATION
const std::string                       MediaObject::GetHash() {
    return this->segment->GetHash();
}
