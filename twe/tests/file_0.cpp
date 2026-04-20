#include "file_test_harness.h"

TEST_F(FileSystemTest, ReadWriteRoundtrip)
{
    wz::fs::Buffer data = {'h', 'e', 'l', 'l', 'o'};

    auto path = p("file.txt");

    auto err = wz::fs::write_file(path, data);
    ASSERT_EQ(err, wz::Error::None);

    auto result = wz::fs::read_file(path);
    ASSERT_TRUE(result);

    EXPECT_EQ(result.value, data);
}

TEST_F(FileSystemTest, Exists)
{
    auto path = p("exists.txt");

    wz::fs::Buffer data = {'x'};
    wz::fs::write_file(path, data);

    EXPECT_TRUE(wz::fs::exists(path));
    EXPECT_FALSE(wz::fs::exists(p("missing.txt")));
}

TEST_F(FileSystemTest, OverwriteBehavior)
{
    auto path = p("file.txt");

    wz::fs::write_file(path, {'A'}, true);
    auto err = wz::fs::write_file(path, {'B'}, true);

    ASSERT_EQ(err, wz::Error::None);

    auto result = wz::fs::read_file(path);
    ASSERT_TRUE(result);

    EXPECT_EQ(result.value[0], 'B');
}

TEST_F(FileSystemTest, NoOverwriteBehavior)
{
    auto path = p("file.txt");

    wz::fs::write_file(path, {'A'}, false);
    auto err = wz::fs::write_file(path, {'B'}, false);

    EXPECT_NE(err, wz::Error::None);
}

TEST_F(FileSystemTest, FileSize)
{
    auto path = p("size.txt");

    wz::fs::Buffer data = {'a', 'b', 'c', 'd'};
    wz::fs::write_file(path, data);

    auto size = wz::fs::file_size(path);

    EXPECT_EQ(size, 4);
}

TEST_F(FileSystemTest, CreateDirectories)
{
    auto path = p("a/b/c");

    auto err = wz::fs::create_directories(path);

    EXPECT_EQ(err, wz::Error::None);
    EXPECT_TRUE(wz::fs::exists(path));
}

TEST_F(FileSystemTest, ListDirectory)
{
    auto dir = p("dir");
    wz::fs::create_directories(dir);

    wz::fs::write_file(dir + "/a.txt", {'1'});
    wz::fs::write_file(dir + "/b.txt", {'2'});

    auto result = wz::fs::list_directory(dir);

    ASSERT_TRUE(result);

    EXPECT_EQ(result.value.size(), 2);
}

TEST_F(FileSystemTest, MissingFileReturnsError)
{
    auto result = wz::fs::read_file(p("does_not_exist.txt"));

    EXPECT_FALSE(result);
    EXPECT_EQ(result.error, wz::Error::NotFound);
}

TEST_F(FileSystemTest, WriteFileText)
{
    auto path = p("text_file.txt");

    std::string text = "Hello Wozzits Engine";

    auto err = wz::fs::write_file_text(path, text, true);
    ASSERT_EQ(err, wz::Error::None);

    auto result = wz::fs::read_file(path);
    ASSERT_TRUE(result);

    std::string read_back(
        reinterpret_cast<const char *>(result.value.data()),
        result.value.size());

    EXPECT_EQ(read_back, text);
}

TEST_F(FileSystemTest, WriteFileTextOverwrite)
{
    auto path = p("text_overwrite.txt");

    wz::fs::write_file_text(path, "First", true);
    wz::fs::write_file_text(path, "Second", true);

    auto result = wz::fs::read_file(path);
    ASSERT_TRUE(result);

    std::string read_back(
        reinterpret_cast<const char *>(result.value.data()),
        result.value.size());

    EXPECT_EQ(read_back, "Second");
}

TEST_F(FileSystemTest, RemoveFile)
{
    auto path = p("to_delete.txt");

    wz::fs::Buffer data = {'x', 'y', 'z'};
    auto err = wz::fs::write_file(path, data);

    ASSERT_EQ(err, wz::Error::None);
    ASSERT_TRUE(wz::fs::exists(path));

    auto remove_err = wz::fs::remove_file(path);
    EXPECT_EQ(remove_err, wz::Error::None);

    EXPECT_FALSE(wz::fs::exists(path));
}

TEST_F(FileSystemTest, RemoveFileMissing)
{
    auto path = p("does_not_exist.txt");

    auto err = wz::fs::remove_file(path);

    EXPECT_NE(err, wz::Error::None);
}

TEST_F(FileSystemTest, RemoveDirectoryNonRecursiveEmpty)
{
    auto dir = p("empty_dir");
    auto err = wz::fs::create_directories(dir);

    ASSERT_EQ(err, wz::Error::None);

    auto remove_err = wz::fs::remove_directory(dir, false);

    EXPECT_EQ(remove_err, wz::Error::None);
    EXPECT_FALSE(wz::fs::exists(dir));
}

TEST_F(FileSystemTest, RemoveDirectoryNonRecursiveNotEmptyFails)
{
    auto dir = p("not_empty_dir");
    wz::fs::create_directories(dir);

    wz::fs::write_file(dir + "/file.txt", {'a'});

    auto err = wz::fs::remove_directory(dir, false);

    EXPECT_NE(err, wz::Error::None);

    EXPECT_TRUE(wz::fs::exists(dir));
}

TEST_F(FileSystemTest, RemoveDirectoryRecursive)
{
    auto dir = p("tree");

    wz::fs::create_directories(dir + "/a/b");

    wz::fs::write_file(dir + "/a/file1.txt", {'1'});
    wz::fs::write_file(dir + "/a/b/file2.txt", {'2'});

    auto err = wz::fs::remove_directory(dir, true);

    EXPECT_EQ(err, wz::Error::None);
    EXPECT_FALSE(wz::fs::exists(dir));
}

TEST_F(FileSystemTest, RemoveDirectoryMissing)
{
    auto dir = p("does_not_exist");

    auto err = wz::fs::remove_directory(dir, true);

    EXPECT_NE(err, wz::Error::None);
}

TEST_F(FileSystemTest, AsyncReadFile)
{
    FakeExecutor executor;
    wz::set_async_executor(&executor);

    auto path = p("async.txt");

    wz::fs::write_file(path, {'h', 'i'});

    bool called = false;

    wz::fs::async_read_file(
        path,
        [&](auto result)
        {
            ASSERT_TRUE(result);
            EXPECT_EQ(result.value[0], 'h');
            called = true;
        });

    executor.run_all();

    EXPECT_TRUE(called);
}

TEST_F(FileSystemTest, AsyncWriteFile)
{
    FakeExecutor executor;
    wz::set_async_executor(&executor);

    auto path = p("async_write.txt");

    bool called = false;

    wz::fs::async_write_file(
        path,
        {'x', 'y', 'z'},
        [&](wz::Error err)
        {
            EXPECT_EQ(err, wz::Error::None);
            called = true;
        },
        true);

    executor.run_all();

    EXPECT_TRUE(wz::fs::exists(path));
    EXPECT_TRUE(called);
}

TEST_F(FileSystemTest, NormalizeBasicSeparators)
{
    auto result = wz::fs::normalize("C:/foo//bar\\baz/");

    EXPECT_EQ(result, "C:\\foo\\bar\\baz");
}

TEST_F(FileSystemTest, NormalizeCollapsesRepeatedSeparators)
{
    auto result = wz::fs::normalize("C:\\\\foo\\\\\\bar");

    EXPECT_EQ(result, "C:\\foo\\bar");
}

TEST_F(FileSystemTest, NormalizePreservesRoot)
{
    auto result = wz::fs::normalize("C:\\");

    EXPECT_EQ(result, "C:\\");
}

TEST_F(FileSystemTest, NormalizeEmptyPath)
{
    auto result = wz::fs::normalize("");

    EXPECT_EQ(result, "");
}

TEST_F(FileSystemTest, JoinBasic)
{
    auto result = wz::fs::join("C:\\foo", "bar");

    EXPECT_EQ(result, "C:\\foo\\bar");
}

TEST_F(FileSystemTest, JoinAddsSeparator)
{
    auto result = wz::fs::join("C:\\foo", "bar");

    EXPECT_EQ(result, "C:\\foo\\bar");
}

TEST_F(FileSystemTest, JoinAvoidsDuplicateSeparators)
{
    auto result = wz::fs::join("C:\\foo\\", "\\bar");

    EXPECT_EQ(result, "C:\\foo\\bar");
}

TEST_F(FileSystemTest, JoinLeftEmpty)
{
    auto result = wz::fs::join("", "bar");

    EXPECT_EQ(result, "bar");
}

TEST_F(FileSystemTest, JoinRightEmpty)
{
    auto result = wz::fs::join("C:\\foo", "");

    EXPECT_EQ(result, "C:\\foo");
}

TEST_F(FileSystemTest, JoinMixedSeparators)
{
    auto result = wz::fs::join("C:/foo", "/bar");

    EXPECT_EQ(result, "C:/foo/bar");
}

TEST_F(FileSystemTest, ParentPathBasic)
{
    auto result = wz::fs::parent_path("C:\\foo\\bar");

    EXPECT_EQ(result, "C:\\foo");
}

TEST_F(FileSystemTest, ParentPathSingleLevel)
{
    auto result = wz::fs::parent_path("C:\\foo");

    EXPECT_EQ(result, "C:\\");
}

TEST_F(FileSystemTest, ParentPathNoSeparator)
{
    auto result = wz::fs::parent_path("foo");

    EXPECT_EQ(result, "");
}

TEST_F(FileSystemTest, ParentPathDeep)
{
    auto result = wz::fs::parent_path("a\\b\\c\\d");

    EXPECT_EQ(result, "a\\b\\c");
}

TEST_F(FileSystemTest, ParentPathRoot)
{
    auto result = wz::fs::parent_path("C:\\");

    EXPECT_EQ(result, "C:\\");
}

TEST_F(FileSystemTest, ParentPathMixedSeparators)
{
    auto result = wz::fs::parent_path("C:/foo/bar");

    EXPECT_EQ(result, "C:/foo");
}

TEST_F(FileSystemTest, FilenameBasic)
{
    EXPECT_EQ(wz::fs::filename("C:\\foo\\bar.txt"), "bar.txt");
}

TEST_F(FileSystemTest, FilenameNoPath)
{
    EXPECT_EQ(wz::fs::filename("bar.txt"), "bar.txt");
}

TEST_F(FileSystemTest, FilenameTrailingSeparator)
{
    EXPECT_EQ(wz::fs::filename("C:\\foo\\bar\\"), "");
}

TEST_F(FileSystemTest, FilenameMixedSeparators)
{
    EXPECT_EQ(wz::fs::filename("C:/foo\\bar.txt"), "bar.txt");
}

TEST_F(FileSystemTest, StemBasic)
{
    EXPECT_EQ(wz::fs::stem("C:\\foo\\bar.txt"), "bar");
}

TEST_F(FileSystemTest, StemMultipleDots)
{
    EXPECT_EQ(wz::fs::stem("archive.tar.gz"), "archive.tar");
}

TEST_F(FileSystemTest, StemNoExtension)
{
    EXPECT_EQ(wz::fs::stem("bar"), "bar");
}

TEST_F(FileSystemTest, StemHiddenFile)
{
    EXPECT_EQ(wz::fs::stem(".gitignore"), ".gitignore");
}

TEST_F(FileSystemTest, ExtensionBasic)
{
    EXPECT_EQ(wz::fs::extension("C:\\foo\\bar.txt"), "txt");
}

TEST_F(FileSystemTest, ExtensionMultipleDots)
{
    EXPECT_EQ(wz::fs::extension("archive.tar.gz"), "gz");
}

TEST_F(FileSystemTest, ExtensionNone)
{
    EXPECT_EQ(wz::fs::extension("bar"), "");
}

TEST_F(FileSystemTest, ExtensionHiddenFile)
{
    EXPECT_EQ(wz::fs::extension(".gitignore"), "");
}

TEST_F(FileSystemTest, IsAbsoluteDrivePath)
{
    EXPECT_TRUE(wz::fs::is_absolute("C:\\foo"));
}

TEST_F(FileSystemTest, IsAbsoluteUNCPath)
{
    EXPECT_TRUE(wz::fs::is_absolute("\\\\server\\share"));
}

TEST_F(FileSystemTest, IsRelativePath)
{
    EXPECT_FALSE(wz::fs::is_absolute("foo\\bar"));
}

TEST_F(FileSystemTest, CurrentDirectory)
{
    auto cwd = wz::fs::current_directory();

    EXPECT_FALSE(cwd.empty());
    EXPECT_TRUE(wz::fs::is_absolute(cwd));
}

TEST_F(FileSystemTest, SetCurrentDirectory)
{
    auto original = wz::fs::current_directory();

    auto temp_dir = p("cwd_test");
    wz::fs::create_directories(temp_dir);

    auto err = wz::fs::set_current_directory(temp_dir);
    EXPECT_EQ(err, wz::Error::None);

    auto cwd = wz::fs::current_directory();
    EXPECT_NE(cwd, original);

    // restore
    wz::fs::set_current_directory(original);
}

TEST_F(FileSystemTest, SetCurrentDirectoryInvalid)
{
    auto err = wz::fs::set_current_directory("C:\\this_path_should_not_exist_123456");

    EXPECT_NE(err, wz::Error::None);
}

TEST_F(FileSystemTest, ExecutablePath)
{
    auto exe = wz::fs::executable_path();

    EXPECT_FALSE(exe.empty());
    EXPECT_TRUE(wz::fs::is_absolute(exe));
}