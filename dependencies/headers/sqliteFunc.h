#include <iostream>
#include <string>
#include <sqlite3.h>
#include <ctime>
#include <optional>
using namespace std;


int callback(void* data, int argc, char** argv, char** colName) {
    for (int i = 0; i < argc; i++) {
        cout << colName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\n";
    }
    cout << "\n";
    return 0;
}

void addUser(sqlite3* db, string username, string hashed_password, string email) {
    sqlite3_stmt* stmt;
    string sql = "INSERT INTO Users (username, password, email) VALUES (?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_STATIC);

    // Execute the query
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
    }
    else {
        cout << "User inserted successfully\n";
    }

    sqlite3_finalize(stmt);
}



void addTransaction(sqlite3* db, const std::string& message_id, const std::string& user_id,
    double amount, unsigned int category_id, const std::string& message,
    time_t unix_time) {
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO Transactions (MessageID, UserID, Amount, CategoryID, Message, Unix) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        std::cerr << "SQL statement: " << sql << std::endl;
        return;
    }

    // Bind parameters according to the table structure
    sqlite3_bind_text(stmt, 1, message_id.c_str(), -1, SQLITE_STATIC);  // MessageID
    sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_STATIC);     // UserID
    sqlite3_bind_double(stmt, 3, amount);                               // Amount
    sqlite3_bind_int(stmt, 4, category_id);                             // CategoryID
    sqlite3_bind_text(stmt, 5, message.c_str(), -1, SQLITE_STATIC);     // Message
    sqlite3_bind_int64(stmt, 6, unix_time);                             // Unix

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    }
    else {
        std::cout << "Transaction added successfully\n";
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}

void deleteRow(sqlite3* db, const string& table, const string& column, const string& value) {
    sqlite3_stmt* stmt;
    string sql = "DELETE FROM " + table + " WHERE " + column + " = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, value.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
    }
    else {
        cout << "Row deleted successfully\n";
    }

    sqlite3_finalize(stmt);
}

void selectTable(sqlite3* db, string object, string place) {
    sqlite3_stmt* stmt;
    string sql = "SELECT " + object + " FROM '" + place + "';";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int numColumns = sqlite3_column_count(stmt);
        char** colNames = new char* [numColumns]; 
        char** rowValues = new char* [numColumns]; 

        for (int i = 0; i < numColumns; i++) {
            colNames[i] = (char*)sqlite3_column_name(stmt, i);  
            rowValues[i] = (char*)sqlite3_column_text(stmt, i); 
        }

        callback(nullptr, numColumns, rowValues, colNames);

        delete[] colNames;
        delete[] rowValues;
    }

    if (rc != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
    }

    sqlite3_finalize(stmt);
}


optional<string> getItem(sqlite3* db, const string& table, const string& column, const string& conditionColumn, const string& conditionValue) {
    sqlite3_stmt* stmt;
    string sql = "SELECT " + column + " FROM " + table + " WHERE " + conditionColumn + " = ?;";

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        return nullopt;
    }

    sqlite3_bind_text(stmt, 1, conditionValue.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        string result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        return result;
    }
    else {
        cerr << "No data found or execution failed: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        return nullopt;
    }
}