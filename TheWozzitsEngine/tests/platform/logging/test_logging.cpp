#include <gtest/gtest.h>
#include <algorithm>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

#include "logging.h"

namespace
{

    // Test buffer to capture log output
    std::vector<std::string> g_test_log_output;
    std::mutex g_test_log_mutex;

    // Test callback that stores logs in our buffer
    void test_log_callback(LogLevel level, const char *message)
    {
        std::lock_guard<std::mutex> lock(g_test_log_mutex);
        g_test_log_output.emplace_back(message);
    }

    // Helper to clear test output
    void clear_test_output()
    {
        std::lock_guard<std::mutex> lock(g_test_log_mutex);
        g_test_log_output.clear();
    }

    // Helper to get sorted output for comparison
    std::vector<std::string> get_sorted_output()
    {
        std::lock_guard<std::mutex> lock(g_test_log_mutex);
        std::vector<std::string> result = g_test_log_output;
        std::sort(result.begin(), result.end());
        return result;
    }

    // Helper to count occurrences of a substring in output
    int count_occurrences(const std::string &substring)
    {
        std::lock_guard<std::mutex> lock(g_test_log_mutex);
        int count = 0;
        for (const auto &line : g_test_log_output)
        {
            if (line.find(substring) != std::string::npos)
            {
                count++;
            }
        }
        return count;
    }

    // Helper to wait for all threads to complete and flush their logs
    void wait_and_flush_threads(std::vector<std::thread> &threads)
    {
        // Wait for all threads to complete
        for (auto &thread : threads)
        {
            thread.join();
        }
        
        // Small delay to ensure thread-local destructors have run
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

} // namespace

// TEST 1: Single-thread baseline
TEST(LoggingTest, SingleThreadBaseline)
{
    // Set up test callback
    set_log_callback(test_log_callback);
    clear_test_output();

    // Log several messages
    log_info("A");
    log_info("B");
    log_info("C");

    // Flush to ensure output is written
    flush_thread_local_logs();

    // Verify all messages appear
    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), 3);

    // Check each message appears exactly once
    EXPECT_EQ(count_occurrences("A"), 1);
    EXPECT_EQ(count_occurrences("B"), 1);
    EXPECT_EQ(count_occurrences("C"), 1);
}

// TEST 2: Multi-thread basic safety
TEST(LoggingTest, MultiThreadBasicSafety)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    const int num_threads = 8;
    const int messages_per_thread = 100;

    std::vector<std::thread> threads;

    // Create threads
    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([t, messages_per_thread]()
                             {
                                 for (int i = 0; i < messages_per_thread; ++i)
                                 {
                                     std::string msg = "thread=" + std::to_string(t) + " msg=" + std::to_string(i);
                                     log_info(msg.c_str());
                                 } });
    }

    // Wait for all threads to complete (destructors will flush)
    wait_and_flush_threads(threads);

    // Verify total message count
    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), num_threads * messages_per_thread);

    // Verify each expected message appears exactly once
    for (int t = 0; t < num_threads; ++t)
    {
        for (int i = 0; i < messages_per_thread; ++i)
        {
            std::string expected = "thread=" + std::to_string(t) + " msg=" + std::to_string(i);
            EXPECT_EQ(count_occurrences(expected), 1) << "Missing or duplicate: " << expected;
        }
    }
}

// TEST 3: Message atomicity
TEST(LoggingTest, MessageAtomicity)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    const int num_threads = 16;
    const int messages_per_thread = 200;

    std::vector<std::thread> threads;

    // Create threads
    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([t, messages_per_thread]()
                             {
                                 for (int i = 0; i < messages_per_thread; ++i)
                                 {
                                     std::string msg = "[THREAD " + std::to_string(t) + "] BEGIN " + std::to_string(i) + " END";
                                     log_info(msg.c_str());
                                 } });
    }

    // Wait for all threads to complete
    wait_and_flush_threads(threads);

    // Verify all lines match expected pattern
    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), num_threads * messages_per_thread);

    // Check each line matches the pattern
    for (const auto &line : output)
    {
        // Verify line starts with "[THREAD " and ends with " END"
        EXPECT_EQ(line.substr(0, 8), "[THREAD ");
        EXPECT_EQ(line.substr(line.size() - 4), " END");

        // Verify it contains "BEGIN" and "END"
        EXPECT_NE(line.find("BEGIN"), std::string::npos);
        EXPECT_NE(line.find("END"), std::string::npos);
    }
}

// TEST 4: High contention stress
TEST(LoggingTest, HighContentionStress)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    const int num_threads = 32;
    const int messages_per_thread = 500;

    std::vector<std::thread> threads;

    // Create threads
    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([t, messages_per_thread]()
                             {
                                 for (int i = 0; i < messages_per_thread; ++i)
                                 {
                                     std::string msg = "T" + std::to_string(t) + "-M" + std::to_string(i);
                                     log_info(msg.c_str());
                                 } });
    }

    // Wait for all threads to complete
    wait_and_flush_threads(threads);

    // Verify total message count
    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), num_threads * messages_per_thread);

    // Verify no corrupted lines (each line should match T#-M# pattern)
    for (const auto &line : output)
    {
        // Check if line matches pattern T\d+-M\d+
        bool valid = line.size() >= 6 && line[0] == 'T' && line.find("-M") != std::string::npos;
        EXPECT_TRUE(valid) << "Corrupted line: " << line;
    }
}

// TEST 5: Large message safety
TEST(LoggingTest, LargeMessageSafety)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    const int num_threads = 8;
    const int messages_per_thread = 50;

    std::vector<std::thread> threads;

    // Create threads
    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([t, messages_per_thread]()
                             {
                                 for (int i = 0; i < messages_per_thread; ++i)
                                 {
                                     // Generate large message (4KB-16KB)
                                     std::string large_data(4096 + (i % 12) * 1024, 'X');
                                     std::string msg = "T" + std::to_string(t) + "-M" + std::to_string(i) + ": " + large_data;
                                     log_info(msg.c_str());
                                 } });
    }

    // Wait for all threads to complete
    wait_and_flush_threads(threads);

    // Verify total message count
    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), num_threads * messages_per_thread);

    // Verify message integrity
    for (const auto &line : output)
    {
        // Check that line starts with expected prefix
        EXPECT_EQ(line.substr(0, 2), "T");
        EXPECT_NE(line.find("-M"), std::string::npos);

        // Check that the large data portion is intact (contains X's)
        EXPECT_NE(line.find("XXXX"), std::string::npos);
    }
}

// TEST 6: Log level filtering
TEST(LoggingTest, LogLevelFiltering)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    // Set minimum level to Warning
    set_min_log_level(LogLevel::Warning);

    log_debug("debug message");
    log_info("info message");
    log_warning("warning message");
    log_error("error message");
    log_critical("critical message");

    flush_thread_local_logs();

    auto output = get_sorted_output();
    
    // Only Warning, Error, and Critical should appear
    EXPECT_EQ(output.size(), 3);
    EXPECT_EQ(count_occurrences("debug message"), 0);
    EXPECT_EQ(count_occurrences("info message"), 0);
    EXPECT_EQ(count_occurrences("warning message"), 1);
    EXPECT_EQ(count_occurrences("error message"), 1);
    EXPECT_EQ(count_occurrences("critical message"), 1);
}

// TEST 7: Log levels work correctly
TEST(LoggingTest, LogLevels)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    // Reset to Debug level
    set_min_log_level(LogLevel::Debug);

    log_debug("debug message");
    log_info("info message");
    log_warning("warning message");
    log_error("error message");
    log_critical("critical message");

    flush_thread_local_logs();

    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), 5);

    EXPECT_EQ(count_occurrences("debug message"), 1);
    EXPECT_EQ(count_occurrences("info message"), 1);
    EXPECT_EQ(count_occurrences("warning message"), 1);
    EXPECT_EQ(count_occurrences("error message"), 1);
    EXPECT_EQ(count_occurrences("critical message"), 1);
}

// TEST 8: Empty message handling
TEST(LoggingTest, EmptyMessage)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    log_info("");

    flush_thread_local_logs();

    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), 1);
    EXPECT_EQ(output[0], "");
}

// TEST 9: Null message handling
TEST(LoggingTest, NullMessage)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    log_info(nullptr);

    flush_thread_local_logs();

    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), 1);
    EXPECT_EQ(output[0], "");
}

// TEST 10: Thread auto-flush on exit
TEST(LoggingTest, ThreadAutoFlushOnExit)
{
    set_log_callback(test_log_callback);
    clear_test_output();

    std::thread t([]()
                  {
                      log_info("message from thread");
                      // Thread exits without explicit flush
                  });

    t.join();
    
    // Small delay to ensure destructor has run
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Message should still appear due to destructor flush
    auto output = get_sorted_output();
    EXPECT_EQ(output.size(), 1);
    EXPECT_EQ(count_occurrences("message from thread"), 1);
}

// TEST 11: Callback change during logging
TEST(LoggingTest, CallbackChangeDuringLogging)
{
    std::vector<std::string> g_test_log_output_2;
    std::mutex g_test_log_mutex_2;

    auto test_callback_2 = [&g_test_log_output_2, &g_test_log_mutex_2](LogLevel level, const char *message)
    {
        std::lock_guard<std::mutex> lock(g_test_log_mutex_2);
        g_test_log_output_2.emplace_back(message);
    };

    set_log_callback(test_log_callback);
    clear_test_output();

    log_info("before callback change");
    
    // Change callback
    set_log_callback(test_callback_2);
    
    log_info("after callback change");

    flush_thread_local_logs();

    // First message should be in original callback
    EXPECT_EQ(count_occurrences("before callback change"), 1);
    
    // Second message should be in new callback
    {
        std::lock_guard<std::mutex> lock(g_test_log_mutex_2);
        EXPECT_EQ(g_test_log_output_2.size(), 1);
        EXPECT_EQ(g_test_log_output_2[0], "after callback change");
    }
}
