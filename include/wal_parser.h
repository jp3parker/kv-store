
#pragma once

#include <iostream>
#include "status.h"
#include "wal_record.h"
#include "crc32.h"

class WALParser {
public:
    static RecoveryResult read_record(std::istream& in, WALRecord& record);

private:
    static constexpr size_t DEFAULT_MAX_KEY_SIZE = 1024;
    static constexpr size_t DEFAULT_MAX_VALUE_SIZE = 16 * 1024 * 1024;
};
