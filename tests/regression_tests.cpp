// Behavioural regression tests for Redux's storage layer.
//
// These link the real account.cpp / file.cpp and drive them against a throwaway config
// directory, so they assert what the program actually does on disk rather than what the source
// looks like. Every case here corresponds to a bug that was shipped at some point; they exist to
// stop it coming back.
//
// The config directory is relocated via REDUX_CONFIG_DIR, which file::vault and friends check on
// every platform (unlike overriding HOME, which does nothing on Windows).

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
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace {

    int failures = 0;
    int checks = 0;

    // setenv/unsetenv are POSIX; _putenv_s is the equivalent on Windows, where an empty value
    // unsets the variable rather than setting it to an empty string.
#ifdef _WIN32
    void set_env(char const* name, std::string const& value) { _putenv_s(name, value.c_str()); }
    // maybe_unused: its only caller, the fail-closed config-dir test, is POSIX-only.
    [[maybe_unused]] void unset_env(char const* name) { _putenv_s(name, ""); }
#else
    void set_env(char const* name, std::string const& value) { setenv(name, value.c_str(), 1); }
    void unset_env(char const* name) { unsetenv(name); }
#endif

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

    // REDUX_CONFIG_DIR names the config directory directly -- unlike HOME, there is no
    // ".config/Redux" appended underneath it.
    fs::path config_dir() { return sandbox; }

    void reset() {
        // Restore permissions first; a test may have made the directory read-only.
        std::error_code ec;
        fs::permissions(sandbox, fs::perms::owner_all, ec);
        fs::remove_all(sandbox, ec);
        fs::create_directories(sandbox);
    }

    std::string raw_bytes(fs::path const& p) {
        std::ifstream in{p, std::ios::binary};
        return std::string{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    }

    void flip_byte_at(fs::path const& p, std::streamoff offset) {
        std::fstream f{p, std::ios::binary | std::ios::in | std::ios::out};
        f.seekg(offset);
        char c = 0;
        f.read(&c, 1);
        c = static_cast<char>(c ^ 0xFF);
        f.seekp(offset);
        f.write(&c, 1);
    }

    void set_byte_at(fs::path const& p, std::streamoff offset, unsigned char value) {
        std::fstream f{p, std::ios::binary | std::ios::in | std::ios::out};
        f.seekp(offset);
        char const c = static_cast<char>(value);
        f.write(&c, 1);
    }

    void section(char const* name) { std::cout << "\n[" << name << "]\n"; }

    std::string const pw = "correct-horse-battery";

    // ---------------------------------------------------------------------

    void usernames_that_could_escape_the_config_directory_are_rejected() {
        section("username validation");

        for (auto const& bad : {
                 "",                 // empty
                 "/etc/passwd",      // absolute: operator/ would discard the base directory
                 "../../../.bashrc", // traversal
                 "sub/alice",        // separator: parent directory would not exist
                 "sub\\alice",       // Windows separator
                 ".last_user",       // collides with Redux's own state file
                 ".",                // current directory
                 "..",               // parent directory
                 "alice$(whoami)",   // shell metacharacters
                 "alice name",       // space
             }) {
            check(!account::valid_username(bad), std::string{"rejects \""} + bad + '"');
        }

        check(!account::valid_username(std::string(65, 'a')), "rejects a 65-character name");
        check(account::valid_username(std::string(64, 'a')), "accepts a 64-character name");

        for (auto const& good :
             {"alice", "Alice99", "alice_smith", "alice-smith", "alice.smith", "a"}) {
            check(account::valid_username(good), std::string{"accepts \""} + good + '"');
        }
    }

    void a_failing_write_leaves_the_previous_vault_intact() {
        section("writes are atomic and report failure");

#ifdef _WIN32
        std::cout << "  skip  a read-only directory attribute does not block file creation on "
                     "Windows\n";
#else
        if (geteuid() == 0) {
            std::cout << "  skip  running as root, directory permissions would not be enforced\n";
            return;
        }

        reset();
        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "u@example.com", "gh-s3cret"}}, pw);

        auto const vault_path = file::vault::path("alice");
        std::string const before = raw_bytes(vault_path);
        check(!before.empty(), "vault written");

        // Make the directory unwritable so the staging file cannot be created.
        fs::permissions(config_dir(), fs::perms::owner_read | fs::perms::owner_exec);

        bool threw = false;
        try {
            file::vault::write("alice", {credential{"Replacement", "x", "y"}}, pw);
        } catch (std::exception const&) {
            threw = true;
        }

        fs::permissions(config_dir(), fs::perms::owner_all);

        check(threw, "a write that cannot succeed throws instead of failing silently");
        check(raw_bytes(vault_path) == before, "the previous vault is byte-for-byte intact");
        check(fs::exists(vault_path) && fs::file_size(vault_path) > 0,
              "the vault was never truncated to 0 bytes");

        auto recovered = file::vault::read("alice", pw);
        check(recovered.size() == 1 && recovered[0].password == "gh-s3cret",
              "the original credential is still readable");
#endif
    }

    void no_staging_file_is_left_behind() {
        section("no leftover staging files");

        reset();
        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "u", "s"}}, pw);

        bool leftover = false;
        for (auto const& e : fs::directory_iterator{config_dir()}) {
            if (e.path().extension() == ".tmp") {
                leftover = true;
            }
        }
        check(!leftover, "no '.tmp' staging file remains after a successful write");
    }

    void vault_files_are_not_readable_by_others() {
        section("file permissions");

        reset();
        account::create(user{"alice", pw});

#ifndef _WIN32
        auto perms = fs::status(file::vault::path("alice")).permissions();
        check((perms & (fs::perms::group_all | fs::perms::others_all)) == fs::perms::none,
              "vault file is owner-only");
#else
        std::cout << "  skip  POSIX mode bits do not apply on Windows\n";
#endif
    }

    void secrets_never_appear_in_cleartext_on_disk() {
        section("vault contents are ciphertext");

        reset();
        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "alice@example.com", "gh-s3cret-token"}},
                           pw);

        auto const bytes = raw_bytes(file::vault::path("alice"));
        for (auto const& secret : {"gh-s3cret-token", "alice@example.com", "GitHub", pw.c_str()}) {
            check(bytes.find(secret) == std::string::npos,
                  std::string{"vault does not contain \""} + secret + "\" in cleartext");
        }
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
        file::vault::write("alice", {credential{"GitHub", "u", "gh-s3cret"}}, pw);

        std::string const newpw = "a-brand-new-passphrase";
        account::change_password(user{"alice", pw}, newpw);

        check(!account::valid_password(user{"alice", pw}), "old password no longer works");
        check(account::valid_password(user{"alice", newpw}), "new password works");

        auto after = file::vault::read("alice", newpw);
        check(after.size() == 1 && after[0].password == "gh-s3cret",
              "vault readable under the new password, contents intact");

        bool old_fails = false;
        try {
            (void)file::vault::read("alice", pw);
        } catch (std::exception const&) {
            old_fails = true;
        }
        check(old_fails, "vault not readable under the old password");
    }

    void the_remembered_username_holds_no_secret() {
        section("remembered username");

        reset();
        file::last_user::save("alice");
        check(file::last_user::load() == "alice", "remembered username round-trips");

        auto const p = config_dir() / ".last_user";
        check(raw_bytes(p) == "alice", "file contains exactly the username");
        check(raw_bytes(p).find(pw) == std::string::npos, "file contains no password");

#ifndef _WIN32
        auto perms = fs::status(p).permissions();
        check((perms & (fs::perms::group_all | fs::perms::others_all)) == fs::perms::none,
              ".last_user is owner-only");
#endif
    }

    // Two accounts must never map onto each other's files. These neighbour names probe every
    // string Redux still derives from a username: the lock file name (".<name>.lock") and the
    // staging name used while writing (dot-prefixed, pid + random suffix). Each of these is a
    // name a user can register.
    void one_account_cannot_overwrite_another() {
        section("account file paths cannot collide");

        reset();

        std::vector<std::string> const neighbours{"alice.lock", "alice.tmp", "alice_data"};
        std::string const npw = "neighbour-password";

        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "u", "alice-secret"}}, pw);

        for (auto const& n : neighbours) {
            check(account::valid_username(n), n + " is a name a user could register");
            account::create(user{n, npw});
            file::vault::write(n, {credential{"Site", "u", n + "-secret"}}, npw);
        }

        // Every derived vault path must be distinct.
        std::vector<std::string> paths{file::vault::path("alice").string()};
        for (auto const& n : neighbours) {
            paths.push_back(file::vault::path(n).string());
        }
        std::sort(paths.begin(), paths.end());
        check(std::adjacent_find(paths.begin(), paths.end()) == paths.end(),
              "no two account vault paths are identical");

        // Rewrite alice's files now that the neighbours exist -- this is the moment a colliding
        // lock or staging name would do its damage: both are created and then released or
        // renamed away while a neighbour's account is on disk.
        account::change_password(user{"alice", pw}, pw);
        file::vault::write("alice", {credential{"GitHub", "u", "alice-secret"}}, pw);

        check(account::valid_password(user{"alice", pw}), "alice's account still works");
        auto alice_creds = file::vault::read("alice", pw);
        check(alice_creds.size() == 1 && alice_creds[0].password == "alice-secret",
              "alice's vault is intact");

        for (auto const& n : neighbours) {
            check(account::valid_password(user{n, npw}), n + "'s account survived writes to alice");
            std::vector<credential> c;
            try {
                c = file::vault::read(n, npw);
            } catch (std::exception const&) {
                // leave c empty; the check below reports it
            }
            check(c.size() == 1 && c[0].password == n + "-secret", n + "'s vault is intact");
        }
    }

    void the_envelope_starts_with_the_expected_magic_and_version() {
        section("envelope header");

        reset();
        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "u", "s"}}, pw);
        auto const first = raw_bytes(file::vault::path("alice"));

        check(first.size() >= 43, "vault is at least as large as the 43-byte header");
        check(first.compare(0, 8, "REDUXVLT", 8) == 0, "vault begins with the REDUXVLT magic");
        check(static_cast<unsigned char>(first[8]) == 1, "vault declares format version 1");

        file::vault::write("alice", {credential{"GitHub", "u", "s"}}, pw);
        auto const second = raw_bytes(file::vault::path("alice"));

        check(first.compare(14, 16, second, 14, 16) != 0,
              "salt differs between two writes of the same data");
        check(first.compare(31, 12, second, 31, 12) != 0,
              "nonce differs between two writes of the same data");
    }

    void the_header_declares_scrypt_parameters_at_least_as_strong_as_the_minimum() {
        section("scrypt parameters");

        reset();
        account::create(user{"alice", pw});
        auto const bytes = raw_bytes(file::vault::path("alice"));

        check(static_cast<unsigned char>(bytes[9]) == 1, "kdf id is scrypt (1)");
        check(static_cast<unsigned char>(bytes[10]) >= 15, "log2(N) is at least 15");
        check(static_cast<unsigned char>(bytes[11]) >= 8, "r is at least 8");
        check(static_cast<unsigned char>(bytes[12]) >= 1, "p is at least 1");
    }

    void wrong_password_is_false_but_a_corrupted_header_throws() {
        section("error taxonomy: wrong password vs. corruption");

        reset();
        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "u", "s"}}, pw);

        check(!account::valid_password(user{"alice", "wrong-password"}),
              "a wrong password returns false");

        // Flip a magic byte: corrupts the header, not the ciphertext, so the failure has nothing
        // to do with the password at all.
        flip_byte_at(file::vault::path("alice"), 0);

        bool threw = false;
        try {
            (void)account::valid_password(user{"alice", pw});
        } catch (std::exception const&) {
            threw = true;
        }
        check(threw, "a corrupted header throws rather than merely returning false");
    }

    void truncated_ciphertext_throws() {
        section("truncated body");

        reset();
        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "u", "s"}}, pw);

        auto const p = file::vault::path("alice");
        auto const full = raw_bytes(p);
        check(full.size() > 43, "vault has a non-empty ciphertext body");

        {
            std::ofstream out{p, std::ios::binary | std::ios::trunc};
            out.write(full.data(), static_cast<std::streamsize>(full.size() - 5));
        }

        bool threw = false;
        try {
            (void)file::vault::read("alice", pw);
        } catch (std::exception const&) {
            threw = true;
        }
        check(threw, "a truncated vault throws on read");
    }

    void tampering_with_the_header_salt_is_detected() {
        section("header is authenticated (AAD)");

        reset();
        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "u", "s"}}, pw);

        // Byte 14 is the first salt byte; the ciphertext body is untouched.
        flip_byte_at(file::vault::path("alice"), 14);

        bool threw = false;
        try {
            (void)file::vault::read("alice", pw);
        } catch (std::exception const&) {
            threw = true;
        }
        check(threw, "tampering with a salt byte (body untouched) is caught by the AAD tag");
    }

    // The scrypt r and p bytes (header offsets 11 and 12) reach key derivation before the GCM
    // tag check authenticates the header, so an out-of-range value must be rejected as an
    // unsupported format -- fast, without deriving a key -- rather than forcing a huge
    // allocation and only then failing the tag check.
    void hostile_scrypt_parameters_are_rejected_before_key_derivation() {
        section("hostile scrypt parameters in the header");

        auto rejected_as_unsupported = [](std::streamoff offset, unsigned char value) {
            reset();
            account::create(user{"alice", pw});
            file::vault::write("alice", {credential{"GitHub", "u", "s"}}, pw);
            set_byte_at(file::vault::path("alice"), offset, value);

            try {
                (void)file::vault::read("alice", pw);
            } catch (std::runtime_error const& e) {
                // The unsupported-format error, not HashVerificationFailed: the latter would
                // mean key derivation ran and only the tag check caught the tampering.
                return std::string{e.what()}.find("unsupported Redux vault format") !=
                       std::string::npos;
            } catch (std::exception const&) {
            }
            return false;
        };

        check(rejected_as_unsupported(11, 200), "r = 200 is rejected without deriving a key");
        check(rejected_as_unsupported(11, 0), "r = 0 is rejected");
        check(rejected_as_unsupported(12, 200), "p = 200 is rejected without deriving a key");
        check(rejected_as_unsupported(12, 0), "p = 0 is rejected");

        // Individually, log2(N) = 24 and r = 32 each pass their own range check; combined, the
        // scrypt working set is 128 * 2^24 * 32 = 64 GiB. The product ceiling has to reject the
        // pair before key derivation -- per-parameter bounds alone cannot.
        {
            reset();
            account::create(user{"alice", pw});
            file::vault::write("alice", {credential{"GitHub", "u", "s"}}, pw);
            set_byte_at(file::vault::path("alice"), 10, 24);
            set_byte_at(file::vault::path("alice"), 11, 32);

            bool rejected = false;
            try {
                (void)file::vault::read("alice", pw);
            } catch (std::runtime_error const& e) {
                rejected = std::string{e.what()}.find("unsupported Redux vault format") !=
                           std::string::npos;
            } catch (std::exception const&) {
            }
            check(rejected, "log2(N) = 24 with r = 32 (64 GiB combined) is rejected");
        }
    }

    void a_vault_copied_onto_another_username_is_rejected() {
        section("username recorded in the plaintext must match the account");

        reset();
        account::create(user{"alice", pw});
        file::vault::write("alice", {credential{"GitHub", "u", "alice-secret"}}, pw);
        account::create(user{"bob", pw});

        fs::copy_file(file::vault::path("alice"), file::vault::path("bob"),
                      fs::copy_options::overwrite_existing);

        bool read_threw = false;
        try {
            (void)file::vault::read("bob", pw);
        } catch (std::exception const&) {
            read_threw = true;
        }
        check(read_threw,
              "reading alice's vault under bob's name throws rather than returning data");

        bool valid_password_threw = false;
        try {
            (void)account::valid_password(user{"bob", pw});
        } catch (std::exception const&) {
            valid_password_threw = true;
        }
        check(valid_password_threw,
              "valid_password propagates the mismatch instead of reporting it as a wrong password");
    }

    void account_lock_is_exclusive_and_non_blocking() {
        section("account_lock");

        reset();
        account::create(user{"alice", pw});

        std::optional<file::account_lock> first;
        first.emplace("alice");

        bool busy = false;
        try {
            file::account_lock second{"alice"};
            (void)second;
        } catch (file::account_busy const&) {
            busy = true;
        }
        check(busy, "a second lock on the same account is refused while the first is held");

        first.reset();

        bool threw_after_release = false;
        try {
            file::account_lock third{"alice"};
            (void)third;
        } catch (std::exception const&) {
            threw_after_release = true;
        }
        check(!threw_after_release, "the lock can be acquired again once the first is released");
    }

    // A minimal RFC-4180 field parser -- enough to check column counts and quoting for this
    // test, not a general CSV library.
    std::vector<std::string> parse_csv_line(std::string const& line) {
        std::vector<std::string> fields;
        std::string field;
        bool in_quotes = false;
        for (std::size_t i = 0; i < line.size(); ++i) {
            char const c = line[i];
            if (in_quotes) {
                if (c == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        field += '"';
                        ++i;
                    } else {
                        in_quotes = false;
                    }
                } else {
                    field += c;
                }
            } else if (c == '"') {
                in_quotes = true;
            } else if (c == ',') {
                fields.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }
        fields.push_back(field);
        return fields;
    }

    void csv_export_quotes_fields_and_neutralizes_formulas() {
        section("CSV export");

        reset();
        std::vector<credential> const creds{
            credential{"Ac,me", "user,name", "pa\"ss"},
            credential{"=2+3", "@cmd", "value"},
        };

        auto const csv_path = sandbox / "alice.csv";
        file::users::writeToCSV(csv_path.string(), creds);

        check(fs::exists(csv_path), "CSV file was written");

#ifndef _WIN32
        auto perms = fs::status(csv_path).permissions();
        check((perms & (fs::perms::group_all | fs::perms::others_all)) == fs::perms::none,
              "CSV file is owner-only");
#endif

        std::string const contents = raw_bytes(csv_path);

        std::istringstream lines{contents};
        std::string line;
        std::vector<std::vector<std::string>> rows;
        while (std::getline(lines, line)) {
            if (!line.empty()) {
                rows.push_back(parse_csv_line(line));
            }
        }

        check(rows.size() == 3, "header plus two credential rows");
        for (auto const& row : rows) {
            check(row.size() == 4, "exactly 4 columns per row");
        }

        check(rows[1][1] == "Ac,me", "embedded comma in company name survives round-trip");
        check(rows[1][2] == "user,name", "embedded comma in username survives round-trip");
        check(rows[1][3] == "pa\"ss", "embedded quote survives round-trip");

        check(rows[2][1] == "'=2+3", "a leading '=' is neutralized with a leading single quote");
        check(rows[2][2] == "'@cmd", "a leading '@' is neutralized with a leading single quote");

        check(contents.find("\"=2+3\"") == std::string::npos,
              "the raw file never contains an unneutralized formula-looking field");
    }

    void config_dir_fails_closed_instead_of_falling_back_to_the_working_directory() {
        section("fail-closed config directory");

#ifdef _WIN32
        // On Windows the profile directory is resolved from the process token, not the
        // environment, so there is no environment-only failure to simulate -- unsetting the
        // variables would just make the probe land in the real %USERPROFILE%\Redux.
        std::cout << "  skip  the profile directory comes from the process token, not the "
                     "environment\n";
#else
        char const* saved_redux_dir = std::getenv("REDUX_CONFIG_DIR");
        std::string const saved_redux_dir_value = saved_redux_dir ? saved_redux_dir : "";
        char const* saved_home = std::getenv("HOME");
        std::string const saved_home_value = saved_home ? saved_home : "";
        bool const had_home = saved_home != nullptr;

        unset_env("REDUX_CONFIG_DIR");
        unset_env("HOME");

        auto const cwd = fs::current_path();
        std::vector<std::string> before_entries;
        for (auto const& e : fs::directory_iterator{cwd}) {
            before_entries.push_back(e.path().filename().string());
        }

        bool threw = false;
        try {
            account::create(user{"cwd-probe", pw});
        } catch (std::exception const&) {
            threw = true;
        }
        check(threw, "vault operations throw rather than silently using the working directory");

        std::vector<std::string> after_entries;
        for (auto const& e : fs::directory_iterator{cwd}) {
            after_entries.push_back(e.path().filename().string());
        }
        check(before_entries.size() == after_entries.size(),
              "no file was created in the current working directory");

        set_env("REDUX_CONFIG_DIR", saved_redux_dir_value);
        if (had_home) {
            set_env("HOME", saved_home_value);
        }
#endif
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
    set_env("REDUX_CONFIG_DIR", sandbox.string());

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
             testcase{"remembered username", the_remembered_username_holds_no_secret},
             testcase{"path collisions", one_account_cannot_overwrite_another},
             testcase{"envelope header", the_envelope_starts_with_the_expected_magic_and_version},
             testcase{"scrypt parameters",
                      the_header_declares_scrypt_parameters_at_least_as_strong_as_the_minimum},
             testcase{"error taxonomy", wrong_password_is_false_but_a_corrupted_header_throws},
             testcase{"truncated body", truncated_ciphertext_throws},
             testcase{"AAD tampering", tampering_with_the_header_salt_is_detected},
             testcase{"hostile scrypt parameters",
                      hostile_scrypt_parameters_are_rejected_before_key_derivation},
             testcase{"username mismatch", a_vault_copied_onto_another_username_is_rejected},
             testcase{"account_lock", account_lock_is_exclusive_and_non_blocking},
             testcase{"CSV export", csv_export_quotes_fields_and_neutralizes_formulas},
             testcase{"fail-closed config dir",
                      config_dir_fails_closed_instead_of_falling_back_to_the_working_directory},
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
    fs::permissions(sandbox, fs::perms::owner_all, ec);
    fs::remove_all(sandbox, ec);

    std::cout << "\n" << (checks - failures) << "/" << checks << " checks passed\n";
    if (failures != 0) {
        std::cout << failures << " FAILED\n";
        return 1;
    }
    return 0;
}
