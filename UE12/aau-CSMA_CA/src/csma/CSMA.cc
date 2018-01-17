#include "CSMA.h"
#include "CSMAFrame_m.h"

Define_Module(CSMA);

void CSMA::initialize()
{
    this->backoffTimeout = NULL;    // SIFS*FACTOR
    this->rtsTimeout = NULL;        // DIFS
    this->colTimeout = NULL;        // SIFS
    this->maxBackoff = 6.0;
    this->SIFS = 1.0;
    this->DIFS = 3.0;
    this->transmitDuration = 10.0;
    this->numOfConcurrentMsgs = 0;
    this->srcMAC = new MACAddress(getParentModule()->par("macAddress").stringValue());
    this->destMAC = new MACAddress(getParentModule()->par("receiverMac").stringValue());

    findAndSetReachableDevices();
}

void CSMA::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()){
        // Msg is a timeout.
        handleSelfMessage(msg);

    } else if (msg->arrivedOn("inUpperLayer")) {
        // Comes from above.
        handleUpperLayerMessage(msg);

    } else if (msg->arrivedOn("inLowerLayer")) {
        // Comes from net.
        handleLowerLayerMessage(msg);
    }
}

void CSMA::handleSelfMessage(cMessage *msg){

    if (msg == backoffTimeout) {
        // TODO:
        // Retransmit RTS after backoff.
        CSMAFrame* cf = new CSMAFrame;
        send(cf, "outUpperLayer");
        // Schedule another timeout for expected response to RTS.

        this->maxBackoff = 6.0;
        // Reset handled timeout

    } else if (msg == rtsTimeout) {
        // TODO:
        // Create random backoff before trying to re-send an RTS.
        DIFS = rand() % 30;
        // Reset handled timeout

    } else if(msg == colTimeout){
        // TODO:
        // Check if more than one msg arrived during the timeout.
        // If so, it's a collision -> Delete messages and reset counter.
        // Reset handled timeout
    }
}

void CSMA::handleUpperLayerMessage(cMessage *msg)
{
    // TODO
    // Send new RTS
    CSMAFrame* f = new CSMAFrame;

    f->setType(RTS); //FIXME boiiiiiiiii iii iiiiiiiii
    f->setResDuration(0);
    f->setSrc(*srcMAC);
    f->setDest(*destMAC);
    sendToAllReachableDevices(f);

    // Save msg
    CSMAFrame* csmaF = new CSMAFrame;
    csmaF->setType(DATA);
    csmaF->setResDuration(0);
    csmaF->setSrc(*srcMAC);
    csmaF->setDest(*destMAC);
    // TODO & ADD ?? CSMAControlInfo
    csmaF->encapsulate((cPacket*)msg);

    msgBuffer.push_back(csmaF);
}

void CSMA::handleLowerLayerMessage(cMessage *msg)
{
    // Msg is a CSMA frame.
    CSMAFrame *frame = check_and_cast<CSMAFrame *>(msg);

    if (frame->getDest().equals(*srcMAC)) {
        // It is for us
        handleMessageForMe(frame);

    } else {
        // It is for someone else
        handleMessageForOthers(frame);
    }
}

void CSMA::handleMessageForMe(CSMAFrame *frame)
{
    switch (frame->getType()) {
        case RTS: {
            // TODO

            break;
        }
        case CTS: {
            // TODO

            break;
        }
        case DATA: {
            // TODO

            break;
        }
        case ACK: {
            // TODO

            break;
        }
        default:
            EV << "Message type not recognised.\n";
            break;
    }
}

void CSMA::handleMessageForOthers(CSMAFrame *frame)
{
    switch (frame->getType()) {
        case RTS: {
            // TODO

            break;
        }
        case CTS: {
            // TODO

            break;
        }
        case DATA: {
            // TODO?
            break;
        }
        case ACK: {
            // TODO?
            break;
        }
        default:
            EV << "Message type not recognised.\n";
            break;
    }
}

void CSMA::sendToAllReachableDevices(cMessage *msg){
    for (uint i = 0; i < reachableDevices.size(); ++i) {
        sendDirect(msg->dup(), reachableDevices.at(i), "WLAN");
    }
}

/**
 * Allows to set which devices are reachable by which.
 * Individually customisable for each device by commenting out the ones
 * that shoud not be reached.
 */
void CSMA::findAndSetReachableDevices()
{
    std::string name = this->getParentModule()->getName();
    reachableDevices.reserve(5);

    if (name == "accessPointServer") {
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client1"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client2"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client3"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client4"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("accessPointServer"));

    } else if (name == "client1") {
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client1"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client2"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client3"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client4"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("accessPointServer"));

    } else if (name == "client2"){
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client1"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client2"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client3"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client4"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("accessPointServer"));

    } else if (name == "client3"){
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client1"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client2"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client3"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client4"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("accessPointServer"));

    } else if (name == "client4"){
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client1"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client2"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client3"));
//        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("client4"));
        reachableDevices.push_back(getParentModule()->getParentModule()->getSubmodule("accessPointServer"));
    }
}
