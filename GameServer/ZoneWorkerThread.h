#pragma once
class ZoneWorkerThread
{

//private:
//    queue<ZoneJob*> localQueue;
//    moodycamel::ConcurrentQueue<ZoneJob*> inputQueue;
//
//    void DrainInputQueue() {
//        // 1. ���� inputQueue�� ũ�⸸ŭ�� ó��
//        size_t currentSize = inputQueue.size_approx();
//
//        // 2. �ִ� ó�� ���� ����
//        const size_t MAX_DRAIN_COUNT = 1000;  // ������ ������ ���� �ʿ�
//        currentSize = std::min(currentSize, MAX_DRAIN_COUNT);
//
//        ZoneJob* job;
//        for (size_t i = 0; i < currentSize; ++i) {
//            if (inputQueue.try_dequeue(job)) {
//                localQueue.push(job);
//            }
//        }
//    }
//
//    void ProcessFrame() {
//        frameStart = chrono::steady_clock::now();
//
//        // ������ ���۽� �ѹ��� �Է� ť���� ������
//        DrainInputQueue();
//
//        // �̹� �����ӿ��� ó���� �� �ִ� �ִ� job �� ���
//        const size_t MAX_JOBS_PER_FRAME = CalculateMaxJobsForFrame();
//        size_t processedJobs = 0;
//
//        while (!localQueue.empty() && !IsOverloaded()
//            && processedJobs < MAX_JOBS_PER_FRAME) {
//            auto* job = localQueue.front();
//            localQueue.pop();
//            job->Process();
//            processedJobs++;
//        }
//
//        // ó������ ���� job���� �۷ι� ť��
//        if (!localQueue.empty()) {
//            MoveRemainingJobsToGlobalQueue();
//        }
//    }
//
//    void MonitorQueueSize() {
//        size_t queueSize = inputQueue.size_approx();
//        if (queueSize > QUEUE_WARNING_THRESHOLD) {
//            // �α� ��� �Ǵ� ���
//            LogWarning("Input queue size getting too large: " + std::to_string(queueSize));
//
//            // �ʿ�� ���� �л� ��å ����
//            AdjustLoadBalancing();
//        }
//    }
//
//    // ť ũ�⿡ ���� ���� ó���� ����
//    size_t GetDrainCount() {
//        size_t queueSize = inputQueue.size_approx();
//        if (queueSize > HIGH_LOAD_THRESHOLD) {
//            return MAX_DRAIN_COUNT * 2;  // ���ϰ� ���� ���� �� ���� ó��
//        }
//        return MAX_DRAIN_COUNT;
//    }
//
//private:
//    size_t CalculateMaxJobsForFrame() {
//        // �����Ӵ� ó���� �ִ� job ���� �������� ���
//        // ���� �����ӵ��� job ó�� �ð��� ������� ����
//        return some_calculated_value;
//    }


};

