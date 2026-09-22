#ifndef LIBRARY_SERIALIZER_H
#define LIBRARY_SERIALIZER_H

#include "model/LibraryModel.h"
#include <string>

class LibrarySerializer {
public:
    // Deserialize a JSON string into a Library domain model.
    // Completely parses the JSON into the compact Library model and frees DOM memory.
    // Returns true on successful parse and syntax validity; returns false with outError on failure.
    static bool deserializeJson(const char* jsonStr, size_t len, Library& outLibrary, std::string& outError);

    // Convenience overload for std::string
    static bool deserializeJson(const std::string& jsonStr, Library& outLibrary, std::string& outError) {
        return deserializeJson(jsonStr.data(), jsonStr.size(), outLibrary, outError);
    }

    // Serialize a Library domain model into a deterministic JSON string.
    static bool serializeJson(const Library& library, std::string& outJson, bool pretty = true);
};

#endif // LIBRARY_SERIALIZER_H
