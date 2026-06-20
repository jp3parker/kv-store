
#pragma once

#include <cstdint>
#include <string>

struct WALRecord {
    enum class Operation : uint8_t {
        Put = 1,
        Delete = 2
    };

    Operation op;
    std::string key;
    std::string value;

    // these functions create a new WALRecord
    // object and return it
    static WALRecord Put(std::string key, std::string value);
    static WALRecord Delete(std::string key);
};
