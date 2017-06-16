#ifndef EXAMPLES_FILE_SYSTEM_STRING_STORE_H_
#define EXAMPLES_FILE_SYSTEM_STRING_STORE_H_

#include <string>

class StringStore {
 public:
  void Write(int off, const std::string& s);
  std::string Read(int start, int stop) const;

 private:
  std::string s_;
};

#endif  // EXAMPLES_FILE_SYSTEM_STRING_STORE_H_
