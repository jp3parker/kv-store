
#include <iostream>
#include "./kv_store.h"

int main() {

  KVStore store("wal.log");
  
  std::ofstream wal("wal.log");
  wal << "PUT 3 5 abc";
  wal.flush();
  
  bool r=store.recover();
//  
  std::cout << "main.cpp -- " << r << std::endl;

  
  return 0;

}
