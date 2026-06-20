
#pragma once

#include <ostream>
#include "wal_record.h"

class WALSerializer {
public:
    static void write_record(std::ostream& out, const WALRecord& record);

private:
    static void write_put(std::ostream& out, const WALRecord& record);
    static void write_delete(std::ostream& out, const WALRecord& record);
};
