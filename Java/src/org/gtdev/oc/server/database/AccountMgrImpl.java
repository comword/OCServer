/**
 * @file AccountMgrImpl.java
 * @brief This file contain the wrap methods of database connection related to user account.
 * @auther Wang Daofei
 * Account class create table named "prefix_users" in database.
 */
package org.gtdev.oc.server.database;

public final class AccountMgrImpl extends Database {

    private AccountMgrImpl() {}

    public static String getUNamebyID(long UID) {
        return null;
    }

    public static String getUNamebyEmail(String Email) {
        return null;
    }

    public static int AuthPassword(long UID, String pwdMD5) {
    	
        return -1;
    }

    public static long getUIDbyName(String UName) {
        return 0;
    }

    public static long getUIDbyEmail(String Email) {
        return 0;
    }

    public static int createAccount(long UID, String pwdMd5) {
        return -1;
    }

    public static int createAccount(long UID, String UName, String pwdMd5) {
        return -1;
    }

    public static int createAccount(long UID, String UName, String UEmail, String pwdMd5) {
        return -1;
    }

    public static long getNextUID() {
        return -1;
    }

    public static int updateUName(long UID, String NewName) {
        return -1;
    }

    public static int updateUEmail(long UID, String Email) {
        return -1;
    }

    public static int updatePwd(long UID, String OldPwdMd5, String NewPwdMd5) {
        return -1;
    }

    public static int insLoginHistory(long UID, String IP_addr) {
        return -1;
    }

    public static String[] getLoginHistory(long UID, short limit) {
        return null;
    }

}
