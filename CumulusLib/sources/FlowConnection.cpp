/* 
	Copyright 2010 OpenRTMFP
 
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License received along this program for more
	details (or else see http://www.gnu.org/licenses/).

	This file is a part of Cumulus.
*/

#include "FlowConnection.h"
#include "AMFReader.h"
#include "AMFWriter.h"
#include "Logs.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Cumulus {


FlowConnection::FlowConnection(Poco::UInt8 id,Peer& peer,ServerData& data) : Flow(id,peer,data) {
}

FlowConnection::~FlowConnection() {
}

int FlowConnection::requestHandler(UInt8 stage,PacketReader& request,PacketWriter& response) {

	char buff[MAX_SIZE_MSG];
	AMFReader reader(request);
	AMFWriter writer(response);

	switch(stage){
		case 0x01:
			request.readRaw(buff,6);
			response.writeRaw(buff,6);
			response.writeRaw("\x02\x0a\x02",3);
			request.readRaw(buff,6);
			response.writeRaw(buff,6);
			writer.write("_result");
			writer.write(1);
			writer.writeNull();

			writer.beginObject();
			writer.writeObjectProperty("objectEncoding",3);
			writer.writeObjectProperty("description","Connection succeeded");
			writer.writeObjectProperty("level","status");
			writer.writeObjectProperty("code","NetConnection.Connect.Success");
			writer.endObject();

			return 0x10;
		case 0x02: {

			request.readRaw(buff,6); // unknown, 11 00 00 03 96 00

			string tmp; 
			reader.read(tmp); // "setPeerInfo"

			reader.readNumber(); // Unknown, always equals at 0
			reader.readNull();

			while(reader.available()) {
				reader.read(tmp); // private host
				peer.addPrivateAddress(SocketAddress(tmp));
			}

			response.writeRaw("\x04\x00\x00\x00\x00\x00\x29\x00\x00",9); // Unknown!
			response.write16(data.keepAliveServer);
			response.write16(0); // Unknown!
			response.write16(data.keepAlivePeer);
			return 0x10;
		}
		default:
			ERROR("Unkown FlowNetConnection stage '%02x'",stage);
	}

	return 0;
}

} // namespace Cumulus