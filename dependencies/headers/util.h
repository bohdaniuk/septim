#ifndef UTIL_H
#define UTIL_H 
#include <string>
#include <optional>
#include <iomanip>
#include <iostream>
#include <ctime>
#include <sstream>
#include <sqlite3.h>
using namespace std;

std::time_t getCurrentTime() {
	return std::time(nullptr);
}

//---------------------------------------------------------------------------------------------------
//------------------GETTING THE UNIX TIME OF THE STARTS----------------------------------------------
//---------------------------------------------------------------------------------------------------
time_t getStartOfDay(time_t timestamp) {
    struct tm timeInfo;
    localtime_s(&timeInfo, &timestamp);  // Use localtime_s for safety
    timeInfo.tm_hour = 0;               // Reset hours
    timeInfo.tm_min = 0;                // Reset minutes
    timeInfo.tm_sec = 0;                // Reset seconds
    return mktime(&timeInfo);           // Convert back to UNIX time
}

time_t getStartOfWeek(time_t timestamp) {
    struct tm timeInfo;
    localtime_s(&timeInfo, &timestamp);  // Convert to broken-down local time

    // Adjust to the start of the week (Sunday + 0) or Monday(+1)
    timeInfo.tm_mday -= (timeInfo.tm_wday + 6) % 7; // Subtract the number of days since Sunday
    timeInfo.tm_hour = 0;                 // Reset hours
    timeInfo.tm_min = 0;                  // Reset minutes
    timeInfo.tm_sec = 0;                  // Reset seconds

    return mktime(&timeInfo);             // Convert back to UNIX time
}

time_t getStartOfMonth(time_t timestamp) {
    struct tm timeInfo;
    localtime_s(&timeInfo, &timestamp); // Use localtime_s for safety
    timeInfo.tm_mday = 1;               // Reset to the first day of the month
    timeInfo.tm_hour = 0;               // Reset hours
    timeInfo.tm_min = 0;                // Reset minutes
    timeInfo.tm_sec = 0;                // Reset seconds
    return mktime(&timeInfo);           // Convert back to UNIX time
}
//---------------------------------------------------------------------------------------------------
//------------------DATA EXTRACTION FROM SQLite3 DB--------------------------------------------------
//---------------------------------------------------------------------------------------------------
std::string unixToString(time_t raw_time) {
    struct tm time_info;
    localtime_s(&time_info, &raw_time);

    char buffer[80];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", &time_info);
    return std::string(buffer);
}

std::optional<struct tm> optStringToStructTm(const std::optional<std::string>& dateTimeStr) {
    if (!dateTimeStr) {
        return std::nullopt; // Return empty if the optional doesn't contain a value
    }
    struct tm timeInfo = {};
    std::istringstream ss(*dateTimeStr);
    // Parse the string assuming the format "dd-mm-yyyy HH:MM:SS"
    ss >> std::get_time(&timeInfo, "%d-%m-%Y %H:%M:%S");
    if (ss.fail()) {
        std::cerr << "Failed to parse date-time string: " << *dateTimeStr << std::endl;
        return std::nullopt;
    }
    return timeInfo;
}
void showStructTm(struct tm result) {
        struct tm timeInfo = result;
        cout << (timeInfo.tm_year + 1900) << "-" << (timeInfo.tm_mon + 1) << "-" << timeInfo.tm_mday << " " << timeInfo.tm_hour << ":" << timeInfo.tm_min << ":" << timeInfo.tm_sec << endl;
}

time_t structTmToUnix(const std::optional<tm>& timeInfo) {
    if (!timeInfo) {
        return -1;
    }
    return mktime(const_cast<struct tm*>(&(*timeInfo)));
}

time_t extractDate(sqlite3* db, const std::string& table, const std::string& column, const std::string& conditionColumn, const std::string& conditionValue) {
    // Step 1: Retrieve the date string from the database
    auto dateString = getItem(db, table, column, conditionColumn, conditionValue);
    if (!dateString) {
        std::cerr << "Error: Unable to retrieve date from the database." << std::endl;
        return -1;
    }

    // Step 2: Convert the optional string to struct tm
    auto timeStruct = optStringToStructTm(dateString);
    if (!timeStruct) {
        std::cerr << "Error: Failed to parse date string to struct tm." << std::endl;
        return -1;
    }

    // Step 3: Convert struct tm to UNIX timestamp
    return structTmToUnix(*timeStruct);
}

#endif UTIL_H