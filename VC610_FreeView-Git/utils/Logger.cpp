#include "Logger.h"

Logger::Logger(const std::string& filename) : logFile(filename, std::ios_base::app) {
    if (!logFile.is_open()) {
        std::cerr << "Error: Unable to open log file." << std::endl;
    }
    logFile << std::endl;
    logFile << "==========================" << std::endl;
    logFile << std::endl;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (logFile.is_open()) {
        logFile << getCurrentTimestamp().c_str() << " ";
        switch (level) {
            case LogLevel::INFO:
                logFile << "[INFO] ";
                break;
            case LogLevel::WARNING:
                logFile << "[WARNING] ";
                break;
            case LogLevel::ERR:
                logFile << "[ERROR] ";
                break;
        }
        logFile << message.c_str() << std::endl;
    }
}

std::string Logger::getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buffer;
}
