
#include <iostream>
#include "./kv_store.h"

int main() {

    std::string wal_file;
    wal_file = "wal.log";

    
    KVStore store(wal_file);
    store.put("3", "4");
store.del("3");

dump_wal_bytes("wal.log");
    


  return 0;

}
