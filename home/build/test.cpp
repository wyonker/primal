/* Standard C++ headers */
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>

/* MySQL Connector/C++ specific headers */
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/exception.h>
#include <cppconn/warning.h>

#define DBHOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "admin"
#define DATABASE "test"

#define NUMOFFSET 100
#define COLNAME 200

using namespace std;
using namespace sql;
sql::Driver *driver;
sql::Connection *con;
sql::Statement *stmt;
sql::ResultSet *res;

static void retrieve_data_and_print (ResultSet *rs, int type, int colidx, string colname) {

	/* retrieve the row count in the result set */
	cout << "\nRetrieved " << rs -> rowsCount() << " row(s)." << endl;

	cout << "\nCityName" << endl;
	cout << "--------" << endl;

	/* fetch the data : retrieve all the rows in the result set */
	while (rs->next()) {
		if (type == NUMOFFSET) {
                       cout << rs -> getString(colidx) << endl;
		} else if (type == COLNAME) {
                       cout << rs -> getString(colname) << endl;
		} // if-else
	} // while

	cout << endl;

} // retrieve_data_and_print()

static void retrieve_dbmetadata_and_print (Connection *dbcon) {

	if (dbcon -> isClosed()) {
		throw runtime_error("DatabaseMetaData FAILURE - database connection closed");
	}

	cout << "\nDatabase Metadata" << endl;
	cout << "-----------------" << endl;

	cout << boolalpha;

	/* The following commented statement won't work with Connector/C++ 1.0.5 and later */
	//auto_ptr < DatabaseMetaData > dbcon_meta (dbcon -> getMetaData());

	DatabaseMetaData *dbcon_meta = dbcon -> getMetaData();

	try {
		driver = get_driver_instance();

		/* create a database connection using the Driver */
		con = driver -> connect(url, user, password);

		/* alternate syntax using auto_ptr to create the db connection */
		//auto_ptr <Connection> con (driver -> connect(url, user, password));

		/* turn off the autocommit */
		con -> setAutoCommit(0);

		cout << "\nDatabase connection\'s autocommit mode = " << con -> getAutoCommit() << endl;

		/* select appropriate database schema */
		con -> setSchema(database);

		/* retrieve and display the database metadata */
		retrieve_dbmetadata_and_print (con);

		/* create a statement object */
		stmt = con -> createStatement();

		cout << "Executing the Query: \"SELECT * FROM City\" .." << endl;

		/* run a query which returns exactly one result set */
		res = stmt -> executeQuery ("SELECT * FROM City");

		cout << "Retrieving the result set .." << endl;

		/* retrieve the data from the result set and display on stdout */
		retrieve_data_and_print (res, NUMOFFSET, 1, string("CityName"));

		/* retrieve and display the result set metadata */
		retrieve_rsmetadata_and_print (res);

		cout << "Demonstrating Prepared Statements .. " << endl << endl;

		/* insert couple of rows of data into City table using Prepared Statements */
		prep_stmt = con -> prepareStatement ("INSERT INTO City (CityName) VALUES (?)");

		cout << "\tInserting \"London, UK\" into the table, City .." << endl;

		prep_stmt -> setString (1, "London, UK");
		updatecount = prep_stmt -> executeUpdate();

		cout << "\tCreating a save point \"SAVEPT1\" .." << endl;
		savept = con -> setSavepoint ("SAVEPT1");

		cout << "\tInserting \"Paris, France\" into the table, City .." << endl;

		prep_stmt -> setString (1, "Paris, France");
		updatecount = prep_stmt -> executeUpdate();

		cout << "\tRolling back until the last save point \"SAVEPT1\" .." << endl;
		con -> rollback (savept);
		con -> releaseSavepoint (savept);

		cout << "\tCommitting outstanding updates to the database .." << endl;
		con -> commit();

		cout << "\nQuerying the City table again .." << endl;

		/* re-use result set object */
		res = NULL;
		res = stmt -> executeQuery ("SELECT * FROM City");

		/* retrieve the data from the result set and display on stdout */
		retrieve_data_and_print (res, COLNAME, 1, string ("CityName"));

		cout << "Cleaning up the resources .." << endl;

		/* Clean up */
		delete res;
		delete stmt;
		delete prep_stmt;
		con -> close();
		delete con;

	} catch (SQLException &e) {
		cout << "ERROR: SQLException in " << __FILE__;
		cout << " (" << __func__<< ") on line " << __LINE__ << endl;
		cout << "ERROR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << ")" << endl;

		if (e.getErrorCode() == 1047) {
			/*
 * 			Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
 * 						Message: Unknown command
 * 									*/
			cout << "\nYour server does not seem to support Prepared Statements at all. ";
			cout << "Perhaps MYSQL < 4.1?" << endl;
		}

		return EXIT_FAILURE;
	} catch (std::runtime_error &e) {

		cout << "ERROR: runtime_error in " << __FILE__;
		cout << " (" << __func__ << ") on line " << __LINE__ << endl;
		cout << "ERROR: " << e.what() << endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
} // main()

