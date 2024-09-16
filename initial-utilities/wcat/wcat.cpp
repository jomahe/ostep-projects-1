#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {

  for (size_t i = 1; i < argc; ++i) {
    char* filename = argv[i];
    int fd = open(filename, O_RDONLY);

    if (fd < 0) { /* Bad file descriptor or bad open */
      cout << "wcat: cannot open file" << endl;
      return 1;
    }

    // Iteratively read from the file into a buffer, append the contents into a string
    string fileContents = "";
    size_t bufferSize = 1024;  // 1MB buffer

    char buffer[bufferSize];

    ssize_t bytesRead = read(fd, buffer, bufferSize - 1);
    do {
      buffer[bytesRead] = '\0';
      fileContents += buffer;
      bytesRead = read(fd, buffer, bufferSize - 1);
    } while (bytesRead > 0);

    cout << fileContents;
    close(fd);
  }
  return 0;
}
