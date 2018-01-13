//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// (c) 2017 Christian Timmerer, Alpen-Adria-Universität Klagenfurt / Bitmovin Inc.
//          christian.timmerer@itec.aau.at / christian.timmerer@bitmovin.com
//
// 621.800 (17W) Computer Networks and Network Programming

#ifndef UDP_H_
#define UDP_H_

#include "../3rdParty/IPv4Address.h"
#include "../3rdParty/IPv6Address.h"

using namespace omnetpp;

class UDP : public cSimpleModule
{
private:
    int srcPort, destPort;
    inet::IPv4Address srcIPv4, destIPv4;
    inet::IPv6Address srcIPv6, destIPv6;

protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void handleAppMessage(cPacket *msg);
	virtual void handleUDPSegment(cPacket *msg);
};

#endif /* UDP_H_ */
