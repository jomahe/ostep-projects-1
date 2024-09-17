#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

using namespace std;

inline bool contains(string& content, string& line) {
  return line.find(content) != string::npos;
}

void parseInputCLI(string& content, string& output) {
  string currLine;
  while (true) {
    getline(cin, currLine);

    if (currLine.empty()) break;
    if (contains(content, currLine)) output += currLine + '\n';
  }
}

void parseInputFile(FILE*& file, string& content, string& output) {
  char* line = nullptr;
  size_t lineLen = 0;

  while (getline(&line, &lineLen, file) != -1) {
    string lineStr = string(line);
    if (contains(content, lineStr)) output += line;
  }

  free(line);
  return;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    cout << "wgrep: searchterm [file ...]" << endl;
    return 1;
  }

  string content = string(argv[1]);
  string output = "";

  if (argc == 2) {
    parseInputCLI(content, output);
  } else {
    char* filename = argv[2];
    int fd = open(filename, O_RDONLY);

    if (fd < 0) { /* Bad file descriptor or bad open */
      cout << "wgrep: cannot open file" << endl;
      return 1;
    }

    FILE* file = fdopen(fd, "r");
    if (!file) {
      cerr << "Error converting the file descriptor to FILE*" << endl;
      close(fd);
      return 1;
    }

    parseInputFile(file, content, output);
    fclose(file);
  }

  cout << output;
  return 0;
}