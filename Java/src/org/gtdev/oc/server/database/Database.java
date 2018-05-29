/**
 * @file Database.java
 * @auther Wang Daofei
 * Database class make connection to MySQL database using MySQL-Connector-Java driver.
 * The database name and connection information should be initialize first by static
 * method. The database should be created in utf8_general_ci collection.
 */
package org.gtdev.oc.server.database;

import java.sql.*;

public abstract class Database {
	enum DBtype {
		MYSQL,
		SQLITE
	}

    private Connection dbconn;
    private Statement st = null;
    protected DBtype t;
    
    //protected String prefix = "";
    
    public void releaseStatement() throws SQLException {
    	if(st != null)
    		st.close();
    }
    
    Database(){
    }
    
    //String get_prefix() { return prefix;}
    //void set_prefix(String p) { prefix = p; }

    /**
     * This method initialize the connection to database.
     * @param table_pref Set the prefix used in tables.
     * @param addr The MySQL connector address e.g."jdbc:mysql://localhost:3306/OC"
     * @param uname Database username.
     * @param pwd Database password.
     * @return 0, Success; -1, Unknown System error; -2, Connection error.
     */
    public int Initialize(String addr, String uname, String pwd) {
        try {
            Class.forName("com.mysql.jdbc.Driver");
            t = DBtype.MYSQL;
            dbconn = DriverManager.getConnection(addr, uname, pwd);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return -1;
        } catch (SQLException e) {
            e.printStackTrace();
            return -2;
        }
        return 0;
    }
    
    public int InitializeSQLite(String addr) {
        try {
            Class.forName("org.sqlite.JDBC");
            t = DBtype.SQLITE;
            dbconn = DriverManager.getConnection(addr);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            return -1;
        } catch (SQLException e) {
            e.printStackTrace();
            return -2;
        }
        return 0;
    }

    /**
     * This method create the database.
     * @param dbName The requested database name.
     * @param addr The header of jdbc address e.g. "jdbc:mysql://localhost:3306/"
     * @param uname Database username.
     * @param pwd Database password.
     * @return 0, Success; -1, Unknown System error; -2, Connection error.
     */
    public int createDatabase(String dbName, String addr, String uname, String pwd) {
    	int res = Initialize(addr,uname,pwd);
    	if(res!=0)
    		return -1;
        try {
            Statement s = dbconn.createStatement();
            s.executeUpdate("CREATE DATABASE IF NOT EXISTS "+dbName);
            s.close();
            dbconn.close();
            dbconn = DriverManager.getConnection(addr+dbName, uname, pwd);
            t = DBtype.MYSQL;
            s = dbconn.createStatement();
            s.addBatch("CREATE TABLE `users` ( `UID` BIGINT(20) UNSIGNED NOT NULL, `PwdMd5` VARCHAR(32) NOT NULL , `PwdSalt` VARCHAR(32) NOT NULL , `Username` VARCHAR(32) NOT NULL , `Email` VARCHAR(50) NOT NULL , `JoinTime` DATETIME NOT NULL , PRIMARY KEY (`UID`) , UNIQUE(`Email`)) ENGINE = InnoDB");
            s.addBatch("CREATE TABLE `login_history` ( `id` BIGINT UNSIGNED NOT NULL , `UID` BIGINT UNSIGNED NOT NULL , `login_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, `logout_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY (`id`) , FOREIGN KEY (`UID`) REFERENCES `users`(`UID`)) ENGINE = InnoDB");
            s.addBatch("CREATE TABLE `ships` ( `id` INT UNSIGNED NOT NULL , `Name` VARCHAR(50) NOT NULL, `Description` TEXT NOT NULL , `Type` ENUM('Destroyer','Cruiser','Battleship','Aircraft_carrier') NOT NULL , PRIMARY KEY (`id`)) ENGINE = InnoDB");
            s.addBatch("CREATE TABLE `teams` ( `id` INT UNSIGNED NOT NULL , `TeamName` VARCHAR(32) NOT NULL , `Member1` BIGINT UNSIGNED NOT NULL , `Member2` BIGINT UNSIGNED , `Member3` BIGINT UNSIGNED , PRIMARY KEY (`id`) , FOREIGN KEY (`Member1`) REFERENCES `users`(`UID`) , FOREIGN KEY (`Member2`) REFERENCES `users`(`UID`) , FOREIGN KEY (`Member3`) REFERENCES `users`(`UID`)) ENGINE = InnoDB");
            s.addBatch("CREATE TABLE `game_history` ( `id` BIGINT UNSIGNED NOT NULL , `Time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP , `Team1` INT UNSIGNED NOT NULL , `Team2` INT UNSIGNED NOT NULL , `Result` ENUM('T1Win','T2Win','WW') NOT NULL , PRIMARY KEY (`id`) , FOREIGN KEY (`Team1`) REFERENCES `teams`(`id`) , FOREIGN KEY (`Team2`) REFERENCES `teams`(`id`)) ENGINE = InnoDB");
            s.executeBatch();
        } catch (SQLException e) {
            e.printStackTrace();
            return -2;
        }
        return 0;
        
    }
    
    protected ResultSet simple_query(String SQL) throws SQLException {
    	if(st == null)
    		st = dbconn.createStatement();
		ResultSet rs = st.executeQuery(SQL);
		//st.close();
        return rs;
    }
    
    protected void simple_update(String SQL) throws SQLException {
    	if(st == null)
    		st = dbconn.createStatement();
		st.executeUpdate(SQL);
		st.close();
    }

    //! This method return the existing MySQL connection.
    public Connection getConnection() {
        return dbconn;
    }

}
