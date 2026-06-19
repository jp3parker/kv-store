
#include <iostream>
#include "kv_store.h"

int main() {

    std::string wal_file;
    wal_file = "data/wal.log";

    
    KVStore store(wal_file);
    store.put("3", "4");
    store.del("3");
    store.put("l", "7");

    dump_wal_bytes("data/wal.log");
    


    return 0;

}
