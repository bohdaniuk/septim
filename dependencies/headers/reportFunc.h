#include <util.h>
#include <sqlite3.h>

double getReport(sqlite3* db, time_t boundary_first, time_t boundary_last) {
    double totalMoney = 0.0;
    sqlite3_stmt* stmt;
    std::string sql = "SELECT amount FROM Transactions WHERE date_unix >= ? AND date_unix <= ?;";

    // Prepare the SQL statement
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return 0.0;
    }

    // Bind the parameters
    sqlite3_bind_int64(stmt, 1, boundary_first);
    sqlite3_bind_int64(stmt, 2, boundary_last);

    // Execute the query and iterate over the result set
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        double amount = sqlite3_column_double(stmt, 0);  // Fetch the 'amount' value
        totalMoney += amount;  // Accumulate the total money
    }

    // Check for any errors during execution
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    }

    // Finalize the statement to release resources
    sqlite3_finalize(stmt);

    return totalMoney;
}
