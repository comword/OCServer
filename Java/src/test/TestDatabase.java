/**
 * @file TestDatabase.java
 * @auther Wang Daofei
 * This file use JUnit to test the implement of the database.
 */
package test;

import static org.junit.Assert.*;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

import org.gtdev.oc.server.MD5;
import org.gtdev.oc.server.database.AccountMgrImpl;
import org.junit.Test;

public class TestDatabase {
	
	@SuppressWarnings("unused")
	@Test
	public void DB_test() throws IOException {
		Files.copy(new File("OCDatabase.db.orig").toPath(), new File("OCDatabase.db").toPath(), StandardCopyOption.REPLACE_EXISTING);
		AccountMgrImpl db = AccountMgrImpl.getInstance();
		assertEquals(0, db.InitializeSQLite("jdbc:sqlite:OCDatabase.db"));
		long UID = db.createAccount("test1", "test1@example.com", MD5.toMD5("12345qwert"));
		long[] test1 = db.getUIDbyName("test1");
		assertEquals(UID, db.getUIDbyName("test1")[0]);
		assertEquals(UID, db.getUIDbyEmail("test1@example.com"));
		assertEquals(0, db.getUIDbyName("notexist").length);
		assertEquals(-1, db.getUIDbyEmail("wrong@email.com"));
		assertEquals(0, db.AuthPassword(UID, MD5.toMD5("12345qwert")));
		assertFalse(db.AuthPassword(UID, MD5.toMD5("wrongpassword"))==0);
		assertFalse(db.AuthPassword(123456789, MD5.toMD5("wrongpassword"))==0);
	}
}
