/**
 * Automatically generated by Zserio C++ extension version 1.2.0.
 */

#include "zserio/SqliteException.h"

#include "Database.h"


Database::Database(const std::string& fileName, const TRelocationMap& tableToDbFileNameRelocationMap)
{
    sqlite3 *internalConnection = NULL;
    const int sqliteOpenMode = SQLITE_OPEN_URI | SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
    const int result = sqlite3_open_v2(fileName.c_str(), &internalConnection, sqliteOpenMode, NULL);
    if (result != SQLITE_OK)
        throw zserio::SqliteException("Database::open(): can't open DB " + fileName, result);

    m_db.reset(internalConnection, zserio::SqliteConnection::INTERNAL_CONNECTION);

    TRelocationMap tableToAttachedDbNameRelocationMap;
    std::map<std::string, std::string> dbFileNameToAttachedDbNameMap;
    for (TRelocationMap::const_iterator relocationIt = tableToDbFileNameRelocationMap.begin();
            relocationIt != tableToDbFileNameRelocationMap.end(); ++relocationIt)
    {
        const std::string& tableName = relocationIt->first;
        const std::string& fileName = relocationIt->second;
        std::map<std::string, std::string>::const_iterator attachedDbIt =
                dbFileNameToAttachedDbNameMap.find(fileName);
        if(attachedDbIt == dbFileNameToAttachedDbNameMap.end())
        {
            const std::string attachedDbName = std::string(Database::databaseName) + "_" + tableName;
            attachDatabase(fileName, attachedDbName);
            attachedDbIt = dbFileNameToAttachedDbNameMap.insert(std::make_pair(fileName, attachedDbName)).first;
        }
        tableToAttachedDbNameRelocationMap.insert(std::make_pair(tableName, attachedDbIt->second));
    }

    initTables(tableToAttachedDbNameRelocationMap);
}

Database::Database(sqlite3* externalConnection, const TRelocationMap& tableToAttachedDbNameRelocationMap)
{
    m_db.reset(externalConnection, zserio::SqliteConnection::EXTERNAL_CONNECTION);
    initTables(tableToAttachedDbNameRelocationMap);
}

Database::~Database()
{
    detachDatabases();
}

sqlite3* Database::connection()
{
    return m_db.getConnection();
}

Table& Database::getTbl()
{
    return *m_tbl_;
}

void Database::createSchema()
{
    const bool wasTransactionStarted = m_db.startTransaction();

    m_tbl_->createTable();

    m_db.endTransaction(wasTransactionStarted);
}

void Database::createSchema(const std::set<std::string>&)
{
    createSchema();
}

void Database::deleteSchema()
{
    const bool wasTransactionStarted = m_db.startTransaction();

    m_tbl_->deleteTable();

    m_db.endTransaction(wasTransactionStarted);
}

/*constexpr const char* Database::databaseName()
{
    return "Database";
}*/

/*constexpr const char* Database::tableNameTbl()
{
    return "tbl";
}*/

/*void Database::fillTableNames(std::vector<std::string>& tableNames)
{
    tableNames.clear();
    tableNames.resize(1);
    tableNames[0] = tableNameTbl();
}*/

void Database::initTables(const TRelocationMap& tableToAttachedDbNameRelocationMap)
{
    static const char* EMPTY_STR = "";
    TRelocationMap::const_iterator relocationIt =
            tableToAttachedDbNameRelocationMap.find(tableNames[0]);
    m_tbl_.reset(new Table(
            this->m_db, tableNames[0],
            relocationIt != tableToAttachedDbNameRelocationMap.end() ? relocationIt->second : EMPTY_STR));
}

void Database::attachDatabase(const std::string& fileName, const std::string& attachedDbName)
{
    std::string sqlQuery = "ATTACH DATABASE '";
    sqlQuery += fileName;
    sqlQuery += "' AS ";
    sqlQuery += attachedDbName;

    m_db.executeUpdate(sqlQuery);

    m_attachedDbList.push_back(attachedDbName);
}

void Database::detachDatabases()
{
    for (std::vector<std::string>::const_iterator attachedDbIt = m_attachedDbList.begin();
            attachedDbIt != m_attachedDbList.end(); ++attachedDbIt)
    {
        const std::string sqlQuery = "DETACH DATABASE " + *attachedDbIt;
        m_db.executeUpdate(sqlQuery);
    }
    m_attachedDbList.clear();
}

constexpr const char* Database::databaseName;
constexpr std::array<const char*, 1> Database::tableNames;