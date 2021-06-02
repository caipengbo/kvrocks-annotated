#include "task_runner.h"

#include <thread>
#include "util.h"

// 注册任务
Status TaskRunner::Publish(Task task) {
  mu_.lock();
  if (stop_) {
    mu_.unlock();
    return Status(Status::NotOK, "the runner was stopped");
  }
  if (task_queue_.size() >= max_queue_size_) {
    mu_.unlock();
    return Status(Status::NotOK, "the task queue was reached max length");
  }
  task_queue_.emplace_back(task);
  // notify 条件变量
  cond_.notify_all();
  mu_.unlock();
  return Status::OK();
}

void TaskRunner::Start() {
  stop_ = false;
  // 这个线程池，每次都会创建thread, 没有常驻线程(核心线程)
  for (int i = 0; i < n_thread_; i++) {
    threads_.emplace_back(std::thread([this]() {
      Util::ThreadSetName("task-runner");
      this->run();
    }));
  }
}

void TaskRunner::Stop() {
  mu_.lock();
  stop_ = true;
  cond_.notify_all();
  mu_.unlock();
}

void TaskRunner::Join() {
  for (size_t i = 0; i < threads_.size(); i++) {
    if (threads_[i].joinable()) threads_[i].join();
  }
}

void TaskRunner::Purge() {
  mu_.lock();
  threads_.clear();
  task_queue_.clear();
  mu_.unlock();
}

void TaskRunner::run() {
  Task task;
  std::unique_lock<std::mutex> lock(mu_);
  // 如果不停止会一直循环
  while (!stop_) {
    // 条件变量，直到有 Task 被 Publish
    cond_.wait(lock, [this]() -> bool { return stop_ || !task_queue_.empty();});
    while (!stop_ && !task_queue_.empty()) {
      // 执行队列中的任务
      task = task_queue_.front();
      task_queue_.pop_front();
      lock.unlock();
      if (task.callback) task.callback(task.arg);
      lock.lock();
    }
  }
  task_queue_.clear();
  lock.unlock();
  // CAUTION: drop the rest of tasks, don't use task runner if the task can't be drop
}
