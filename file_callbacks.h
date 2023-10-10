#ifndef GODOTFMOD_FILE_CALLBACKS_H
#define GODOTFMOD_FILE_CALLBACKS_H

#include "core/os/file_access.h"
#include <condition_variable>
#include <thread>
#include "fmod_common.h"
#include "fmod_studio_common.h"

namespace Callbacks {
	struct FmodAsyncReadInfoHandle {
		FMOD_ASYNCREADINFO* info;

		bool operator<(const FmodAsyncReadInfoHandle& other) const
		{
			return info->priority > other.info->priority;
		}

		bool operator==(const FmodAsyncReadInfoHandle& other) const
		{
			return info == other.info;
		}
	};

    class GodotFileRunner {
    public:
        static GodotFileRunner* get_singleton();
        ~GodotFileRunner() = default;

    private:
        std::thread fileThread;

        std::condition_variable read_cv;
        std::mutex read_mut;

        std::condition_variable cancel_cv;
        std::mutex cancel_mut;

        bool stop = false;
        FMOD_ASYNCREADINFO* current_request = nullptr;
        Vector<FmodAsyncReadInfoHandle> requests = Vector<FmodAsyncReadInfoHandle>();

        GodotFileRunner() = default;
        GodotFileRunner(const GodotFileRunner&) = delete;
        GodotFileRunner& operator=(const GodotFileRunner&) = delete;

        void run();

    public:
        void queueReadRequest(FMOD_ASYNCREADINFO* request);
        void cancelReadRequest(FMOD_ASYNCREADINFO* request);
        void start();
        void finish();
    };

    FMOD_RESULT F_CALLBACK godotFileOpen(
            const char* name,
            unsigned int* filesize,
            void** handle,
            void* userdata);

    FMOD_RESULT F_CALLBACK godotFileClose(
            void* handle,
            void* userdata);

	FMOD_RESULT F_CALLBACK godotFileRead(
			void *handle,
			void *buffer,
			unsigned int sizebytes,
			unsigned int *bytesread,
			void *userdata);

	FMOD_RESULT F_CALLBACK godotFileSeek(
			void *handle,
			unsigned int pos,
			void *userdata);

    FMOD_RESULT F_CALLBACK godotSyncRead(
            FMOD_ASYNCREADINFO* info,
            void* userdata);

    FMOD_RESULT F_CALLBACK godotSyncCancel(
            FMOD_ASYNCREADINFO* info,
            void* userdata);
}// namespace Callbacks

#endif// GODOTFMOD_FILE_CALLBACKS_H
