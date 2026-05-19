#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <ctime>

enum LogLevel {
    INFO,
    WARNING,
    ERR
};

class Logger {
public:
    Logger(const std::string& filename);

    void log(LogLevel level, const std::string& message);
private:
    std::ofstream logFile;

    std::string getCurrentTimestamp();
};

#endif // LOGGER_H
