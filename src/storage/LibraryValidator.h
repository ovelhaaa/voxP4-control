#ifndef LIBRARY_VALIDATOR_H
#define LIBRARY_VALIDATOR_H

#include "model/LibraryModel.h"
#include <string>
#include <vector>

struct ValidationResult {
    bool isValid() const { return errors.empty(); }
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

class LibraryValidator {
public:
    static constexpr uint32_t kSupportedFormatVersion = 1;

    static constexpr size_t kMaxPresets = 64;
    static constexpr size_t kMaxScenes = 128;
    static constexpr size_t kMaxSubscenesPerScene = 32;
    static constexpr size_t kMaxSetlists = 32;
    static constexpr size_t kMaxEntriesPerSetlist = 64;
    static constexpr size_t kMaxStringLength = 64;

    static ValidationResult validate(const Library& library);
};

#endif // LIBRARY_VALIDATOR_H
