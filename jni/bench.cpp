#include <binder/ProcessState.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDebug.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MPEG4Writer.h>
#include <media/stagefright/OMXClient.h>
#include <media/stagefright/OMXCodec.h>
#include <time.h>
#include <signal.h>

using namespace android;

sp<MediaSource> createSource(const char *filename) {
    sp<MediaSource> source;

    sp<MediaExtractor> extractor =
        MediaExtractor::Create(new FileSource(filename));
    if (extractor == NULL) {
        return NULL;
    }

    size_t num_tracks = extractor->countTracks();

    sp<MetaData> meta;
    for (size_t i = 0; i < num_tracks; ++i) {
        meta = extractor->getTrackMetaData(i);
        CHECK(meta.get() != NULL);

        const char *mime;
        if (!meta->findCString(kKeyMIMEType, &mime)) {
            continue;
        }

        if (strncasecmp(mime, "video/", 6)) {
            continue;
        }

        source = extractor->getTrack(i);
        break;
    }

    return source;
}

clock_t start_time=0, end_time=0;
int frame_count=0;

OMXClient client;
sp<MediaSource> decoder;

void terminate(int)
{
    end_time = clock();
    status_t err = decoder->stop();

    client.disconnect();

    printf("fps: %lf\n", frame_count/((double)(end_time - start_time)/CLOCKS_PER_SEC));
    if (err != OK && err != ERROR_END_OF_STREAM) {
        fprintf(stderr, "failed: %d\n", err);
        exit(1);
    }

    exit(0);
}
 
int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }
    signal(SIGINT, terminate);
    android::ProcessState::self()->startThreadPool();

    DataSource::RegisterDefaultSniffers();

    CHECK_EQ(client.connect(), OK);

    status_t err = OK;

    sp<MediaSource> source = createSource(argv[1]);

    if (source == NULL) {
        fprintf(stderr, "Unable to find a suitable video track.\n");
        return 1;
    }

    sp<MetaData> meta = source->getFormat();

    decoder = OMXCodec::Create(
            client.interface(), meta, false /* createEncoder */, source, NULL, OMXCodec::kClientNeedsFramebuffer);

    int width, height;
    bool success = meta->findInt32(kKeyWidth, &width);
    success = success && meta->findInt32(kKeyHeight, &height);
    CHECK(success);
    
    CHECK_EQ(OK, decoder->start());

    MediaBuffer *buffer;
    start_time = clock();
    while (decoder->read(&buffer) == OK) {
        frame_count++;
        buffer->release();
        buffer = NULL;
    }
    end_time = clock();
    err = decoder->stop();

    client.disconnect();

    printf("fps: %lf\n", frame_count/((double)(end_time - start_time)/CLOCKS_PER_SEC));
    if (err != OK && err != ERROR_END_OF_STREAM) {
        fprintf(stderr, "failed: %d\n", err);
        return 1;
    }
    return 0;
}
