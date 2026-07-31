// Behavioural regression tests for Redux's storage layer.
//
// These link the real account.cpp / file.cpp and drive them against a throwaway HOME, so they
// assert what the program actually does on disk rather than what the source looks like.
// Every case here corresponds to a bug that was shipped at some point; they exist to stop it
// coming back.
//
// POSIX only: the tests relocate the config directory by overriding HOME, which is how
// file::user_files resolves it on POSIX but not on Windows.

#include "account.h"
#include "credential.h"
#include "file.h"
#include "input.h"
#include "user.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

namespace fs = std::filesystem;

namespace {

    int failures = 0;
    int checks = 0;

    void check(bool ok, std::string const& what) {
        ++checks;
        if (!ok) {
            ++failures;
            std::cout << "  FAIL  " << what << '\n';
        } else {
            std::cout << "  ok    " << what << '\n';
        }
    }

    fs::path sandbox;

    fs::path config_dir() { return sandbox / ".config" / "Redux"; }

    void reset() {
        // Restore permissions first; a test may have made the directory read-only.
        std::error_code ec;
        fs::permissions(config_dir(), fs::perms::owner_all, ec);
        fs::remove_all(sandbox, ec);
        fs::create_directories(sandbox);
    }

    std::string raw_bytes(fs::path const& p) {
        std::ifstream in{p, std::ios::binary};
        return std::string{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    }

    void section(char const* name) { std::cout << "\n[" << name << "]\n"; }

    std::string const pw = "correct-horse-battery";

    // ---------------------------------------------------------------------

    void usernames_that_could_escape_the_config_directory_are_rejected() {
        section("username validation");

        for (auto const& bad : {
                 "",                    // empty
                 "/etc/passwd",         // absolute: operator/ would discard the base directory
                 "../../../.bashrc",    // traversal
                 "sub/alice",           // separator: parent directory would not exist
                 "sub\\alice",          // Windows separator
                 ".last_user",          // collides with Redux's own state file
                 ".",                   // current directory
                 "..",                  // parent directory
                 "alice$(whoami)",      // shell metacharacters
                 "alice name",          // space
             }) {
            check(!account::valid_username(bad),
                  std::string{"rejects \""} + bad + '"');
        }

        check(!account::valid_username(std::string(65, 'a')), "rejects a 65-character name");
        check(account::valid_username(std::string(64, 'a')), "accepts a 64-character name");

        for (auto const& good : {"alice", "Alice99", "alice_smith", "alice-smith", "alice.smith",
                                 "a"}) {
            check(account::valid_username(good), std::string{"accepts \""} + good + '"');
        }
    }

    void a_failing_write_leaves_the_previous_vault_intact() {
        section("writes are atomic and report failure");

        if (geteuid() == 0) {
            std::cout << "  skip  running as root, directory permissions would not be enforced\n";
            return;
        }

        reset();
        account::create(user{"alice", pw});

        std::vector<credential> const creds{credential{"GitHub", "u@example.com", "gh-s3cret"}};
        std::string const data = file::user_files::data("alice");
        file::credentials::write(data, creds, pw);

        std::string const before = raw_bytes(data);
        check(!before.empty(), "vault written");

        // Make the directory unwritable so the staging file cannot be created.
        fs::permissions(config_dir(), fs::perms::owner_read | fs::perms::owner_exec);

        bool threw = false;
        try {
            file::credentials::write(data, {credential{"Replacement", "x", "y"}}, pw);
        } catch (std::exception const&) {
            threw = true;
        }

        fs::permissions(config_dir(), fs::perms::owner_all);

        check(threw, "a write that cannot succeed throws instead of failing silently");
        check(raw_bytes(data) == before, "the previous vault is byte-for-byte intact");
        check(fs::exists(data) && fs::file_size(data) > 0,
              "the vault was never truncated to 0 bytes");

        auto recovered = file::credentials::read(data, pw);
        check(recovered.size() == 1 && recovered[0].password == "gh-s3cret",
              "the original credential is still readable");
    }

    void no_staging_file_is_left_behind() {
        section("no leftover staging files");

        reset();
        account::create(user{"alice", pw});
        file::credentials::write(file::user_files::data("alice"),
                                 {credential{"GitHub", "u", "s"}}, pw);

        bool leftover = false;
        for (auto const& e : fs::directory_iterator{config_dir()}) {
            if (e.path().extension() == ".new") {
                leftover = true;
            }
        }
        check(!leftover, "no '.new' staging file remains after a successful write");
    }

    void vault_files_are_not_readable_by_others() {
        section("file permissions");

        reset();
        account::create(user{"alice", pw});

        for (auto const& p : {file::user_files::account("alice"), file::user_files::data("alice")}) {
            auto perms = fs::status(p).permissions();
            check((perms & (fs::perms::group_all | fs::perms::others_all)) == fs::perms::none,
                  fs::path{p}.filename().string() + " is owner-only");
        }
    }

    void secrets_never_appear_in_cleartext_on_disk() {
        section("vault contents are ciphertext");

        reset();
        account::create(user{"alice", pw});
        std::string const data = file::user_files::data("alice");
        file::credentials::write(
            data, {credential{"GitHub", "alice@example.com", "gh-s3cret-token"}}, pw);

        auto const bytes = raw_bytes(data);
        for (auto const& secret : {"gh-s3cret-token", "alice@example.com", "GitHub", pw.c_str()}) {
            check(bytes.find(secret) == std::string::npos,
                  std::string{"vault does not contain \""} + secret + "\" in cleartext");
        }
        check(raw_bytes(file::user_files::account("alice")).find(pw) == std::string::npos,
              "account file does not contain the master password");
    }

    void a_wrong_password_does_not_lock_the_account_out() {
        section("failed attempts leave no state behind");

        reset();
        account::create(user{"alice", pw});

        check(account::valid_password(user{"alice", pw}), "correct password accepted");
        for (int i = 0; i < 5; ++i) {
            check(!account::valid_password(user{"alice", "wrong"}), "wrong password rejected");
        }
        check(account::valid_password(user{"alice", pw}),
              "correct password still accepted after repeated failures");
    }

    void changing_the_password_re_encrypts_the_vault() {
        section("change_password");

        reset();
        account::create(user{"alice", pw});
        std::string const data = file::user_files::data("alice");
        file::credentials::write(data, {credential{"GitHub", "u", "gh-s3cret"}}, pw);

        std::string const newpw = "a-brand-new-passphrase";
        account::change_password(user{"alice", pw}, newpw);

        check(!account::valid_password(user{"alice", pw}), "old password no longer works");
        check(account::valid_password(user{"alice", newpw}), "new password works");

        auto after = file::credentials::read(data, newpw);
        check(after.size() == 1 && after[0].password == "gh-s3cret",
              "vault readable under the new password, contents intact");

        bool old_fails = false;
        try {
            (void)file::credentials::read(data, pw);
        } catch (std::exception const&) {
            old_fails = true;
        }
        check(old_fails, "vault not readable under the old password");
    }

    void changing_the_password_with_no_vault_does_not_throw() {
        section("change_password with an absent vault");

        reset();
        account::create(user{"alice", pw});
        fs::remove(file::user_files::data("alice"));

        bool threw = false;
        try {
            account::change_password(user{"alice", pw}, "a-brand-new-passphrase");
        } catch (std::exception const&) {
            threw = true;
        }
        check(!threw, "an absent vault is handled rather than aborting the program");
        check(account::valid_password(user{"alice", "a-brand-new-passphrase"}),
              "the password change still took effect");
    }

    void the_remembered_username_holds_no_secret() {
        section("remembered username");

        reset();
        file::last_user::save("alice");
        check(file::last_user::load() == "alice", "remembered username round-trips");

        auto const p = config_dir() / ".last_user";
        check(raw_bytes(p) == "alice", "file contains exactly the username");
        check(raw_bytes(p).find(pw) == std::string::npos, "file contains no password");

        auto perms = fs::status(p).permissions();
        check((perms & (fs::perms::group_all | fs::perms::others_all)) == fs::perms::none,
              ".last_user is owner-only");
    }

    // Two accounts must never map onto each other's files. Both of these were real collisions:
    // "alice_data" took over "alice"'s vault under the old suffix, and "alice.new" collided with
    // the staging file used while saving "alice".
    void one_account_cannot_overwrite_another() {
        section("account file paths cannot collide");

        reset();

        // Names chosen to probe every string Redux derives from "alice": the vault suffix and the
        // staging name used while writing a file. Each of these is a name a user can register.
        std::vector<std::string> const neighbours{"alice_data", "alice.new", "alice.vault",
                                                  "alice_datavault"};
        std::string const npw = "neighbour-password";

        account::create(user{"alice", pw});
        file::credentials::write(file::user_files::data("alice"),
                                 {credential{"GitHub", "u", "alice-secret"}}, pw);

        for (auto const& n : neighbours) {
            check(account::valid_username(n), n + " is a name a user could register");
            account::create(user{n, npw});
            file::credentials::write(file::user_files::data(n),
                                     {credential{"Site", "u", n + "-secret"}}, npw);
        }

        // Every derived path must be distinct.
        std::vector<std::string> paths{file::user_files::account("alice"),
                                       file::user_files::data("alice")};
        for (auto const& n : neighbours) {
            paths.push_back(file::user_files::account(n));
            paths.push_back(file::user_files::data(n));
        }
        std::sort(paths.begin(), paths.end());
        check(std::adjacent_find(paths.begin(), paths.end()) == paths.end(),
              "no two account or vault paths are identical");

        // Rewrite alice's files now that the neighbours exist. This is the moment a colliding
        // suffix or staging name does its damage: staging is created and then renamed away, so it
        // would take a neighbour's file with it.
        account::change_password(user{"alice", pw}, pw);
        file::credentials::write(file::user_files::data("alice"),
                                 {credential{"GitHub", "u", "alice-secret"}}, pw);

        check(account::valid_password(user{"alice", pw}), "alice's account still works");
        auto alice_creds = file::credentials::read(file::user_files::data("alice"), pw);
        check(alice_creds.size() == 1 && alice_creds[0].password == "alice-secret",
              "alice's vault is intact");

        for (auto const& n : neighbours) {
            check(account::valid_password(user{n, npw}),
                  n + "'s account survived writes to alice");
            std::vector<credential> c;
            try {
                c = file::credentials::read(file::user_files::data(n), npw);
            } catch (std::exception const&) {
                // leave c empty; the check below reports it
            }
            check(c.size() == 1 && c[0].password == n + "-secret", n + "'s vault is intact");
        }
    }

    // An exhausted stdin used to make input::choice spin forever: clear() followed by ignore()
    // re-set eofbit immediately, so it printed the retry message without bound (93 MB in three
    // seconds, measured). Each of these must terminate by throwing.
    //
    // If this regresses, the functions loop instead of returning, so the test hangs rather than
    // failing -- the ctest TIMEOUT in tests/CMakeLists.txt is what turns that into a red build.
    void exhausted_stdin_terminates_instead_of_spinning() {
        section("end of input");

        auto with_empty_stdin = [](auto&& fn) {
            std::istringstream empty;
            std::ostringstream discard;
            auto* old_in = std::cin.rdbuf(empty.rdbuf());
            auto* old_out = std::cout.rdbuf(discard.rdbuf());

            bool threw = false;
            try {
                fn();
            } catch (input::end_of_input const&) {
                threw = true;
            } catch (...) {
                // any other exception counts as "did not spin", but not as correct
            }

            std::cin.rdbuf(old_in);
            std::cout.rdbuf(old_out);
            return threw;
        };

        check(with_empty_stdin([] { (void)input::choice("choice: "); }),
              "choice() throws end_of_input rather than looping");
        check(with_empty_stdin([] { (void)input::line("line: "); }),
              "line() throws end_of_input rather than returning junk");
        check(with_empty_stdin([] { (void)input::line_or("line_or: ", "fallback"); }),
              "line_or() throws end_of_input rather than silently accepting the default");
    }

    void nothing_on_disk_grants_a_session() {
        section("no credential-free session state");

        reset();
        account::create(user{"alice", pw});
        file::last_user::save("alice");

        bool found = false;
        for (auto const& e : fs::recursive_directory_iterator{sandbox}) {
            auto const name = e.path().filename().string();
            if (name == "do_not_open") {
                found = true;
            }
        }
        check(!found, "no 'do_not_open' style session file is created");
    }

} // namespace

int main() {
    sandbox = fs::temp_directory_path() / "redux-regression-tests";
    reset();
    setenv("HOME", sandbox.c_str(), 1);

    // Each case is run under its own handler so an unexpected exception is reported as a failure
    // rather than aborting the process and hiding which case broke.
    struct testcase {
        char const* name;
        void (*run)();
    };

    for (auto const& tc : {
             testcase{"username validation",
                      usernames_that_could_escape_the_config_directory_are_rejected},
             testcase{"atomic writes", a_failing_write_leaves_the_previous_vault_intact},
             testcase{"staging cleanup", no_staging_file_is_left_behind},
             testcase{"file permissions", vault_files_are_not_readable_by_others},
             testcase{"ciphertext at rest", secrets_never_appear_in_cleartext_on_disk},
             testcase{"no lockout", a_wrong_password_does_not_lock_the_account_out},
             testcase{"change_password", changing_the_password_re_encrypts_the_vault},
             testcase{"change_password, no vault", changing_the_password_with_no_vault_does_not_throw},
             testcase{"remembered username", the_remembered_username_holds_no_secret},
             testcase{"path collisions", one_account_cannot_overwrite_another},
             testcase{"end of input", exhausted_stdin_terminates_instead_of_spinning},
             testcase{"no session state", nothing_on_disk_grants_a_session},
         }) {
        try {
            tc.run();
        } catch (std::exception const& e) {
            ++checks;
            ++failures;
            std::cout << "  FAIL  " << tc.name << " threw unexpectedly: " << e.what() << '\n';
        }
    }

    std::error_code ec;
    fs::permissions(config_dir(), fs::perms::owner_all, ec);
    fs::remove_all(sandbox, ec);

    std::cout << "\n" << (checks - failures) << "/" << checks << " checks passed\n";
    if (failures != 0) {
        std::cout << failures << " FAILED\n";
        return 1;
    }
    return 0;
}
