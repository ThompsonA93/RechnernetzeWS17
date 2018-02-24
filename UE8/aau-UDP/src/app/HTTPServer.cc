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

#include "HTTPServer.h"
#include "HTTPClientMsg_m.h"
#include "HTTPServerMsg_m.h"
#include "../udp/UDPControlInfo_m.h"

Define_Module(HTTPServer);

void HTTPServer::initialize()
{
    // TODO use from previous exercise and extend if needed
}

void HTTPServer::handleMessage(cMessage *msg)
{
    // Fetch and Parse
    HTTPClientMsg* hcm = check_and_cast<HTTPClientMsg*>(msg);
    // TODO: request to string & Substring
    const char* reqIn = hcm->getRequest();  // /test/\r\n || /test/logo.gif
    const char* meth = hcm->getMethod();  //  GET

    // Filter Request
    std::string method(meth);
    std::string request(reqIn);
    request = request.substr(0, request.rfind("\r"));

    EV << "\nRequest:\n" << request << "\nMethod\n"<< method << std::endl;
    if(method == "GET")
    {
        EV << "Echo::GET" << std::endl;
        // Refine operations on request
        if(request == "/test/") // FIXME Why the fuck even
        {
            HTTPServerMsg* rMsg = new HTTPServerMsg;
            UDPControlInfo* uci = new UDPControlInfo;
            uci->setSrcPort(srcPort);
            uci->setDestPort(destPort);
            rMsg->setControlInfo(uci);

            rMsg->setResponse(data[0].c_str());
            bubble("Sending index.html");
            send(rMsg, out);
        }
        else if(request == "/test/logo.gif")
        {
            HTTPServerMsg* rMsg = new HTTPServerMsg;
            UDPControlInfo* uci = new UDPControlInfo;
            uci->setSrcPort(srcPort);
            uci->setDestPort(destPort);
            rMsg->setControlInfo(uci);

            rMsg->setResponse(data[1].c_str());
            bubble("Sending .gif");
            send(rMsg, out);
        }
        else if(request == "/test/TechnikErleben.png")
        {
            HTTPServerMsg* rMsg = new HTTPServerMsg;
            UDPControlInfo* uci = new UDPControlInfo;
            uci->setSrcPort(srcPort);
            uci->setDestPort(destPort);
            rMsg->setControlInfo(uci);

            rMsg->setResponse(data[2].c_str());
            bubble("Sending .png");
            send(rMsg, out);
        }
        else{
            // Invalid request
            HTTPServerMsg* rMsg = new HTTPServerMsg;
            UDPControlInfo* uci = new UDPControlInfo;
            uci->setSrcPort(srcPort);
            uci->setDestPort(destPort);
            rMsg->setControlInfo(uci);
            rMsg->setResponse("Error 404");
            bubble("Error!");
            send(rMsg, out);
        }

        delete msg, request, method;
    }else{
        // Invalid Method
        bubble("Invalid Request!");
        delete hcm, msg, request, method;
    }
}
