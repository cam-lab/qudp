#include <QDebug>
#include <QThread>
#include <QHostAddress>
#include <QElapsedTimer>

#include <cstdlib>

#include "../common/RawStreamTester.h"
#include "qudp_lib.h"

//------------------------------------------------------------------------------
// max UDP payload = 1472 (octets)
// UDP overhead    = 66 (octets)
//
// max relative speed (for UDP palyload = 1472) = 0.9571
// max relative speed (for UDP palyload = 1024) = 0.9394
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#define UDP_LIB_ADDR_FORMAT

QThread::Priority MainThreadPriority   = QThread::NormalPriority;
QThread::Priority WorkerThreadPriority = QThread::NormalPriority;

const unsigned BundleNum = 128;
const unsigned Timeout   = 2000;
const int SocketBufSize  = 8*1024*1024;

const unsigned TypeInterval = 10000;

//------------------------------------------------------------------------------
const int PrgParams = 9;
// argv[0] - program name
// argv[1] - PacketNum
// argv[2] - host IP
// argv[3] - Port
// argv[4] - PacketSize
// argv[5] - PacketsInBuf
// argv[6] - PacketGenType
// argv[7] - TxDelay
// argv[8] - device IP
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
static unsigned long convertIpAddr(unsigned long addr);
static bool typeInfo(unsigned nBuf, UDP_LIB::TStatus status, UDP_LIB::Transfer& transfer);

//------------------------------------------------------------------------------
class TNetworkTest
{
    public:
        TNetworkTest() { UDP_LIB::init(); }
        ~TNetworkTest() { UDP_LIB::cleanUp(); }
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    if(argc < PrgParams) {
        printf("No input parameters\n");
        return 0;
    }

    //---
    const unsigned BufNum           = atoi(argv[1]);
    const char*    HostAddrStr      = argv[2];
    const unsigned Port             = atoi(argv[3]);
    const unsigned PacketSize       = atoi(argv[4]);
    const unsigned PacketsInBuf     = atoi(argv[5]);
    const unsigned PacketGenType    = atoi(argv[6]);
    const unsigned TxDelay          = atoi(argv[7]);
    const char*    PeerAddrStr      = argv[8];

    //---
    printf("--------------------------------------------\n");
    printf("[INFO] [%s start]\n\n",argv[0]);
    printf("[PARAM] BufNum:     %8d\n",BufNum);
    printf("[PARAM] PacketSize:    %5d\n",PacketSize);
    printf("[PARAM] PacketsInBuf:  %5d\n",PacketsInBuf);
    printf("[PARAM] TxDelay:       %5d\n",TxDelay);
    printf("[PARAM] host IP:       %s\n",HostAddrStr);
    printf("[PARAM] port:          %5d\n",Port);
    printf("[PARAM] PacketGenType: %5d\n",PacketGenType);
    printf("[PARAM] peer IP:       %s\n",PeerAddrStr);
    printf("--------------------------------------------\n");
    printf("\n");

    //---
    unsigned long HostAddr              = convertIpAddr(QHostAddress(HostAddrStr).toIPv4Address());
    unsigned long PeerAddr              = convertIpAddr(QHostAddress(PeerAddrStr).toIPv4Address());
    const UDP_LIB::TParams txParams     = { PacketSize, PacketsInBuf, BundleNum, WorkerThreadPriority, Timeout, SocketBufSize, PeerAddr, Port, 0/*onTransferReady*/ };
    const UDP_LIB::TDirection Direction = UDP_LIB::Transmit;

    //---
    UDP_LIB::TStatus status;
    unsigned sentBuf = 0;
    uint64_t sentBytes = 0;
    QElapsedTimer timer;

    //---
    QThread::currentThread()->setPriority(MainThreadPriority);
    TNetworkTest networkTest;

    status = UDP_LIB::createSocket(HostAddr,Port,0,&txParams);
    if(status == UDP_LIB::Ok) {
        UDP_LIB::Transfer transfer;
        TStreamTester streamGen;

        QThread::msleep(5000);
        timer.start();
        for(unsigned nBuf = 0; nBuf < BufNum; ++nBuf) {
            status = UDP_LIB::getTransfer(HostAddr, Port, Direction, transfer);
            if(!typeInfo(nBuf,status,transfer))
                continue;
            transfer.length = transfer.bufLength;
            if (PacketGenType) {
                streamGen.fillBuf(transfer.buf,transfer.length);
            }

            #if 0 // DEBUG
                static unsigned bufCntr = 0;
                if(++bufCntr <= 2*BundleNum) {
                    uint32_t* bufPtr = (uint32_t*)transfer.buf;
                    for(int k = 0; k < 8; ++k) {
                        printf("[main-2] %2d-0x%08x: buf[%2d] = %5d\n",transfer.bundleId,transfer.buf,k,bufPtr[k]);
                    }
                    printf("-------------------\n");
                }
            #endif

            status = UDP_LIB::submitTransfer(HostAddr,Port,Direction,transfer);
            if(status != UDP_LIB::Ok) {
                printf("[ERROR][MAIN] [submitTransfer] [%8d]: status: %2d\n",nBuf,status);
                break;
            }
            ++sentBuf;
            sentBytes += transfer.length;
            if(TxDelay && ((nBuf & 3) == 0)) {
                QThread::msleep(TxDelay);
            }
        }
        qint64 timeElapsed = timer.nsecsElapsed();

        //---
        printf("\n--------------------------------------------\n");
        printf("[INFO] buffers sent %20d\n",sentBuf);
        #if defined(Q_CC_GNU)
            #if defined(Q_PROCESSOR_X86_64)
                printf("[INFO] bytes sent   %20lu\n",sentBytes);
            #elif defined(Q_PROCESSOR_X86_32)
                printf("[INFO] bytes sent   %20llu\n",sentBytes);
            #else
                #warning "unknown processor type"
            #endif
        #else
            printf("[INFO] bytes sent   %20I64u\n",sentBytes);
        #endif
        printf("[INFO] elapsed time     %20.5fs\n",double(timeElapsed)/1e9);
        printf("[INFO] transfer rate:   %20.1f MB/s\n",(((double)sentBytes)/timeElapsed)*1000.0);
        printf("[INFO] transfer rate:   %20.1f Mb/s\n",(((double)sentBytes*8)/timeElapsed)*1000.0);
        printf("--------------------------------------------\n");
        printf("\n");
    } else {
        printf("[ERROR][MAIN] [createSocket] status: %d\n",status);
    }

    printf("\n--------------------------------------------\n");
    QThread::msleep(500);
    return 0;
}

//------------------------------------------------------------------------------
unsigned long convertIpAddr(unsigned long addr) {
    #if defined(UDP_LIB_ADDR_FORMAT)
        return ((addr & 0xFF000000) >> 24) | ((addr & 0x00FF0000) >> 8) | ((addr & 0x0000FF00) << 8) | ((addr & 0x000000FF) << 24);
    #else
        return addr;
    #endif
}

//------------------------------------------------------------------------------
bool typeInfo(unsigned nBuf, UDP_LIB::TStatus status, UDP_LIB::Transfer& transfer)
{
    if(nBuf == 0) {
        printf("\n--------------------------------------------\n");
    }

    if(status != UDP_LIB::Ok) {
        printf("[ERROR][MAIN] [getTransfer] [%8d]: status: %2d, trStat: %2d\n",nBuf,status,transfer.status);
        return false;
    }
    if(transfer.status != UDP_LIB::Ok) {
        printf("[WARN][MAIN] [getTransfer] [%8d]: bufId: %3d, trStat: %2d\n",nBuf,transfer.bundleId,transfer.status);
    }

    bool enaTypeInfo = ((nBuf % TypeInterval) == 0);
    if(enaTypeInfo) {
            printf("[INFO][MAIN] [getTransfer] [%8d]: bufId: %3d, dir: %1d, trStat: %2d, len: %5d, bufLen: %5d, isStream: %1d\n",
                                                  nBuf,
                                                  transfer.bundleId,
                                                  transfer.direction,
                                                  transfer.status,
                                                  transfer.length,
                                                  transfer.bufLength,
                                                  transfer.isStream
                    );
    }
    return true;
}
