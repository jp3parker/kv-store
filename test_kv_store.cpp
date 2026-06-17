#include <gtest/gtest.h>
#include "kv_store.h"


class KVStoreTest : public ::testing::Test {
protected:
    std::string wal_file = "test.wal";

    void SetUp() override {
        std::remove(wal_file.c_str());
    }

    void TearDown() override {
        std::remove(wal_file.c_str());
    }
};


TEST_F(KVStoreTest, PutAndGet) {
    KVStore store(wal_file);
    store.put("name", "alice");
    EXPECT_EQ(store.get("name"), "alice");
}

TEST_F(KVStoreTest, OverwriteKey) {
    KVStore store(wal_file);
    store.put("name", "alice");
    store.put("name", "bob");
    EXPECT_EQ(store.get("name"), "bob");
}

TEST_F(KVStoreTest, DeleteKey) {
    KVStore store(wal_file);
    store.put("name", "alice");
    store.del("name");
    EXPECT_EQ(store.get("name"), "");
}

TEST_F(KVStoreTest, DeleteMissingKey) {
    KVStore store(wal_file);
    EXPECT_NO_THROW(store.del("missing"));
}

TEST_F(KVStoreTest, GetMissingKey) {
    KVStore store(wal_file);
    EXPECT_EQ(store.get("missing"), "");
}

TEST_F(KVStoreTest, RecoverPut) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
        store.put("b", "2");
    }

    KVStore recovered(wal_file);
    recovered.recover();

    EXPECT_EQ(recovered.get("a"), "1");
    EXPECT_EQ(recovered.get("b"), "2");
}

TEST_F(KVStoreTest, RecoverDelete) {
    {
        KVStore store(wal_file);
        store.put("a", "1");
        store.del("a");
    }

    KVStore recovered(wal_file);
    recovered.recover();

    EXPECT_EQ(recovered.get("a"), "");
}

TEST_F(KVStoreTest, RecoverOverwrite) {
    {
        KVStore store(wal_file);

        store.put("a", "1");
        store.put("a", "2");
        store.put("a", "3");
    }

    KVStore recovered(wal_file);
    recovered.recover();

    EXPECT_EQ(recovered.get("a"), "3");
}

TEST_F(KVStoreTest, CorruptedWalReturnsFailure) {
    KVStore store(wal_file);

    std::ofstream wal(wal_file);
    wal << "PUT 3 5 abc";
    wal.flush();

    EXPECT_FALSE(store.recover());
}

TEST_F(KVStoreTest, EmptyValue) {
    KVStore store(wal_file);

    store.put("key", "");

    EXPECT_EQ(store.get("key"), "");
}


