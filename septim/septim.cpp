#define CURL_STATICLIB
#include "septim.h"
#include <curl/curl.h>
using namespace std;

int main() {
    sqlite3* db;
    int rc;
    // DB OPENING
    rc = sqlite3_open("septim.db", &db);

    if (rc) {
        std::cerr << "DB Error: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 1;
    }

    CURL* curl;

    curl = curl_easy_init();
    curl_easy_cleanup(curl);
    //unsigned int user_id = 1;

    //time_t unix = 1703984450;

    //double weekly = getReport(db, getStartOfWeek(unix), unix);
    //cout << "Weekly income between: " << unixToString(getStartOfWeek(unix))
    //    << " and " << unixToString(unix) << endl;
    //cout << "Weekly income: " << weekly << endl;


    // DB CLOSING
    sqlite3_close(db);
    return 0;
    return 0;
}