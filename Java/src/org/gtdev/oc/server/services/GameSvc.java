package org.gtdev.oc.server.services;

import org.gtdev.oc.server.channel.AppPort;
import org.gtdev.oc.server.protocol.*;

public class GameSvc extends AppPort.AppPortImpl {
	static final String TAG = "GameSvc";
	
	@Override
    public boolean transact(ProtoInputStream data, ProtoOutputStream reply, int flags) {
		
		return false;
	}
}
