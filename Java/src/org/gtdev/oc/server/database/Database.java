/**
 * @file Database.java
 * @auther Wang Daofei
 * Database class make connection to MySQL database using MySQL-Connector-Java driver.
 * The database name and connection information should be initialize first by static
 * method. The database should be created in utf8_general_ci collection.
 */
package org.gtdev.oc.server.database;

import java.sql.*;

public class Database {

    private static Connection dbconn;
    private static String prefix = "";

    /**
     * This method initialize the connection to database.
     * @param table_pref Set the prefix used in tables.
     * @param addr The MySQL connector address e.g."jdbc:mysql://localhost:3306/OC"
     * @param uname Database username.
     * @param pwd Database password.
     * @return 0, Success; -1, Unknown System error; -2, Connection error.
     */
    public static int Initialize(String table_pref, String addr, String uname, String pwd) {
        prefix = table_pref;
        try {
            Class.forName("com.mysql.jdbc.Driver");
            dbconn =  DriverManager.getConnection(addr, uname, pwd);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return -1;
    }

    /**
     * This method create the database.
     * @param dbName The requested database name.
     * @param addr The header of jdbc address e.g. "jdbc:mysql://localhost:3306/"
     * @param uname Database username.
     * @param pwd Database password.
     * @return 0, Success; -1, Unknown System error; -2, Connection error.
     */
    public static int createDatabase(String dbName, String addr, String uname, String pwd) {
        try {
            Class.forName("com.mysql.jdbc.Driver");
            dbconn =  DriverManager.getConnection(addr+dbName, uname, pwd);
        } catch (SQLException e) {
            e.printStackTrace();
            return -2;
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return -1;
        }

        return 0;
    }

    //! This method return the existing MySQL connection.
    public static Connection getConnection() {
        return dbconn;
    }

}
