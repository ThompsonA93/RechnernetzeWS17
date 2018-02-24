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

#include "HTTPClient.h"
#include "HTTPClientMsg_m.h"
#include "HTTPServerMsg_m.h"
#include "../udp/UDPControlInfo_m.h"

Define_Module(HTTPClient);

void HTTPClient::initialize() {
    // Initialize Params - FIXMEs
    srcPort = par("srcPort");
    destPort = par("destPort");

    // Initialize state
    state = REQUEST_INDEX;

    // Generate Request
    request = "/test/\r\n";
    method = "GET";

    // Generate HTTPMsg
    HTTPClientMsg* hcm = new HTTPClientMsg;
    hcm->setRequest(request);
    hcm->setMethod(method);

    UDPControlInfo* uci = new UDPControlInfo;
    uci->setSrcPort(srcPort);
    uci->setDestPort(destPort);

    // Attack ControlInfo
    hcm->setControlInfo(uci);

    // Send Msg.
    send(hcm, out);
}

void HTTPClient::handleMessage(cMessage *msg) {
    // Parse Info
    HTTPServerMsg* hsm = check_and_cast<HTTPServerMsg*>(msg);
    std::string resp = hsm->getResponse();

    //'Cache' Info
    receivedData.push_back(resp);

    if(receivedData.size() == 1)
    {
        // We cached an index.html. Now Assume further inputs
        state = REQUEST_DATA;
    }
    if(receivedData.size() > 1)
    {
        // We have requested everything. Do not duplicate the request.
        state = TRANSMISSION_FINISHED;
    }


    if(resp.find(".gif") != std::string::npos && state == REQUEST_DATA)
    {
        // Matched some .gif. Now Fetch!
        request = "/test/logo.gif\r\n";
        HTTPClientMsg* hcm = new HTTPClientMsg;
        hcm->setRequest(request);
        hcm->setMethod(method);

        UDPControlInfo* uci = new UDPControlInfo;
        uci->setSrcPort(srcPort);
        uci->setDestPort(destPort);

        // Attack ControlInfo
        hcm->setControlInfo(uci);


        send(hcm, out);

    }
    if(resp.find(".png") != std::string::npos && state == REQUEST_DATA)
    {
        // Found some .png
        request = "/test/TechnikErleben.png\r\n";
        HTTPClientMsg* hcm = new HTTPClientMsg;
        hcm->setRequest(request);
        hcm->setMethod(method);


        UDPControlInfo* uci = new UDPControlInfo;
        uci->setSrcPort(srcPort);
        uci->setDestPort(destPort);

        // Attack ControlInfo
        hcm->setControlInfo(uci);

        send(hcm, out);
    }
    delete msg, resp;
}
