#pragma once
class ZoneWorkerThread
{

//private:
//    queue<ZoneJob*> localQueue;
//    moodycamel::ConcurrentQueue<ZoneJob*> inputQueue;
//
//    void DrainInputQueue() {
//        // 1. 현재 inputQueue의 크기만큼만 처리
//        size_t currentSize = inputQueue.size_approx();
//
//        // 2. 최대 처리 개수 제한
//        const size_t MAX_DRAIN_COUNT = 1000;  // 적절한 값으로 조정 필요
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
//        // 프레임 시작시 한번만 입력 큐에서 가져옴
//        DrainInputQueue();
//
//        // 이번 프레임에서 처리할 수 있는 최대 job 수 계산
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
//        // 처리하지 못한 job들은 글로벌 큐로
//        if (!localQueue.empty()) {
//            MoveRemainingJobsToGlobalQueue();
//        }
//    }
//
//    void MonitorQueueSize() {
//        size_t queueSize = inputQueue.size_approx();
//        if (queueSize > QUEUE_WARNING_THRESHOLD) {
//            // 로그 기록 또는 경고
//            LogWarning("Input queue size getting too large: " + std::to_string(queueSize));
//
//            // 필요시 부하 분산 정책 조정
//            AdjustLoadBalancing();
//        }
//    }
//
//    // 큐 크기에 따른 동적 처리량 조절
//    size_t GetDrainCount() {
//        size_t queueSize = inputQueue.size_approx();
//        if (queueSize > HIGH_LOAD_THRESHOLD) {
//            return MAX_DRAIN_COUNT * 2;  // 부하가 높을 때는 더 많이 처리
//        }
//        return MAX_DRAIN_COUNT;
//    }
//
//private:
//    size_t CalculateMaxJobsForFrame() {
//        // 프레임당 처리할 최대 job 수를 동적으로 계산
//        // 이전 프레임들의 job 처리 시간을 기반으로 조정
//        return some_calculated_value;
//    }


};

