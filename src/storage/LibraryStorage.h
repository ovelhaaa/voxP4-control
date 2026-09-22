#ifndef LIBRARY_STORAGE_H
#define LIBRARY_STORAGE_H

#include "model/LibraryModel.h"
#include <string>

class LibraryStorage {
public:
    // Initialize storage subsystem (mounts LittleFS on ESP32, no-op on host).
    static bool init();

    // Load active library from persistent storage.
    // If file does not exist or is invalid, falls back to the built-in factory library.
    static bool loadLibrary(Library& outLibrary, const char* path = nullptr);

    // Save library atomically (write temp -> validate -> rename).
    // Returns true on success, false if validation fails or write fails.
    static bool saveLibraryAtomic(const Library& library, const char* path = nullptr);

    // Creates the canonical factory default library.
    static Library createFactoryLibrary();
};

#endif // LIBRARY_STORAGE_H
