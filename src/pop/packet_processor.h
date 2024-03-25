#pragma once
#include <thread>
#include <vector>
#include <atomic>

#include "packet_structures.h"
#include "worker.h"

class PacketProcessor
{
private:
    ThreadSafeQueue<std::shared_ptr<packet>> sendQueue;
    ThreadSafeQueue<std::shared_ptr<packet>> recvQueue;
    std::vector<std::thread> workerThreads;
    std::atomic<bool> stopFlag{false};

    void processSendQueue()
    {
        while (!stopFlag)
        {
            std::shared_ptr<packet> pkt;
            sendQueue.wait_and_pop(pkt);
            intercept_manager::on_packet_send(pkt.get());
        }
    }

    void processRecvQueue()
    {
        while (!stopFlag)
        {
            std::shared_ptr<packet> pkt;
            recvQueue.wait_and_pop(pkt);
            intercept_manager::on_packet_recv(pkt.get());
        }
    }

public:
    PacketProcessor()
    {
        workerThreads.emplace_back(&PacketProcessor::processSendQueue, this);
        workerThreads.emplace_back(&PacketProcessor::processRecvQueue, this);
    }

    ~PacketProcessor()
    {
        stopFlag = true;
        for (auto &thread : workerThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    void enqueueSend(std::shared_ptr<packet> pkt)
    {
        sendQueue.push(std::move(pkt));
    }

    void enqueueRecv(std::shared_ptr<packet> pkt)
    {
        recvQueue.push(std::move(pkt));
    }
};
