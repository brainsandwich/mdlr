#include <fmt/format.h>
#include <string_view>

namespace tc
{
    static const char* shorten_filepath(const char* fp)
    {
        static constexpr const char basepath[] = "/Users/alexandrebeaudet/Documents/Personal/Development/audio-synths";
        std::string_view sv(fp);
        auto p = sv.find(basepath);
        if (p == std::string::npos)
            return fp;
        return sv.substr(sizeof(basepath) - 1).data();
    }

    enum class test_result: int
    {
        success = 0,
        failure = 1,
        requirement_failure = 2
    };
    
    test_result operator|=(const test_result& left, const test_result& right) { return (test_result) std::max((int) left, (int) right); }

    struct session
    {
        int exitcode = 0;
        int total = 0;
        int passed = 0;
        int failed = 0;

        void print_summary() const
        {
            fmt::println("[{}/{}] tests passed", passed, total);
        }

        void update(test_result result)
        {
            if (result == test_result::success)
                passed++;
            else
            {
                failed++;
                exitcode = -1;
            }
            total++;
        }
    };
}

#define CHECK(...)                                                  \
    {                                                               \
        bool expr_result = (__VA_ARGS__);                           \
        fmt::print("[{}] in {}:{} -- Check \"" #__VA_ARGS__ "\""    \
            , __test_session.total                                  \
            , tc::shorten_filepath(__FILE__)                        \
            , __LINE__);                                            \
                                                                    \
        tc::test_result result = expr_result                        \
            ? tc::test_result::success                              \
            : tc::test_result::failure;                             \
        __test_session.update(result);                              \
        __test_result |= result;                                    \
                                                                    \
        if (!expr_result)                                           \
        {                                                           \
            fmt::println(" -> Failed");                             \
        } else {                                                    \
            fmt::println(" -> Passed");                             \
        }                                                           \
    }

#define REQUIRE(...)                                                \
    {                                                               \
        bool expr_result = (__VA_ARGS__);                           \
        fmt::print("[{}] in {}:{} -- Require \"" #__VA_ARGS__ "\""  \
            , __test_session.total                                  \
            , tc::shorten_filepath(__FILE__)                        \
            , __LINE__);                                            \
                                                                    \
        tc::test_result result = expr_result                        \
            ? tc::test_result::success                              \
            : tc::test_result::requirement_failure;                 \
        __test_session.update(result);                              \
        __test_result |= result;                                    \
                                                                    \
        if (!expr_result)                                           \
        {                                                           \
            fmt::println(" -> Failed");                             \
            return;                                                 \
        } else {                                                    \
            fmt::println(" -> Passed");                             \
        }                                                           \
    }

#define TEST_CASE(_name) \
    void test_##_name(tc::session& __test_session, tc::test_result& __test_result)

#define RUN_TEST(_func)                                             \
    tc::test_result __test_result = tc::test_result::success;       \
    _func(__test_session, __test_result);                           \
    switch (__test_result)                                          \
    {                                                               \
        case tc::test_result::success:                              \
            break;                                                  \
        case tc::test_result::failure:                              \
        case tc::test_result::requirement_failure:                  \
            __test_session.exitcode = -1;                           \
            break;                                                  \
    }

#define TEST_ENTRY(...)                                             \
    int main(int argc, char** argv)                                 \
    {                                                               \
        tc::session __test_session;                                 \
                                                                    \
        __VA_ARGS__                                                 \
                                                                    \
        __test_session.print_summary();                             \
        return __test_session.exitcode;                             \
    }



TEST_CASE(simple)
{
    double value = 127.0;
    CHECK(true == false);
    REQUIRE(value == 123.0);
}

TEST_ENTRY({
    RUN_TEST(test_simple);
})