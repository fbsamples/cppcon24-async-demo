/*
Copyright (c) Meta Platforms, Inc. and affiliates.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <unifex/file_concepts.hpp>
#include <unifex/inplace_stop_token.hpp>
#include <unifex/io_concepts.hpp>
#include <unifex/linux/io_uring_context.hpp>
#include <unifex/on.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/span.hpp>
#include <unifex/static_thread_pool.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>
#include <unifex/then.hpp>
#include <unifex/via.hpp>
#include <unifex/when_all_range.hpp>

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <iterator>
#include <ranges>
#include <thread>
#include <utility>
#include <vector>

namespace {

namespace fs = std::filesystem;
namespace ranges = std::ranges;
namespace views = ranges::views;

struct io_uring_context {
  io_uring_context() = default;

  ~io_uring_context() {
    stopSource_.request_stop();
    t_.join();
  }

  auto get_scheduler() noexcept { return ctx_.get_scheduler(); }

private:
  unifex::inplace_stop_source stopSource_;
  unifex::linuxos::io_uring_context ctx_;
  std::thread t_{[this] { ctx_.run(stopSource_.get_token()); }};
};

unifex::sender auto async_read_some_at(auto &file, int64_t offset,
                                       auto &buffer) {
  auto outputSpan = unifex::as_writable_bytes(unifex::span(buffer));

  return unifex::async_read_some_at(file, offset, outputSpan);
}

auto open_file_read_only(auto &ioCtx, fs::path filename) {
  return unifex::open_file_read_only(ioCtx.get_scheduler(), filename);
}

struct word_stats {
  unsigned long chars{};
  unsigned long lines{};
};

unifex::task<word_stats> process_file(auto file) {
  word_stats result;

  std::array<char, 4096> buffer;
  int64_t offset = 0;
  while (std::size_t bytesRead =
             co_await async_read_some_at(file, offset, buffer)) {
    auto validBytes = unifex::span(buffer.data(), bytesRead);
    auto newlines = ranges::count(validBytes, '\n');

    result.lines += newlines;
    result.chars += (bytesRead - newlines);

    offset += bytesRead;
  }

  co_return result;
}

unifex::task<void> async_main(unifex::span<char *> args, auto &pool, auto &io) {
  auto jobs = args | views::transform([&](fs::path fileName) {
                auto file = open_file_read_only(io, fileName);
                return process_file(std::move(file));
              });

  auto stats = co_await unifex::on(
      pool.get_scheduler(), unifex::when_all_range(jobs.begin(), jobs.end()));

  for (std::size_t i = 0; i < stats.size(); ++i) {
    double mean = (double)stats[i].chars / (double)stats[i].lines;
    std::printf("Average word length in %s is %g\n", args[i], mean);
  }
}

} // namespace

int main(int argc, char **argv) {
  unifex::static_thread_pool pool;
  io_uring_context ctx;

  unifex::task<void> task = async_main({argv + 1, argc - 1}, pool, ctx);

  unifex::sync_wait(std::move(task));

  return 0;
}
