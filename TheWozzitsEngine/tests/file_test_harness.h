#pragma once

#include <gtest/gtest.h>
#include "../api/filesystem.h"
#include "../api/wozzits_async.h"
#include <gtest/gtest.h>
#include <filesystem>

namespace fs = std::filesystem;

class FileSystemTest : public ::testing::Test
{
protected:
    fs::path test_root;

    void SetUp() override
    {
        test_root = fs::temp_directory_path() / "wozzits_fs_tests";
        fs::remove_all(test_root);
        fs::create_directories(test_root);
    }

    void TearDown() override
    {
        fs::remove_all(test_root);
    }

    std::string p(const std::string &relative)
    {
        return (test_root / relative).string();
    }
};

#include <queue>
#include <functional>

struct FakeExecutor : WZ::IAsyncExecutor
{
    std::queue<std::function<void()>> jobs;

    void post(std::function<void()> job) override
    {
        jobs.push(std::move(job));
    }

    void run_all()
    {
        while (!jobs.empty())
        {
            auto job = std::move(jobs.front());
            jobs.pop();
            job();
        }
    }
};