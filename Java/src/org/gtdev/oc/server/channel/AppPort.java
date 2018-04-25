package org.gtdev.oc.server.channel;

import org.gtdev.oc.server.protocol.*;

public interface AppPort {
	/**
     * The first transaction code available for user commands.
     */
    int FIRST_CALL_TRANSACTION  = 0x00000001;
    /**
     * The last transaction code available for user commands.
     */
    int LAST_CALL_TRANSACTION   = 0x00ffffff;
    
    /**
     * Protocol transaction code: pingServer().
     */
    int PING_TRANSACTION        = ('_'<<24)|('P'<<16)|('N'<<8)|'G';
    
    /**
     * Protocol transaction code: dump internal state.
     */
    int DUMP_TRANSACTION        = ('_'<<24)|('D'<<16)|('M'<<8)|'P';
    
    /**
     * Protocol transaction code: execute a shell command.
     */
    int SHELL_COMMAND_TRANSACTION = ('_'<<24)|('C'<<16)|('M'<<8)|'D';
    /**
     * Protocol transaction code: interrogate the recipient side
     * of the transaction for its canonical interface descriptor.
     */
    int INTERFACE_TRANSACTION   = ('_'<<24)|('N'<<16)|('T'<<8)|'F';
    
	public abstract class AppPortImpl implements AppPort {
		private static String InterfaceToken;
		int resultCode = 1000;
		String errMsg = "";
		int msgCookie = 0;
		byte[] Buffer;
		BaseRemoteCommand serviceCmd;
		
		public byte[] dump() {
			return Buffer;
		}
		
		public boolean transact(int code, ProtoInputStream data, ProtoOutputStream reply, int flags) {
			if (code == INTERFACE_TRANSACTION) {
	            reply.write(InterfaceToken, 1);
	            return true;
	        }
			return false;
		}
		
		//This method is CPP entry point.
		@SuppressWarnings("unused")
		private boolean execTransact(int code, byte[] dataObj, byte[] replyObj, int flags) {
			ProtoInputStream is = new ProtoInputStream(dataObj);
			ProtoOutputStream os = new ProtoOutputStream();
			boolean res = transact(code,is,os,flags);
			replyObj = os.toByteArray();
			return res;
		}
	}
	byte[] dump();
	boolean transact(int code, ProtoInputStream data, ProtoOutputStream reply, int flags);
}