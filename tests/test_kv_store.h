
#include <gtest/gtest.h>
#include <random>
#include "kv_store.h"
#include <sstream>

class KVStoreTest : public ::testing::Test {
protected:
    std::string wal_file = "data/test.wal";
    std::mt19937 rng{12345};
    
    void SetUp() override {
        std::remove(wal_file.c_str());
        rng.seed(12345);
    }

    void TearDown() override {
        std::remove(wal_file.c_str());
    }
    
    int random_key_number() {
        static std::uniform_int_distribution<int> dist(0,49);
        return dist(rng);
    }

    bool random_put() {
        static std::bernoulli_distribution dist(0.5);
        return dist(rng);
    }

    int random_length() {
        static std::uniform_int_distribution<int> dist(0,49);
        return dist(rng);
    }

    char random_byte() {
        static std::uniform_int_distribution<int> dist(0,255);
        return static_cast<char>(dist(rng));
    }};
