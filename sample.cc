#include <cstdlib>
#include<time.h>
#include <stdio.h>
#include <string>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/error-model.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE ("WifiTopology");

void
ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, double em)
{
    uint16_t i = 0;

    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats ();

    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier ());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    {
        Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);

        std::cout << "Flow ID			: "<< stats->first << " ; " << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress << std::endl;
        std::cout << "Tx Packets = " << stats->second.txPackets << std::endl;
        std::cout << "Rx Packets = " << stats->second.rxPackets << std::endl;
        std::cout << "Duration		: "<< (stats->second.timeLastRxPacket.GetSeconds () - stats->second.timeFirstTxPacket.GetSeconds ()) << std::endl;
        std::cout << "Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds () << " Seconds" << std::endl;
        std::cout << "Throughput: " << stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds () - stats->second.timeFirstTxPacket.GetSeconds ()) / 1024 / 1024  << " Mbps" << std::endl;
    
        i++;

        std::cout << "---------------------------------------------------------------------------" << std::endl;
    }

    Simulator::Schedule (Seconds (10),&ThroughputMonitor, fmhelper, flowMon, em);
}

void
AverageDelayMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon, double em)
{
    uint16_t i = 0;

    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats ();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier ());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    {
        Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
        std::cout << "Flow ID			: "<< stats->first << " ; " << fiveTuple.sourceAddress << " -----> " << fiveTuple.destinationAddress << std::endl;
        std::cout << "Tx Packets = " << stats->second.txPackets << std::endl;
        std::cout << "Rx Packets = " << stats->second.rxPackets << std::endl;
        std::cout << "Duration		: "<< (stats->second.timeLastRxPacket.GetSeconds () - stats->second.timeFirstTxPacket.GetSeconds ()) << std::endl;
        std::cout << "Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds () << " Seconds" << std::endl;
        std::cout << "Sum of e2e Delay: " << stats->second.delaySum.GetSeconds () << " s" << std::endl;
        std::cout << "Average of e2e Delay: " << stats->second.delaySum.GetSeconds () / stats->second.rxPackets << " s" << std::endl;
    
        i++;

        std::cout << "---------------------------------------------------------------------------" << std::endl;
    }

    Simulator::Schedule (Seconds (10),&AverageDelayMonitor, fmhelper, flowMon, em);
}

class MyHeader : public Header 
{
public:
    MyHeader ();
    virtual ~MyHeader ();
    void SetData (uint16_t data);
    uint16_t GetData (void) const;
    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);
    virtual uint32_t GetSerializedSize (void) const;
private:
    uint16_t m_data;
};

MyHeader::MyHeader ()
{
}

MyHeader::~MyHeader ()
{
}

TypeId
MyHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::MyHeader")
        .SetParent<Header> ()
        .AddConstructor<MyHeader> ()
    ;
    return tid;
}

TypeId
MyHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void
MyHeader::Print (std::ostream &os) const
{
    os << "data = " << m_data << endl;
}

uint32_t
MyHeader::GetSerializedSize (void) const
{
    return 2;
}

void
MyHeader::Serialize (Buffer::Iterator start) const
{
    start.WriteHtonU16 (m_data);
}

uint32_t
MyHeader::Deserialize (Buffer::Iterator start)
{
    m_data = start.ReadNtohU16 ();

    return 2;
}

void 
MyHeader::SetData (uint16_t data)
{
    m_data = data;
}

uint16_t 
MyHeader::GetData (void) const
{
    return m_data;
}

class master : public Application
{
public:
    master (uint16_t port, Ipv4InterfaceContainer& ip);
    virtual ~master ();
private:
    virtual void StartApplication (void);
    void HandleRead (Ptr<Socket> socket);

    uint16_t port;
    Ipv4InterfaceContainer ip;
    Ptr<Socket> socket;
};


class client : public Application
{
public:
    client (uint16_t port, Ipv4InterfaceContainer& ip);
    virtual ~client ();

private:
    virtual void StartApplication (void);

    uint16_t port;
    Ptr<Socket> socket;
    Ipv4InterfaceContainer ip;
};


int
main (int argc, char *argv[])
{
    double error = 0.000001;
    string bandwidth = "1Mbps";
    bool verbose = true;
    double duration = 60.0;
    bool tracing = false;

    srand(time(NULL));

    CommandLine cmd (__FILE__);
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

    cmd.Parse (argc,argv);

    if (verbose)
    {
        LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    NodeContainer wifiStaNodeClient;
    wifiStaNodeClient.Create (1);

    NodeContainer wifiStaNodeMapper;
    wifiStaNodeMapper.Create (3);

    NodeContainer wifiStaNodeMaster;
    wifiStaNodeMaster.Create (1);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();

    YansWifiPhyHelper phy;
    phy.SetChannel (channel.Create ());

    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns-3-ssid");
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));

    NetDeviceContainer staDeviceClient;
    staDeviceClient = wifi.Install (phy, mac, wifiStaNodeClient);

    //mapper ips
    NetDeviceContainer staDeviceMapper;
    staDeviceMapper = wifi.Install (phy, mac, wifiStaNodeMapper);

    mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

    NetDeviceContainer staDeviceMaster;
    staDeviceMaster = wifi.Install (phy, mac, wifiStaNodeMaster);

    mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));

    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (error));
    phy.SetErrorRateModel("ns3::YansErrorRateModel");

    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (5.0),
                                   "DeltaY", DoubleValue (10.0),
                                   "GridWidth", UintegerValue (3),
                                   "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                               "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    mobility.Install (wifiStaNodeClient);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiStaNodeMaster);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiStaNodeMapper);

    InternetStackHelper stack;
    stack.Install (wifiStaNodeClient);
    stack.Install (wifiStaNodeMaster);
    stack.install (wifiStaNodeMapper);

    Ipv4AddressHelper address;

    Ipv4InterfaceContainer staNodeClientInterface;
    Ipv4InterfaceContainer staNodesMasterInterface;

    Ipv4InterfaceContainer staNodeMapperInterface;

    address.SetBase ("10.1.3.0", "255.255.255.0");
    staNodeClientInterface = address.Assign (staDeviceClient);
    staNodesMasterInterface = address.Assign (staDeviceMaster);

    staNodeMapperInterface = address.Assign (staDeviceMapper);


    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    uint16_t port = 1102;

    Ptr<client> clientApp = CreateObject<client> (port, staNodeClientInterface, staNodesMasterInterface);// port - client - master
    wifiStaNodeClient.Get (0)->AddApplication (clientApp);
    clientApp->SetStartTime (Seconds (0.0));
    clientApp->SetStopTime (Seconds (duration));  

    Ptr<master> masterApp = CreateObject<master> (port, staNodesMasterInterface, staNodeMapperInterface);//port - master - mapper
    wifiStaNodeMaster.Get (0)->AddApplication (masterApp);
    masterApp->SetStartTime (Seconds (0.0));
    masterApp->SetStopTime (Seconds (duration));

    Ptr<master> mapperApp1 = CreateObject<mapper> (port, staNodeMapperInterface, staNodeClientInterface, 0);//port - mapper - client - map
    wifiStaNodeMapper.Get (0)->AddApplication (mapperApp1);
    masterApp->SetStartTime (Seconds (0.0));
    masterApp->SetStopTime (Seconds (duration));  

    Ptr<master> mapperApp2 = CreateObject<mapper> (port, staNodeMapperInterface, staNodeClientInterface, 1);//port - mapper - client - map
    wifiStaNodeMapper.Get (1)->AddApplication (mapperApp2);
    masterApp->SetStartTime (Seconds (0.0));
    masterApp->SetStopTime (Seconds (duration));  

    Ptr<master> mapperApp3 = CreateObject<mapper> (port, staNodeMapperInterface, staNodeClientInterface, 2);//port - mapper - client - map
    wifiStaNodeMapper.Get (2)->AddApplication (mapperApp3);
    masterApp->SetStartTime (Seconds (0.0));
    masterApp->SetStopTime (Seconds (duration));  

    NS_LOG_INFO ("Run Simulation");

    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    ThroughputMonitor (&flowHelper, flowMonitor, error);
    AverageDelayMonitor (&flowHelper, flowMonitor, error);

    Simulator::Stop (Seconds (duration));
    Simulator::Run ();

    return 0;
}

client::client (uint16_t port, Ipv4InterfaceContainer& ipClient, Ipv4InterfaceContainer& ipMaster)// port - client - master
        : port (port),
          ipClient (ipClient),
          ipMaster (ipMaster)
{
    std::srand (time(0));
}

client::~client ()
{
}

static void GenerateTraffic (Ptr<Socket> socket, uint16_t data)
{
    Ptr<Packet> packet = new Packet();
    MyHeader m;
    m.SetData(data);

    packet->AddHeader (m);
    packet->Print (std::cout);
    socket->Send(packet);

    Simulator::Schedule (Seconds (0.1), &GenerateTraffic, socket, rand() % 26);
}

void
client::StartApplication (void)
{
    Ptr<Socket> sockMapper = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());// receiving  from mapper
    Ptr<Socket> sockMaster = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());// sending to master
    InetSocketAddress sockAddrMaster (ipMaster.GetAddress(0), port);
    InetSocketAddress sockAddrClient(ipClient.GetAddress(0), port);
    sockMaster->Connect (sockAddrMaster);
    sockMapper->Bind(sockAddrClient);

    GenerateTraffic(sockMapper, 0);
    sockMapper->SetRecvCallback (MakeCallback (&client::HandleRead, this));
}
client::HandleRead (Ptr<Socket> socket)
{
    Ptr<Packet> packet;

    while ((packet = socket->Recv ()))
    {
        if (packet->GetSize () == 0)
        {
            break;
        }

        MyHeader destinationHeader;
        packet->RemoveHeader (destinationHeader);
        destinationHeader.Print(std::cout);
    }
}



master::master (uint16_t port, Ipv4InterfaceContainer& ipMaster, Ipv4InterfaceContainer& ipMapper)//port - master - mapper
        : port (port),
          ipMaster (ipMaster),
          ipMapper (ipMapper)
{
    std::srand (time(0));
}

master::~master ()
{
}

void
master::StartApplication (void)
{
    socketMaster = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    

    InetSocketAddress local = InetSocketAddress (ipMaster.GetAddress(0), port);
    
    socketMaster->Bind (local);
    
    socketMaster->SetRecvCallback (MakeCallback (&master::HandleRead, this));
}

void 
master::HandleRead (Ptr<Socket> socket)
{
    Ptr<Packet> packet;

    socketMapper1 = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
    socketMapper2 = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
    socketMapper3 = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());

    InetSocketAddress Mapper1 = InetSocketAddress (ipMapper.GetAddress(0), port);
    InetSocketAddress Mapper2 = InetSocketAddress (ipMapper.GetAddress(1), port);
    InetSocketAddress Mapper3 = InetSocketAddress (ipMapper.GetAddress(2), port);

    while ((packet = socket->Recv ()))
    {
        if (packet->GetSize () == 0)
        {
            break;
        }

        MyHeader destinationHeader;
        packet->RemoveHeader (destinationHeader);

        Ptr<Packet> newPacket = new Packet();
        MyHeader m;
        m.SetData(destinationHeader);
        packet->AddHeader (m);

        socketMapper1->Connect(Mapper1);
        socketMapper2->Connect(Mapper2);
        socketMapper3->Connect(Mapper3);

        socketMapper1->Send(newPacket);
        socketMapper2->Send(newPacket);
        socketMapper3->Send(newPacket);
    }
}



mapper::mapper (uint16_t port, Ipv4InterfaceContainer& ipMapper, Ipv4InterfaceContainer& ipClient, int map)//port - mapper - client - map
        : port (port),
          ipMapper (ipMapper),
          ipClient (ipClient),
          map (map)
{
    std::srand (time(0));
}

mapper::~mapper ()
{
}

void
mapper::StartApplication (void)
{
    socketMaster = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());//receiving from master
    
    InetSocketAddress local = InetSocketAddress (ipMapper.GetAddress(map), port);
    socketMaster->Bind (local);
    socksocketMasteret->listen(1);
    socketMaster->SetRecvCallback (MakeCallback (&mapper::HandleRead, this));
}

void 
mapper::HandleRead (Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    char data;
    socketClient = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());//sending to client
    InetSocketAddress Client = InetSocketAddress (ipClient.GetAddress(0), port);

    while ((packet = socket->Recv ()))
    {
        if (packet->GetSize () == 0)
        {
            break;
        }

        MyHeader destinationHeader;
        packet->RemoveHeader (destinationHeader);

        if(map == 0)//0-9
        {
            switch (destinationHeader)//checking data of header?!
            {
                case 0:
                    data = 'a';
                    break;
                case 1:
                    data = 'b';
                    break;
                case 2:
                    data = 'c';
                    break;
                case 3:
                    data = 'd';
                    break;
                case 4:
                    data = 'e';
                    break;
                case 5:
                    data = 'f';
                    break;
                case 6:
                    data = 'g';
                    break;
                case 7:
                    data = 'h';
                    break;
                case 8:
                    data = 'i';
                    break;
                case 9:
                    data = 'j';
                    break;
                default:
                    return
            }

        }
        else if(map == 1)//10-20
        {
            switch (destinationHeader)
            {
                case 10:
                    data = 'k';
                    break;
                case 11:
                    data = 'l';
                    break;
                case 12:
                    data = 'm';
                    break;
                case 13:
                    data = 'n';
                    break;
                case 14:
                    data = 'o';
                    break;
                case 15:
                    data = 'p';
                    break;
                case 16:
                    data = 'q';
                    break;
                case 17:
                    data = 'r';
                    break;
                case 18:
                    data = 's';
                    break;
                case 19:
                    data = 't';
                    break;
                case 20:
                    data = 'u';
                    break;
                default:
                    return
            }
        }   
        else//21-25
        {
            switch (destinationHeader)
            {
                case 21:
                    data = 'v';
                    break;
                case 22:
                    data = 'w';
                    break;
                case 23:
                    data = 'x';
                    break;
                case 24:
                    data = 'y';
                    break;
                case 25:
                    data = 'z';
                    break;
                default:
                    return
            }
        }// data = one of a to z

        Ptr<Packet> newPacket = new Packet();
        MyHeader m;
        m.SetData(data);
        packet->AddHeader (m);

        socketClient->Connect(Client);
        socketClient->Send(newPacket);
    }
}