#pragma once

#include <memory>
#include <vector>

#include "base/node.hpp"
#include "worker/worker_helper_thread.hpp"
#include "worker/app_blocker.hpp"
#include "worker/simple_range_manager.hpp"
#include "worker/model_init_thread.hpp"
#include "server/server_thread.hpp"
#include "server/server_thread_group.hpp"
#include "comm/mailbox.hpp"
#include "comm/sender.hpp"
#include "driver/ml_task.hpp"
#include "driver/simple_id_mapper.hpp"
#include "driver/info.hpp"
#include "driver/worker_spec.hpp"

namespace flexps {

class Engine {
 public:
  Engine(const Node& node, const std::vector<Node>& nodes) : node_(node), nodes_(nodes) {}
  /*
   * The flow of starting the engine:
   * 1. Create a mailbox
   * 2. Start Sender
   * 3. Create ServerThreads, WorkerHelperThreads and ModelInitThread
   * 4. Register the threads to mailbox through ThreadsafeQueue
   * 5. Start the mailbox: bind and connect to all other nodes
   */
  void StartEverything();
  void CreateMailbox();
  void StartServerThreads();
  void StartWorkerHelperThreads();
  void StartModelInitThread();
  void StartMailbox();
  void StartSender();

  /*
   * The flow of stopping the engine:
   * 1. Stop the Sender
   * 2. Stop the mailbox: by Barrier() and then exit
   * 3. The mailbox will stop the corresponding registered threads
   * 4. Stop the ServerThreads, WorkerHelperThreads and ModelInitThread
   */
  void StopEverything();
  void StopServerThreads();
  void StopWorkerHelperThreads();
  void StopModelInitThread();
  void StopSender();
  void StopMailbox();
 
  void Barrier();
  WorkerSpec AllocateWorkers(const std::vector<WorkerAlloc>& worker_alloc);
  void CreateTable(uint32_t table_id, const std::vector<third_party::Range>& ranges);
  void InitTable(uint32_t table_id, const std::vector<uint32_t>& worker_ids);
  void Run(const MLTask& task);
 private:
  std::map<uint32_t, SimpleRangeManager> range_manager_map_;
  // nodes
  Node node_;
  std::vector<Node> nodes_;
  // mailbox
  std::unique_ptr<SimpleIdMapper> id_mapper_;
  std::unique_ptr<Mailbox> mailbox_;
  std::unique_ptr<Sender> sender_;
  // worker elements
  std::unique_ptr<AppBlocker> app_blocker_;
  std::unique_ptr<WorkerHelperThread> worker_helper_thread_;
  std::unique_ptr<ModelInitThread> model_init_thread_;
  // server elements
  std::unique_ptr<ServerThreadGroup> server_thread_group_;
};

}  // namespace flexps