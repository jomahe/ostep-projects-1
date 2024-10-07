#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

vector<char*> paths;

inline void printErrorMessage(char* outputFile = nullptr) {
  int out = outputFile
    ? open(outputFile, O_CREAT | O_WRONLY, 0644)
    : STDERR_FILENO;

  out = out == -1 ? STDERR_FILENO : out;  // Error handling for open() failing

  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

void overwritePaths(vector<char*> &args) {
  args.erase(args.begin());
  paths = move(args);
}

char* inSearchPath(char* executable) {
  char fullPath[1024];
  for (char* &path : paths) {
    snprintf(fullPath, sizeof(fullPath), "%s/%s", path, executable);
    if (access(fullPath, X_OK) == 0) {
      return strdup(fullPath);
    }
  }
  return nullptr;
}

const vector<char*> parseInput(FILE*& file) {
  vector<char*> commands;
  if (!file) {
    cerr << "Invalid file!" << endl;
    exit(1);
  }

  char* line = nullptr;
  size_t lineLen = 0;

  while (getline(&line, &lineLen, file) != -1) {
    size_t len = strlen(line);
    if (line[len - 1] == '\n') line[len - 1] = '\0';
    commands.push_back(strdup(line));
  }

  free(line);
  return commands;
}

vector<char*> parseForRedirects(char* argument) {
  // Create an array to store the new args
  vector<char*> newArgs;
  char* before = nullptr;
  char* after = nullptr;

  char* redirPos = strchr(argument, '>');
  if (redirPos != nullptr) {
    *redirPos = '\0';
    before = argument;

    ++redirPos;
    after = redirPos;

    newArgs.push_back(before);
    newArgs.push_back(">\0");
    newArgs.push_back(after);
  } else {
    newArgs.push_back(argument);
  }

  return newArgs;
}

void executeCommand(char* &command) {
  /* Parse the command into its constituent arguments with strsep */
  vector<char*> args;
  char* currArg;
  char* commandCopy = strdup(command);

  bool redirected = false;
  size_t redirIndex;
  char* outputFile = nullptr;

  while ((currArg = strsep(&commandCopy, " ")) != NULL) {
    // Skip empty tokens due to multiple spaces
    // if (*currArg == '\0') continue;

    /* Parse for redirects. If none found, currArg is maintained. */
    vector<char*> redirectArgs = parseForRedirects(currArg);

    // for (char* &arg : redirectArgs) args.push_back(arg);
    size_t newArgs = redirectArgs.size();
    args.push_back(redirectArgs[0]);

    outputFile = (newArgs > 2) ? redirectArgs.back() : outputFile;
    redirected = (newArgs > 1);

    // Handle case where user redirects but doesn't specify a file or many '>'
    if (redirected && !outputFile ||
        newArgs == 3 && strchr(outputFile, '>') != nullptr) {
      printErrorMessage();
      return;
    }
  }
  args.push_back(nullptr);
  free(commandCopy);

  // for (size_t i = 0; i < args.size(); ++i) {
  //   if (args[i]) {
  //       cout << "args[" << i << "]: " << args[i] << endl;
  //   } else {
  //       cout << "args[" << i << "]: (null)" << endl;
  //   }
  // }

  /* Handle built-in commands with parent process */
  if (strcmp(args[0], "exit") == 0) {
    // Exit shouldn't be called with extra arguments
    if (args.size() == 2) {
      exit(0);
    }
    printErrorMessage();
  } else if (strcmp(args[0], "cd") == 0) {
    if (chdir(args[1]) == -1) printErrorMessage();
  } else if (strcmp(args[0], "path") == 0) {
    overwritePaths(args);
  } else {  // If not a built-in command, we fork the process and run on a child
    pid_t pid = fork();

    if (pid > 0) {  // Parent process waits for the child, processes exit status
      waitpid(pid, nullptr, 0);
    } else if (pid == 0) {  // Child process calls execv to execute command
      /* Check to see if the command is executable in any of the search paths */
      if (outputFile) {
        int out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        if (out == -1) {
          printErrorMessage();
          return;
        }
        dup2(out, STDOUT_FILENO);
        close(out);
      }
      char* path = inSearchPath(args[0]);
      if (!path) {
        printErrorMessage();
      } else {
        /* Execute the command in the CLI with execv */
        /* TODO: comment these two lines out after testing */
        int rc = execv(path, args.data());

        if (rc == -1) printErrorMessage();
      }
      exit(0);
    } else {
      printErrorMessage();
    }
  }
}

char* acceptInput() {
  string currLine;
  getline(cin, currLine);

  return strdup(currLine.c_str());
}

int main(int argc, char* argv[]) {
  paths.push_back("/bin");
  // We should be invoking the executable with either one or zero arguments
  if (argc > 2) {
    printErrorMessage();
    exit(1);

  } else if (argc == 2) {
    /* Read input from a batch file, execute the commands therein */
    FILE* in = nullptr;

    if (!(in = fopen(argv[1], "r"))) {
      printErrorMessage();
      exit(1);
    }

    vector<char*> commands = parseInput(in);
    for (char* command : commands) {
      executeCommand(command);
    }
    fclose(in);
  } else {
    /* Enter interactive mode; allow user to type in one command at a time */
    while (true) {
      cout << "wish> ";
      char* command = acceptInput();
      executeCommand(command);
      free(command);
    }
  }
  return 0;
}