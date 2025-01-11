#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <ctime>
#include <curl/curl.h>
#include <C:\Users\gtavp\source\repos\septim\dependencies\nlohmann\json.hpp>
#include <iomanip>
#include <locale>
#include <codecvt>


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userData) {
    size_t totalSize = size * nmemb;
    userData->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

struct MessageData {
    std::string userID;
    std::string message;
    int categoryID;
    double amount;
    std::string messageID;
    int64_t unixTimestamp;

    // Method to initialize the struct from a JSON object
    static MessageData FromJSON(const nlohmann::json& json) {
        const auto& itemJson = json.at("Item"); // Access the "Item" field !!!!!!!!!!!!!!!!!!!

        MessageData data;
        data.userID = itemJson.at("UserID").get<std::string>();
        data.message = itemJson.at("Message").get<std::string>();
        data.categoryID = itemJson.at("CategoryID").get<int>();
        data.amount = itemJson.at("Amount").get<double>();
        data.messageID = itemJson.at("MessageID").get<std::string>();
        data.unixTimestamp = itemJson.at("Unix").get<int64_t>();
        return data;
    }

    // For debugging or displaying
    void Print() const {
        std::cout << "UserID: " << userID << "\n"
            << "Message: " << message << "\n"
            << "CategoryID: " << categoryID << "\n"
            << "Amount: " << amount << "\n"
            << "MessageID: " << messageID << "\n"
            << "Unix Timestamp: " << unixTimestamp << std::endl;
    }
};

std::optional<MessageData> GetMessageData(const std::string& messageID) {
    CURL* curl;
    CURLcode res;
    std::string responseBody;

    std::string url = "https://m4mq3nellj.execute-api.us-east-1.amazonaws.com/production/receive?message_id=" + messageID;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return std::nullopt;
        }
        std::cout << "Raw JSON Response: " << responseBody << std::endl;

        curl_easy_cleanup(curl);

        // Parse the JSON response
        try {
            auto jsonResponse = nlohmann::json::parse(responseBody);

            // Convert the JSON response into a MessageData object
            if (jsonResponse.is_object()) {
                return MessageData::FromJSON(jsonResponse);
            }
            else {
                std::cerr << "Invalid JSON structure: " << responseBody << std::endl;
                return std::nullopt;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
    else {
        std::cerr << "Failed to initialize curl" << std::endl;
        return std::nullopt;
    }
}


// Base64 decoding helper function
std::string Base64Decode(const std::string& encoded) {
    static const std::string base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    std::string decoded;
    std::vector<int> decodeTable(256, -1);

    for (size_t i = 0; i < base64Chars.size(); ++i) {
        decodeTable[base64Chars[i]] = static_cast<int>(i);
    }

    size_t padding = 0;
    size_t length = encoded.size();

    if (encoded.length() >= 2 && encoded[length - 1] == '=') {
        padding++;
        if (encoded[length - 2] == '=') {
            padding++;
        }
    }

    for (size_t i = 0; i < length; i += 4) {
        int n = (decodeTable[encoded[i]] << 18) +
            (decodeTable[encoded[i + 1]] << 12) +
            ((i + 2 < length ? decodeTable[encoded[i + 2]] : 0) << 6) +
            (i + 3 < length ? decodeTable[encoded[i + 3]] : 0);

        decoded += (n >> 16) & 0xFF;
        if (i + 2 < length && encoded[i + 2] != '=') {
            decoded += (n >> 8) & 0xFF;
        }
        if (i + 3 < length && encoded[i + 3] != '=') {
            decoded += n & 0xFF;
        }
    }

    return decoded;
}

// Decodes MessageID and returns userID, timestamp, and randomHex
std::optional<std::tuple<std::string, int, std::string>> DecodeMessageID(const std::string& messageID) {
    // Split the MessageID into parts based on underscores
    size_t firstUnderscore = messageID.find("_");
    size_t secondUnderscore = messageID.find("_", firstUnderscore + 1);

    if (firstUnderscore == std::string::npos || secondUnderscore == std::string::npos) {
        return std::nullopt; // Invalid format
    }

    try {
        std::string userIDEncoded = messageID.substr(0, firstUnderscore);
        std::string timestampHex = messageID.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);
        std::string randomHex = messageID.substr(secondUnderscore + 1);

        // Decode userID
        std::string userID = Base64Decode(userIDEncoded + "==");  // Ensure padding is added

        // Remove the last symbol from the userID
        if (!userID.empty()) {
            userID.pop_back();  // Remove the last character
        }

        // Convert timestamp from hex to int
        int timestamp = std::stoi(timestampHex, nullptr, 16);

        std::cout << "Decoded MessageID - UserID: " << userID << ", Timestamp: " << timestamp << ", Random: " << randomHex << std::endl;

        return std::make_tuple(userID, timestamp, randomHex);
    }
    catch (...) {
        return std::nullopt; // Error in decoding
    }
}


// Filters MessageIDs for a specific user
std::vector<std::string> FilterMessageIDsByUserID(const std::vector<std::string>& messageIDs, const std::string& targetUserID) {
    std::vector<std::string> filteredMessageIDs;

    for (const auto& messageID : messageIDs) {
        auto decoded = DecodeMessageID(messageID);
        if (decoded.has_value()) {
            const auto& [userID, timestamp, randomHex] = decoded.value();
            if (userID == targetUserID) {
                std::cout << "MessageID: " << messageID << " matches targetUserID." << std::endl;
                filteredMessageIDs.push_back(messageID);
            }
        }
        else {
            std::cerr << "Failed to decode MessageID: " << messageID << std::endl;
        }
    }

    return filteredMessageIDs;
}

// Fetches data from API and filters MessageIDs by userID
std::vector<std::string> getActualID(const std::string& targetUserID, time_t actualTimestamp) {
    CURL* curl;
    CURLcode res;
    std::string responseBody;
    std::vector<std::string> filteredMessageIDs;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://m4mq3nellj.execute-api.us-east-1.amazonaws.com/production/getid");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            try {
                auto jsonResponse = nlohmann::json::parse(responseBody);

                if (jsonResponse.contains("MessageIDs")) {
                    std::vector<std::string> messageIDs = jsonResponse["MessageIDs"].get<std::vector<std::string>>();

                    for (const auto& messageID : messageIDs) {
                        auto decoded = DecodeMessageID(messageID);
                        if (decoded.has_value()) {
                            const auto& [userID, timestamp, randomHex] = decoded.value();
                            if (userID == targetUserID && timestamp >= actualTimestamp) {
                                filteredMessageIDs.push_back(messageID);
                            }
                            else {
                                std::cout << "Skipped MessageID: " << messageID << std::endl;
                            }
                        }
                    }
                }
                else {
                    std::cerr << "JSON does not contain 'MessageIDs' key." << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing JSON or filtering MessageIDs: " << e.what() << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "Failed to initialize curl" << std::endl;
    }

    return filteredMessageIDs;
}
