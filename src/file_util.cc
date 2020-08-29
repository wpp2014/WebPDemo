#include "src/file_util.h"

#include <algorithm>
#include <memory>

FILE* OpenFile(const std::string& filename, const char* mode) {
  FILE* result = NULL;
  do {
    result = fopen(filename.c_str(), mode);
  } while (!result && errno == EINTR);
  return result;
}

bool CloseFile(FILE* file) {
  if (file == NULL) {
    return true;
  }
  return fclose(file) == 0;
}

bool ReadFileToString(const std::string& path, std::string* contents) {
  return ReadFileToStringWithMaxSize(
      path, contents, std::numeric_limits<size_t>::max());
}

bool ReadFileToStringWithMaxSize(const std::string& path,
                                 std::string* contents,
                                 size_t max_size) {
  if (contents) {
    contents->clear();
  }

  FILE* file = OpenFile(path, "rb");
  if (file == NULL) {
    return false;
  }

  const size_t kBufferSize = 1 << 16;
  std::unique_ptr<char[]> buf(new char[kBufferSize]);
  size_t len;
  size_t size = 0;
  bool read_status = true;

  // Many files supplied in |path| have incorrect size (proc files etc).
  // Hence, the file is read sequentially as opposed to a one-shot read.
  while ((len = fread(buf.get(), 1, kBufferSize, file)) > 0) {
    if (contents) {
      contents->append(buf.get(), std::min(len, max_size - size));
    }

    if ((max_size - size) < len) {
      read_status = false;
      break;
    }

    size += len;
  }
  read_status = read_status && !ferror(file);
  CloseFile(file);

  return read_status;
}
