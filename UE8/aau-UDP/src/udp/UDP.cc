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

#include <omnetpp.h>
#include "UDP.h"
#include "UDPSegment_m.h"
#include "UDPControlInfo_m.h"
#include "../3rdParty/IPv4Address.h"
#include "../3rdParty/IPv6Address.h"
Define_Module(UDP);

void UDP::initialize(){}

void UDP::handleMessage(cMessage *msg)
{
	if (msg->arrivedOn("fromUpperLayer")) {
		// comes from application
		this->handleAppMessage((cPacket*)msg);
	}

	else if (msg->arrivedOn("fromLowerLayer")) {
		//comes from lower layer
		this->handleUDPSegment((cPacket*)msg);
	}
}

void UDP::handleAppMessage(cPacket *msg)
{
    // Message from application layer
    // Strip UDP-Controlinfo
    UDPControlInfo* uci = (UDPControlInfo*)msg->getControlInfo();

    UDPSegment* us = new UDPSegment;
    us->setSrcPort(uci->getSrcPort());
    us->setDestPort(uci->getDestPort());

    us->encapsulate(msg);

    EV << "Sending to lower Layer" << std::endl;
    send(us, "toLowerLayer");
}

void UDP::handleUDPSegment(cPacket *msg)
{
    UDPSegment* us = check_and_cast<UDPSegment*>(msg);
    cPacket* cp = msg->decapsulate();

    UDPControlInfo* uci = new UDPControlInfo;
    uci->setDestPort(us->getDestPort());
    uci->setSrcPort(us->getSrcPort());

    cp->setControlInfo(uci);

    EV << "Sending to upper layer" << std::endl;
    send(cp, "toUpperLayer");
}
