#include "file_callbacks.h"

namespace Callbacks {

    GodotFileRunner* GodotFileRunner::get_singleton() {
        static GodotFileRunner singleton;
        return &singleton;
    }

    void GodotFileRunner::queueReadRequest(FMOD_ASYNCREADINFO* request) {
        // High priority requests have to be processed first.
		// lock so we can't add and remove elements from the queue at the same time.
		std::lock_guard<std::mutex> lk(read_mut);
		requests.ordered_insert(FmodAsyncReadInfoHandle{request});
        read_cv.notify_one();
    }

    void GodotFileRunner::cancelReadRequest(FMOD_ASYNCREADINFO* request) {
        // lock so we can't add and remove elements from the queue at the same time.
        {
            std::lock_guard<std::mutex> lk(read_mut);
            requests.erase(FmodAsyncReadInfoHandle{request});
        }

        // We lock and check if the current request is the one being canceled.
        // In this case, we wait until it's done.
        {
            std::unique_lock<std::mutex> lk(cancel_mut);
            if (request == current_request) {
                cancel_cv.wait(lk);
            }
        }
    }

    void GodotFileRunner::run() {
        while (!stop) {
            // waiting for the container to have one request
            {
                std::unique_lock<std::mutex> lk(read_mut);
                read_cv.wait(lk, [this] { return !requests.empty() || stop; });
            }

            while (!requests.empty()) {
                // lock so we can't add and remove elements from the queue at the same time.
                // also store the current request so it cannot be cancel during process.
                {
                    std::lock_guard<std::mutex> lk(read_mut);
                    current_request = requests[0].info;
					requests.remove(0);
                }

                // We get the Godot File object from the handle
                FileAccess* file {reinterpret_cast<FileAccess*>(current_request->handle)};

                // update the position of the cursor
                file->seek(current_request->offset);

                // We read and store the requested data into the buffer.
            	size_t bytes_read = file->get_buffer(reinterpret_cast<uint8_t*>(current_request->buffer), current_request->sizebytes);
                current_request->bytesread = bytes_read;

                // Don't forget the return an error if end of the file is reached
                FMOD_RESULT result;
                if (file->eof_reached()) {
                    result = FMOD_RESULT::FMOD_ERR_FILE_EOF;
                } else {
                    result = FMOD_RESULT::FMOD_OK;
                }
                current_request->done(current_request, result);

                // Request no longer processed
                {
                    std::lock_guard<std::mutex> lk(cancel_mut);
                    current_request = nullptr;
                }
                cancel_cv.notify_one();
            }
        }
    }

    void GodotFileRunner::start() {
        stop = false;
        fileThread = std::thread(&GodotFileRunner::run, this);
    }

    void GodotFileRunner::finish() {
        stop = true;
        // we need to notify the loop one last time, otherwise it will stay stuck in the wait method.
        read_cv.notify_one();
        fileThread.join();
    }

    FMOD_RESULT F_CALLBACK godotFileOpen(
            const char* name,
            unsigned int* filesize,
            void** handle,
            void* userdata) {
		Error error;
        FileAccess* file = FileAccess::open(name, FileAccess::READ, &error);

        *filesize = file->get_len();
        *handle = reinterpret_cast<void*>(file);

        if (error == Error::OK) {
            return FMOD_RESULT::FMOD_OK;
        }
        return FMOD_RESULT::FMOD_ERR_FILE_BAD;
    }

    FMOD_RESULT F_CALLBACK godotFileClose(
            void* handle,
            void* userdata) {
        FileAccess* file {reinterpret_cast<FileAccess*>(handle)};
        file->close();
		memdelete(file);
        return FMOD_RESULT::FMOD_OK;
    }

	FMOD_RESULT F_CALLBACK godotFileRead(
			void *handle,
			void *buffer,
			unsigned int sizebytes,
			unsigned int *bytesread,
			void *userdata){
		

		// We get the Godot File object from the handle
		FileAccess* file {reinterpret_cast<FileAccess*>(handle)};



		// We read and store the requested data into the buffer.
		size_t bytes_read = file->get_buffer(reinterpret_cast<uint8_t*>(buffer), sizebytes);
		*bytesread = bytes_read;

		// Don't forget the return an error if end of the file is reached
		FMOD_RESULT result;
		if (file->eof_reached()) {
			result = FMOD_RESULT::FMOD_ERR_FILE_EOF;
		} else {
			result = FMOD_RESULT::FMOD_OK;
		}
		return result;
	}

	FMOD_RESULT F_CALLBACK godotFileSeek(
			void *handle,
			unsigned int pos,
			void *userdata){
		FileAccess* file {reinterpret_cast<FileAccess*>(handle)};
		// update the position of the cursor
		file->seek(pos);
		return FMOD_RESULT::FMOD_OK;
	}

    FMOD_RESULT F_CALLBACK godotSyncRead(
            FMOD_ASYNCREADINFO* info,
            void* userdata) {
        GodotFileRunner* runner {GodotFileRunner::get_singleton()};
        runner->queueReadRequest(info);
        return FMOD_RESULT::FMOD_OK;
    }

    FMOD_RESULT F_CALLBACK godotSyncCancel(
            FMOD_ASYNCREADINFO* info,
            void* userdata) {
        GodotFileRunner* runner {GodotFileRunner::get_singleton()};
        runner->cancelReadRequest(info);
        return FMOD_RESULT::FMOD_OK;
    }
}// namespace Callbacks