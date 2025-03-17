#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iostream>

using namespace std;
string prevDir = "";


void changeDirectory(const string& path) {
    static string prevDir = getcwd(nullptr, 0);
    static string initalDir = getcwd(nullptr, 0);
    string currentDir = getcwd(nullptr, 0);
    if (path == "-") {
        if (prevDir.empty()) {
            cout << "cd: No previous directory set" << endl;
            return;
        }
        cout << prevDir << endl;
        chdir(prevDir.c_str());
    } else if (path == "~") {
        chdir(getenv("HOME"));
    } else if (path.empty()) {
        chdir(initalDir.c_str());
    } else {
        if (chdir(path.c_str()) != 0) {
          perror("cd failed");
          return;
        }
    }
    prevDir = currentDir; 
}

void listFiles(const string& path) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        perror("ls failed");
        return;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        cout << entry->d_name << "  ";
    }
    cout << endl;
    closedir(dir);
}

void changePermissions(const string& file, mode_t mode) {
    if (chmod(file.c_str(), mode) != 0) {
        perror("chmod failed");
    }
}

void makeDirectory(const string& dirName) {
    if (mkdir(dirName.c_str(), 0777) != 0) {
        perror("mkdir failed");
    }
}

void removeFile(const string& file) {
    if (remove(file.c_str()) != 0) {
        perror("rm failed");
    }
}

void moveFile(const string& source, const string& destination) {
    if (rename(source.c_str(), destination.c_str()) != 0) {
        perror("mv failed");
    }
}

void copyFile(const string& source, const string& destination) {
    string command = "cp -r " + source + " " + destination;
    if (system(command.c_str()) != 0) {
        perror("cp failed");
    }
}


void executeCommand(const vector<string>& args) {
    vector<char*> execArgs;
    for (const auto& arg : args) {
        execArgs.push_back(const_cast<char*>(arg.c_str()));
    }
    execArgs.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) {
        execvp(execArgs[0], execArgs.data());
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork failed");
    } else {
        waitpid(pid, nullptr, 0);
    }
}

int main() {
    string input;
    while (true) {
        cout << "newbash$ ";
        getline(cin, input);
        if (input.empty()) continue;

        vector<string> args;
        string arg;
        istringstream iss(input);
        while (iss >> arg) {
            args.push_back(arg);
        }
        
        if (args[0] == "exit") break;
        if (args[0] == "cd" && args.size() > 1) {
            changeDirectory(args[1]);
        } else if (args[0] == "ls") {
            listFiles(args.size() > 1 ? args[1] : ".");
        } else if (args[0] == "chmod" && args.size() > 2) {
            changePermissions(args[2], stoi(args[1], nullptr, 8));
        } else if (args[0] == "mkdir" && args.size() > 1) {
            makeDirectory(args[1]);
        } else if (args[0] == "rm" && args.size() > 1) {
            removeFile(args[1]);
        } else if (args[0] == "mv" && args.size() > 2) {
            moveFile(args[1], args[2]);
        } else if (args[0] == "cp" && args.size() > 2) {
            copyFile(args[1], args[2]);
        } else if (args[0] == "dnf" || args[0] == "apt" || args[0] == "pacman" || args[0] == "yum" || args[0] == "zypper") {
            executeCommand(args);
        } else if (args[0] == "nvim" || args[0] == "ping" || args[0] == "wget" || (args[0] == "ip" && args.size() > 1 && args[1] == "a") || args[0] == "df" || args[0] == "du" || args[0] == "lsblk" || args[0] == "mount") {
            executeCommand(args);
        } else {
            cout << "Command not found: " << args[0] << endl;
        }
    }
    return 0;
}
