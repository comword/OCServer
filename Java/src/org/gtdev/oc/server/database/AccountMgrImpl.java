/**
 * @file AccountMgrImpl.java
 * @brief This file contain the wrap methods of database connection related to user account.
 * @auther Wang Daofei
 * Account class manipulate the database connection, and handle the request to the user account.
 */
package org.gtdev.oc.server.database;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import org.gtdev.oc.server.MD5;
import org.gtdev.oc.server.protocol.Utils;

public final class AccountMgrImpl extends Database {

    private AccountMgrImpl() {}
    
    private static AccountMgrImpl ins = null;
    
    public static AccountMgrImpl getInstance() {
    	if(ins == null)
    		ins = new AccountMgrImpl();
    	return ins;
    }

    public String getUNamebyID(long UID) {
    	String sql="SELECT `Username` FROM `users` WHERE `UID` = '"+UID+"'";
    	String res = null;
    	try {
    		ResultSet r = simple_query(sql);
			r.next();
			res = r.getString("Username");
		} catch (SQLException e) {
			e.printStackTrace();
		}
        return res;
    }

    public int AuthPassword(long UID, String pwdMD5) {
    	String sql="SELECT `pwdMd5`, `PwdSalt` FROM `users` WHERE `UID` = '"+UID+"'";
    	try {
    		ResultSet r = simple_query(sql);
			if(!r.next())
				return -1; //bad user id.
			String rt = r.getString("pwdMd5");
			String salt = r.getString("PwdSalt");
			String pwd2 = getPwdMd5(UID, pwdMD5, salt);
//			System.out.println("AUTH");
//			System.out.println(UID);
//	    	System.out.println(pwdMD5);
//			System.out.println(salt);
//			System.out.println(pwd2);
			if(rt.equals(pwd2))
				return 0;
			return -2; //bad password.
		} catch (SQLException e) {
			e.printStackTrace();
		}
        return -3; 
    }
    
    private String getPwdMd5(long UID, String pwdMd5, String Salt) {
    	byte[] a = Utils.hexStr2Bytes(pwdMd5);
    	for(int i=0;i<a.length;i++)
    		a[i] += UID;
    	String b = Utils.getHexdump(a);
    	StringBuilder res = new StringBuilder();
    	for(int i=0;i<b.length();i++) {
    		res.append(b.charAt(i)).append(Salt.charAt(i));
    	}
    	return MD5.toMD5(res.toString());
    }

    public long[] getUIDbyName(String UName) {
    	String sql="SELECT `UID` FROM `users` WHERE `Username` = '"+UName+"'";
    	List<Long> res = new ArrayList<Long>();
    	try {
			ResultSet r = simple_query(sql);
			while(r.next()) {
				res.add(r.getLong("UID"));
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
        return res.stream().mapToLong(i->i).toArray();
    }

    public long getUIDbyEmail(String Email) {
    	String sql="SELECT `UID` FROM `users` WHERE `Email` = '"+Email+"'";
    	try {
			ResultSet r = simple_query(sql);
			if (r.next() )
				return r.getLong("UID");
		} catch (SQLException e) {
			e.printStackTrace();
		}
        return -1;
    }
    
    private long getnextUID() {
    	String sql="SELECT `UID` FROM `users` ORDER BY `UID` DESC LIMIT 1";
    	try {
			ResultSet r = simple_query(sql);
			if (!r.next() ) {
			    return 1;
			} else {
				return r.getLong("UID")+1;
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
    	return -1;
    }
    
    private String getRandSalt() {
    	byte[] b = new byte[16];
    	new Random().nextBytes(b);
    	return Utils.getHexdump(b);
    }
    
    public long createAccount(String UName, String UEmail, String pwdMd5) {
    	long nextUID = getnextUID();
    	String Salt = getRandSalt();
    	String pwd2 = getPwdMd5(nextUID,pwdMd5,Salt);
//    	System.out.println("CREATE");
//    	System.out.println(nextUID);
//    	System.out.println(pwdMd5);
//		System.out.println(Salt);
//		System.out.println(pwd2);
    	String sql;
    	if(t == DBtype.MYSQL) {
    		sql="INSERT INTO `users` (`UID`, `PwdMd5`, `PwdSalt`, `Username`, `Email`, `JoinTime`) "
        			+ "VALUES ('"+nextUID+"', '"+pwd2+"', '"+Salt+"', '"+UName+"', '"+UEmail+"', NOW())";
    	} else {
    		sql="INSERT INTO `users` (`UID`, `PwdMd5`, `PwdSalt`, `Username`, `Email`, `JoinTime`) "
        			+ "VALUES ('"+nextUID+"', '"+pwd2+"', '"+Salt+"', '"+UName+"', '"+UEmail+"', date('now'))";
    	}
    	
    	try {
    		simple_update(sql);
    		return nextUID;
		} catch (SQLException e) {
			e.printStackTrace();
		}
        return -1;
    }

    public int updateUName(long UID, String NewName) {
    	String sql = "UPDATE `users` SET `Username` = '"+NewName+"' WHERE `UID` = '"+UID+"'";
    	try {
    		simple_update(sql);
    		return 0;
		} catch (SQLException e) {
			e.printStackTrace();
		}
        return -1;
    }

    public int updateUEmail(long UID, String Email) {
    	String sql = "UPDATE `users` SET `Email` = '"+Email+"' WHERE `UID` = '"+UID+"'";
    	try {
    		simple_update(sql);
    		return 0;
		} catch (SQLException e) {
			e.printStackTrace();
		}
        return -1;
    }

    public int updatePwd(long UID, String OldPwdMd5, String NewPwdMd5) {
    	int auth = AuthPassword(UID, OldPwdMd5);
    	if(auth != 0) //Bad old password.
    		return -1;
    	String Salt = getRandSalt();
    	String pwd2 = getPwdMd5(UID,NewPwdMd5,Salt);
    	String sql = "UPDATE `users` SET `PwdMd5` = '"+pwd2+"', `PwdSalt` = '"+Salt+"' WHERE `UID` = '"+UID+"'";
    	try {
    		simple_update(sql);
    		return 0;
		} catch (SQLException e) {
			e.printStackTrace();
		}
    	return -2;
    }
}
