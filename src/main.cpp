
#include <iostream>
#include "kv_store.h"

int main() {


    
    std::string wal_file = "data/tempwal.log";
    KVStore store(wal_file);

    store.put("a", "1");
    store.put("a", "2");
    store.put("a", "3");
    store.put("b", "x");


    
    
    
    store.compact();
    KVStore recovered(wal_file);
    
    dump_wal_bytes("data/tempwal.log");
    
    std::cout << "recovered.recover() = " << static_cast<int>(recovered.recover()) << std::endl;
    std::cout << "EXPECT_EQ(recovered.get(\"a\"), \"3\")" << recovered.get("a") << std::endl;
//    EXPECT_EQ(recovered.get("b"), "x");
//    EXPECT_EQ(recovered.get("b"), "x");

//    ASSERT_EQ(recovered.recover(), RecoveryResult::Success);
//
//    EXPECT_EQ(recovered.get("a"), "3");
//    EXPECT_EQ(recovered.get("b"), "x");


    return 0;

}


//TEST_F(KVStoreTest, CompactRemovesOldVersions)
//{
//    {
//        KVStore store(wal_file);
//
//        store.put("a", "1");
//        store.put("a", "2");
//        store.put("a", "3");
//        store.put("b", "x");
//
//        store.compact();
//    }
//
//    KVStore recovered(wal_file);
//
//    ASSERT_EQ(recovered.recover(), RecoveryResult::Success);
//
//    EXPECT_EQ(recovered.get("a"), "3");
//    EXPECT_EQ(recovered.get("b"), "x");
//}
