#include <gtest.h>
//#include "C:\Users\Администратор\source\repos\Module_32\gtest.h"
#include <gmock.h>
//#include "C:\Users\Администратор\source\repos\Module_32\gmock.h"
//#include <mysql.h>
#include "C:\Users\Администратор\source\repos\Module_32\include\mysql.h"
#include <string>
#include <iostream>

#define db_address "localhost"
#define db_login "frikodelka"
#define db_pw "qwerty"
#define db_name "chatdb"

class DBConnection
{
private:

public:
	MYSQL mysql;
	MYSQL_RES* res;
	MYSQL_ROW row;
	DBConnection() = default;

	virtual ~DBConnection() = default;
	DBConnection(DBConnection& _arr) = delete;
	DBConnection& operator=(DBConnection& _list) = delete;

	std::string msg_data;
	virtual bool open(const char* host, const char* login, const char* pw, const char* db, int port) {
		mysql_init(&mysql);
		if (&mysql == NULL) 
		{
			std::cout << "Error: can't create MySQL-descriptor" << std::endl;
			return false;
		}
		if (!mysql_real_connect(&mysql, host, login, pw, db, port, NULL, 0)) 
		{
			std::cout << "Error: can't connect to database " << mysql_error(&mysql) << std::endl;
			return false;
		}
		else 
		{
			std::cout << "Connection success!" << std::endl;
			mysql_set_character_set(&mysql, "utf8");
			return true;
		}
	}
	virtual void execQuery(const char* query) 
	{
		mysql_query(&mysql, query);
	}
	virtual void close() 
	{
		std::cout << "Connection closed!" << std::endl;
		mysql_close(&mysql);
	}
};

class MockSQL : public DBConnection {
public:
	MOCK_METHOD(bool open, (const char* host, const char* login, const char* pw, const char* db, int port), (override));
	MOCK_METHOD(void execQuery, (const char* query), (override));
	MOCK_METHOD(void close, (), (override));
};

class ClassThatUsesDB 
{
private:
public:
	bool openConnection(DBConnection* db, const char* host, const char* login, const char* pw, const char* db1, int port);
	void useConnection(DBConnection* db, const char* query);
	void closeConnection(DBConnection* db);
};

bool ClassThatUsesDB::openConnection(DBConnection* db, const char* host, const char* login, const char* pw, const char* db1, int port) 
{
	return db->open(host, login, pw, db1, port);
}

void ClassThatUsesDB::closeConnection(DBConnection* db) 
{
	db->close();
}

void ClassThatUsesDB::useConnection(DBConnection* db, const char* query) 
{
	db->execQuery(query);
	MYSQL_RES* result = mysql_store_result(&db->mysql);
	if (!result)
		throw std::string("MySQL not result!");
}

void SQL_anything_do(DBConnection* db_conn) 
{
	db_conn->open(db_address, db_login, db_pw, db_name, 0);
	db_conn->execQuery("SELECT * FROM users");
	if (db_conn->res = mysql_store_result(&db_conn->mysql)) 
	{
		while (db_conn->row = mysql_fetch_row(db_conn->res)) 
		{
			for (int rows_num = 0; rows_num < mysql_num_rows(db_conn->res); rows_num++) 
			{
				std::cout << db_conn->row[rows_num] << std::endl;
			}
		}
	}
	else
		std::cout << "Mysql error: " << mysql_error(&db_conn->mysql);
	db_conn->close();
}

class SomeTestSuite : public ::testing::Test 
{
protected:
	void SetUp() 
	{
		dbconn = new DBConnection();
	}

	void TearDown() 
	{
		delete dbconn;
	}

protected:
	DBConnection* dbconn;
};

TEST(SQLMockTest, test1) 
{
	MockSQL mock_db;
	EXPECT_CALL(mock_db, open).Times(::testing::AtLeast(1));
	EXPECT_CALL(mock_db, execQuery).Times(::testing::AtLeast(1));
	EXPECT_CALL(mock_db, close).Times(1);
	SQL_anything_do(&mock_db);
}

TEST_F(SomeTestSuite, test1) 
{
	DBConnection DB;
	ASSERT_TRUE(DB.open(db_address, db_login, db_pw, db_name, 0));
	DB.execQuery("CREATE TABLE test_table (test_column INT NOT NULL, test_column2 VARCHAR(45) NOT NULL)");
	DB.execQuery("INSERT INTO test_table (test_column, test_column2) VALUES('1234', 'test1')");
		DB.execQuery("SELECT test_column2 FROM test_table where test_column = 1234");
	if (DB.res = mysql_store_result(&DB.mysql)) {
		ASSERT_GE(mysql_num_rows(DB.res), 1);
		while (DB.row = mysql_fetch_row(DB.res)) {
			ASSERT_STREQ(DB.row[0], "test1");
		}
	}
	DB.close();
}

TEST_F(SomeTestSuite, test2)
{
	DBConnection DB;
	ASSERT_TRUE(DB.open(db_address, db_login, db_pw, db_name, 0));
	DB.execQuery("CREATE TABLE test_table (test_column INT NOT NULL, test_column2 VARCHAR(45) NOT NULL)");
	DB.execQuery("INSERT INTO test_table (test_column, test_column2) VALUES('5678', 'test2')");
	DB.execQuery("SELECT test_column2 FROM test_table where test_column = 5678");
	if (DB.res = mysql_store_result(&DB.mysql)) {
		ASSERT_GE(mysql_num_rows(DB.res), 2);
		while (DB.row = mysql_fetch_row(DB.res)) {
			ASSERT_STREQ(DB.row[0], "test2");
		}
	}
	DB.close();
}

int main(int argc, char* argv[]) 
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}