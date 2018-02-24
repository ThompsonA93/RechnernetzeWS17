#include "CSMA.h"
#include "../ip/IPDatagram_m.h"

Define_Module(CSMA);

void CSMA::initialize()
{
    this->backoffTimeout = NULL;
    this->rtsTimeout = NULL;
    this->colTimeout = NULL;
    this->maxBackoff = 6.0;
    this->SIFS = 1.0;
    this->DIFS = 3.0;
    this->transmitDuration = 10.0;
    this->numOfConcurrentMsgs = 0;
    this->srcMAC = new MACAddress(getParentModule()->par("macAddress").stringValue());
    this->destMAC = new MACAddress(getParentModule()->par("receiverMac").stringValue());

    findAndSetReachableDevices();

    this->finished = false;
    this->new_data = false;
    this->transmitData = NULL;
    this->ctsTimeout = NULL;

    // seed random numbers
    srand(time(0));
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
        EV << "backoff timer expired, resend RTS" << endl;

        // TODO:
        // Retransmit RTS after backoff.
        // Schedule another timeout for expected response to RTS.
        // Reset handled timeout

        // prepare & (re)send RTS request
        CSMAFrame *rts = new CSMAFrame;
        rts->setType(RTS);
        rts->setResDuration(transmitDuration);
        rts->setSrc(*srcMAC);
        rts->setDest(*destMAC);

        sendToAllReachableDevices((cPacket*) rts);

        rtsTimeout = new cMessage("rtsTimeout");
        scheduleAt(simTime() + DIFS, rtsTimeout);
        cancelEvent(backoffTimeout);
//        backoffTimeout = NULL;

    } else if (msg == rtsTimeout) {
        // TODO:
        // Create random backoff before trying to re-send an RTS.
        // Reset handled timeout

        double rndDelay = rand() % ((int)maxBackoff * 10) / 10.0;

        backoffTimeout = new cMessage("backoffTimeout");
        scheduleAt(simTime() + rndDelay, backoffTimeout);
        cancelEvent(rtsTimeout);
//        rtsTimeout = NULL;

        EV << "handling rtsTimeout; wait " << rndDelay << " seconds" << endl;

    } else if(msg == colTimeout){
        EV << "handling colTimeout; " << endl;

        // TODO:
        // Check if more than one msg arrived during the timeout.
        // If so, it's a collision -> Delete messages and reset counter.
        // Reset handled timeout

        if (numOfConcurrentMsgs > 1) {
            EV << "collision detected" << endl;
            delete msg;
        } else {
            EV << "sending CTS" << endl;

            // connection with client may be established
            CSMAFrame *rcvframe = check_and_cast<CSMAFrame*>(msg);

            // prepare CTS message
            CSMAFrame *rframe = new CSMAFrame;
            rframe->setType(CTS);
            rframe->setResDuration(rcvframe->getResDuration());
            rframe->setSrc(rcvframe->getDest());
            rframe->setDest(rcvframe->getSrc());

            // send
            sendToAllReachableDevices((cPacket*) rframe);

//            connectedMAC = rcvframe->getSrc();
        }

        // reset related variables
        cancelEvent(colTimeout);
//        colTimeout = NULL;
        numOfConcurrentMsgs = 0;
    } else if(msg == ctsTimeout) {
        EV << "other client's session expired => send new RTS" << endl;

        // resend RTS now that other client's session has expired

        // prepare & (re)send RTS request
        CSMAFrame *rts = new CSMAFrame;
        rts->setType(RTS);
        rts->setResDuration(transmitDuration);
        rts->setSrc(*srcMAC);
        rts->setDest(*destMAC);

        sendToAllReachableDevices((cPacket*) rts);

        rtsTimeout = new cMessage("rtsTimeout");
        scheduleAt(simTime() + DIFS, rtsTimeout);
        cancelEvent(ctsTimeout);
//        ctsTimeout = NULL;
    }
}

void CSMA::handleUpperLayerMessage(cMessage *msg)
{
    // TODO

    EV << "received data from upper layer" << endl;

    // take data packet for transmission
    IPDatagram *ipd = check_and_cast<IPDatagram*>(msg);
    transmitData = ipd->decapsulate();

    // send RTS request to start connection
    CSMAFrame *rts = new CSMAFrame;
    rts->setType(RTS);
    rts->setResDuration(transmitDuration);
    rts->setSrc(*srcMAC);
    rts->setDest(*destMAC);

    sendToAllReachableDevices((cPacket*) rts);

    rtsTimeout = new cMessage("rtsTimeout");
    scheduleAt(simTime() + DIFS, rtsTimeout);
    new_data = true;

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

void CSMA::handleMessageForMe(CSMAFrame *frame) {
    switch (frame->getType()) {
    case RTS: {
        EV << "CSMA::received RTS; waiting" << endl;

        // TODO

        // RTS received -> wait for more in case of collisions
        numOfConcurrentMsgs++;
        colTimeout = (cPacket*)frame;//new cMessage("colTimeout");
        scheduleAt(simTime() + SIFS, colTimeout);

        break;
    }
    case CTS: {
        EV << "CSMA::received CTS response; sending data to server" << endl;

        // TODO

        // send data packet
        CSMAFrame *dataframe = new CSMAFrame;
        dataframe->setType(DATA);
        dataframe->setSrc(*srcMAC);
        dataframe->setDest(*destMAC);
        dataframe->encapsulate((cPacket*)transmitData->dup());

        sendToAllReachableDevices((cPacket*) dataframe);
        new_data = false;

        break;
    }
    case DATA: {
        EV << "CSMA::received data; sending ack" << endl;
        // TODO

        // send ack
        CSMAFrame *ackframe = new CSMAFrame;
        ackframe->setType(ACK);
        ackframe->setSrc(frame->getDest());
        ackframe->setDest(frame->getSrc());

        sendToAllReachableDevices((cPacket*) ackframe);

        break;
    }
    case ACK: {
        EV << "CSMA::received ack; " << endl;

        // TODO

        // send more data packets; or do nothing if finished
        if (new_data) {
            EV << "sending next data packet" << endl;
            // send data
            CSMAFrame *dataframe = new CSMAFrame;
            dataframe->setType(DATA);
            dataframe->setSrc(*srcMAC);
            dataframe->setDest(*destMAC);
            dataframe->encapsulate((cPacket*)transmitData);

            sendToAllReachableDevices((cPacket*) dataframe);
            new_data = false;
        } else {
            EV << "finished transmission; stop" << endl;
            finished = true;
            if (rtsTimeout != NULL) cancelEvent(rtsTimeout);
            if (backoffTimeout != NULL)cancelEvent(backoffTimeout);
            if (ctsTimeout != NULL) cancelEvent(ctsTimeout);
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
            // TODO?

            // do nothing
            break;
        }
        case CTS: {
            EV << "received other client's CTS; ";

            // TODO

            cancelEvent(rtsTimeout);
            cancelEvent(backoffTimeout);

            if (!finished) {
                EV << "wait for other connection to time out" << endl;

                double waitPeriod = frame->getResDuration();
                ctsTimeout = new cMessage("ctsTimeout");
                scheduleAt(simTime() + waitPeriod, ctsTimeout);
            } else {
                EV << "do nothing" << endl;
            }

            break;
        }
        case DATA: {
            // TODO?

            // do nothing
            break;
        }
        case ACK: {
            // TODO?

            // do nothing
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
 * that should not be reached.
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
