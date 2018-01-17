//
// Generated file, do not edit! Created by nedtool 5.2 from udp/UDPSegment.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __UDPSEGMENT_M_H
#define __UDPSEGMENT_M_H

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0502
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>udp/UDPSegment.msg:21</tt> by nedtool.
 * <pre>
 * packet UDPSegment
 * {
 *     int srcPort = -1;     // source port
 *     int destPort = -1;     // destination port 
 *     int length = 0;		 // length of whole datagram
 *     int checksum = 0;		 // checksum
 * }
 * </pre>
 */
class UDPSegment : public ::omnetpp::cPacket
{
  protected:
    int srcPort;
    int destPort;
    int length;
    int checksum;

  private:
    void copy(const UDPSegment& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const UDPSegment&);

  public:
    UDPSegment(const char *name=nullptr, short kind=0);
    UDPSegment(const UDPSegment& other);
    virtual ~UDPSegment();
    UDPSegment& operator=(const UDPSegment& other);
    virtual UDPSegment *dup() const override {return new UDPSegment(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getSrcPort() const;
    virtual void setSrcPort(int srcPort);
    virtual int getDestPort() const;
    virtual void setDestPort(int destPort);
    virtual int getLength() const;
    virtual void setLength(int length);
    virtual int getChecksum() const;
    virtual void setChecksum(int checksum);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const UDPSegment& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, UDPSegment& obj) {obj.parsimUnpack(b);}


#endif // ifndef __UDPSEGMENT_M_H

