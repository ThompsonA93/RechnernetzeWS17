#include "CSMA.h"
#include "CSMAFrame_m.h"
#include "CSMAControlInfo_m.h"
#include "../ip/IPDatagram_m.h"

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

        if (msgBuffer.empty()) {

            CSMAFrame* rtsFrame = new CSMAFrame;

            rtsFrame->setSrc(msgBuffer.front()->getSrc());
            rtsFrame->setDest(msgBuffer.front()->getDest());
            rtsFrame->setType(RTS);

            sendToAllReachableDevices(rtsFrame);

            // Schedule another timeout for expected response to RTS.

            if (!rtsTimeout->isScheduled()) {
                scheduleAt(simTime() + DIFS, rtsTimeout);

            }
            // Reset handled timeout

        }

    } else if (msg == rtsTimeout) {
        // TODO:
        // Create random backoff before trying to re-send an RTS.
        simtime_t delay = SIFS * (rand() % 10);

        if (!backoffTimeout->isScheduled()) {
            scheduleAt(simTime() + delay, backoffTimeout);
        }
        // Reset handled timeout

    } else if(msg == colTimeout){
        // TODO:
        // Check if more than one msg arrived during the timeout.
        // If so, it's a collision -> Delete messages and reset counter.
        // Reset handled timeout


        if (numOfConcurrentMsgs > 1) {

            msgBuffer.clear();
            numOfConcurrentMsgs = 0;

        }
        else if (numOfConcurrentMsgs == 1) {

            CSMAFrame *ctsFrame = new CSMAFrame;

            ctsFrame->setSrc(*srcMAC);
            ctsFrame->setDest(msgBuffer.front()->getDest());
            ctsFrame->setType(CTS);

            sendToAllReachableDevices(ctsFrame);

            msgBuffer.clear();
            numOfConcurrentMsgs = 0;
        }
    }
}

void CSMA::handleUpperLayerMessage(cMessage *msg)
{
    // TODO

    //get control info from received upper layer datagram

    IPDatagram *ipData = check_and_cast<IPDatagram*>(msg);
    CSMAControlInfo *cInfo = check_and_cast<CSMAControlInfo*>(ipData->removeControlInfo());


    //create CSMAFrame
    CSMAFrame *dataFrame = new CSMAFrame;
    dataFrame->setSrc(cInfo->getSrc());
    dataFrame->setDest(cInfo->getDest());
    dataFrame->setType(DATA);
    dataFrame->encapsulate(ipData);


    // send RTS if there is no older frame waiting / nobody else has
    // sent an RTS -> REQUEST TO SEND

    if (msgBuffer.empty() && !backoffTimeout->isScheduled()) {

        CSMAFrame *rts = new CSMAFrame;

        rts->setSrc(cInfo->getSrc());
        rts->setDest(cInfo->getDest());
        rts->setType(RTS);

        sendToAllReachableDevices(rts);


        if (!rtsTimeout->isScheduled()) {

            scheduleAt(simTime() + DIFS, rtsTimeout);
        }
    }

    msgBuffer.push_back(dataFrame);
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

            numOfConcurrentMsgs++;

            msgBuffer.push_back(frame);

            if (!colTimeout->isScheduled()) {
                EV << name << ": collision timeout started\n";

                scheduleAt(simTime() + SIFS, colTimeout);
            }

            break;
        }
        case CTS: {
            // TODO

            // check if we are still waiting for CTS and
            // if its so, cancel the timeout and send data.
            if (rtsTimeout->isScheduled()) {

                cancelEvent(rtsTimeout);
                sendToAllReachableDevices(msgBuffer.front());

            }
            break;
        }
        case DATA: {
            // TODO
            // received data frame -> now send ack frame to
            // source of data.

            EV << name << ": DATA receviced -> send ACK\n";
            CSMAFrame *ackFrame = CSMAFrame;

            ackFrame->setSrc(*srcMAC);
            ackFrame->setDest(frame->getSrc());
            ackFrame->setType(ACK);

            sendToAllReachableDevices(ackFrame);

            break;
        }
        case ACK: {
            // TODO

            EV << name << ": ACK received !!\n";

            // receiver got data -> delet it from buffer
            msgBuffer.pop_front();

            //check if there are still messages waiting.

            if (!msgBuffer.empty()) {

                CSMAFrame *rts = new CSMAFrame;
                rts->setSrc(msgBuffer.front()->getSrc());
                rts->setDest(msgBuffer.front()->getDest());
                rts->setType(RTS);

                sendToAllReachableDevices(rts);
            }

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

            if (!backoffTimeout->isScheduled()) {

               scheduleAt(simTime() + (SIFS * (rand() % 10)), backoffTimeout);

            }
            break;
        }
        case CTS: {
            // TODO

            if (backoffTimeout->isScheduled()) {

                cancelEvent(backoffTimeout));

                scheduleAt(simTime() + frame->getResDuration(), backoffTimeout);
            }
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
