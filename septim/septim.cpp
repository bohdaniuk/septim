#define CURL_STATICLIB
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include "httpFunc.h"
#include <sqliteFunc.h>

int main() {
    // ----------------------------------------------------------------------------------
    // Setting the console to UTF-8
    
    std::locale::global(std::locale("uk_UA.UTF-8"));

    // ----------------------------------------------------------------------------------
    sqlite3* db;
    int rc;

    // DB OPENING
    rc = sqlite3_open("septim.db", &db);
    if (rc) {
        std::cerr << "DB Error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }
    // ----------------------------------------------------------------------------------
    std::string encoded = "RHJheWJpbg_67894914_013e";
    std::cout << "Decoded: " << Base64Decode(encoded) << std::endl;


    std::string targetUserID = "Draybin";
    time_t actualTimestamp = 173000000;
    auto filteredMessageIDs = getActualID(targetUserID, actualTimestamp);

    // Output filtered MessageIDs
    std::cout << "Filtered MessageIDs for user '" << targetUserID << "':\n";
    for (const auto& messageID : filteredMessageIDs) {
        std::cout << messageID << std::endl;
    }

    // Iterate over each filtered message ID
    for (const auto& messageID : filteredMessageIDs) {
        // Fetch and parse the data
        auto data = GetMessageData(messageID);

        if (data.has_value()) {
            const auto& messageData = data.value();

            addTransaction(db, messageData.messageID, messageData.userID, messageData.amount, 
                messageData.categoryID, messageData.message, messageData.unixTimestamp);
        }
        else {
            std::cerr << "Failed to retrieve data for MessageID: " << messageID << std::endl;
        }
    }

    // ----------------------------------------------------------------------------------

    // DB CLOSING
    sqlite3_close(db);
    return 0;
}
